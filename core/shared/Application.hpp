#ifndef APPLICATION_HPP_DEFINED
#define APPLICATION_HPP_DEFINED

#include "GeoCoordinate.hpp"
#include "BoundingBox.hpp"
#include "QuadKey.hpp"
#include "LodRange.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/ExternalBuilder.hpp"
#include "builders/QuadKeyBuilder.hpp"
#include "builders/buildings/LowPolyBuildingBuilder.hpp"
#include "builders/poi/TreeBuilder.hpp"
#include "builders/terrain/TerraBuilder.hpp"
#include "entities/Element.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "heightmap/SrtmElevationProvider.hpp"
#include "index/GeoStore.hpp"
#include "index/InMemoryElementStore.hpp"
#include "index/PersistentElementStore.hpp"
#include "index/StringTable.hpp"
#include "mapcss/MapCssParser.hpp"
#include "mapcss/StyleProvider.hpp"
#include "mapcss/StyleSheet.hpp"
#include "meshing/MeshTypes.hpp"
#include "utils/GeoUtils.hpp"

#include <cstdint>
#include <exception>
#include <fstream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

// Called when mesh is built.
typedef void OnMeshBuilt(const char* name,
                         const double* vertices, int vertexCount,
                         const int* triangles, int triCount,
                         const int* colors, int colorCount);

// Called when element is loaded.
typedef void OnElementLoaded(uint64_t id, const char** tags, int size,
                             const double* vertices, int vertexCount,
                             const char** style, int styleSize);

// Called when operation is completed.
typedef void OnError(const char* errorMessage);

// Exposes API for external usage.
class Application
{
    const std::string InMemoryStorageKey = "InMemory";
    const std::string PersistentStorageKey = "OnDisk";
    const int SrtmElevationLodStart = 12;

    // Visits element in quadkey.
    struct QuadKeyElementVisitor : public utymap::entities::ElementVisitor
    {
        using Tags = std::vector<utymap::formats::Tag>;
        using Coordinates = std::vector<utymap::GeoCoordinate>;

        QuadKeyElementVisitor(utymap::index::StringTable& stringTable, 
                              utymap::mapcss::StyleProvider& styleProvider,
                              int levelOfDetail, 
                              OnElementLoaded* elementCallback) :
            stringTable_(stringTable), styleProvider_(styleProvider), 
            levelOfDetail_(levelOfDetail), elementCallback_(elementCallback)
        {
        }

        void visitNode(const utymap::entities::Node& node)
        {
            visitElement(node, Coordinates{ node.coordinate });
        }

        void visitWay(const utymap::entities::Way& way)
        {
            visitElement(way, way.coordinates);
        }

        void visitArea(const utymap::entities::Area& area)
        {
            visitElement(area, area.coordinates);
        }

        void visitRelation(const utymap::entities::Relation& relation)
        {
            for (const auto& element : relation.elements)
                element->accept(*this);
        }
    private:

        void visitElement(const utymap::entities::Element& element, const Coordinates& coordinates)
        {
            // convert tags
            std::vector<const char*> ctags;
            tagStrings_.reserve(element.tags.size() * 2);
            ctags.reserve(element.tags.size() * 2);
            for (auto i = 0; i < element.tags.size(); ++i) {
                const utymap::entities::Tag& tag = element.tags[i];
                tagStrings_.push_back(stringTable_.getString(tag.key));
                tagStrings_.push_back(stringTable_.getString(tag.value));
                ctags.push_back(tagStrings_[tagStrings_.size() - 2].c_str());
                ctags.push_back(tagStrings_[tagStrings_.size() - 1].c_str());
            }
            // convert geometry
            std::vector<double> coords;
            coords.reserve(coordinates.size() * 2);
            for (auto i = 0; i < coordinates.size(); ++i) {
                const utymap::GeoCoordinate coordinate = coordinates[i];
                coords.push_back(coordinate.longitude);
                coords.push_back(coordinate.latitude);
            }
            // convert style
            utymap::mapcss::Style style = styleProvider_.forElement(element, levelOfDetail_);
            std::vector<const char*> cstyles;
            tagStrings_.reserve(style.declarations.size() * 2);
            cstyles.reserve(style.declarations.size());
            for (const auto pair : style.declarations) {
                styleStrings_.push_back(stringTable_.getString(pair.first));
                styleStrings_.push_back(*pair.second->value());
                cstyles.push_back(styleStrings_[styleStrings_.size() - 2].c_str());
                cstyles.push_back(styleStrings_[styleStrings_.size() - 1].c_str());
            }

            elementCallback_(element.id, ctags.data(), ctags.size(),
                coords.data(), coords.size(), cstyles.data(), cstyles.size());
            tagStrings_.clear();
            styleStrings_.clear();
        }
        utymap::index::StringTable& stringTable_;
        utymap::mapcss::StyleProvider& styleProvider_;
        int levelOfDetail_;
        OnElementLoaded* elementCallback_;
        std::vector<std::string> tagStrings_;   // holds temporary tag strings
        std::vector<std::string> styleStrings_; // holds temporary style strings
    };

public:
    // Composes object graph.
    Application(const char* stringPath, const char* dataPath, const char* elePath, OnError* errorCallback) :
        stringTable_(stringPath), inMemoryStore_(stringTable_), persistentStore_(dataPath, stringTable_),
        geoStore_(stringTable_), srtmEleProvider_(elePath), flatEleProvider_(),
        quadKeyBuilder_(geoStore_, stringTable_)
    {
        geoStore_.registerStore(InMemoryStorageKey, inMemoryStore_);
        geoStore_.registerStore(PersistentStorageKey, persistentStore_);
        registerDefaultBuilders();
    }

    // Register stylesheet.
    void registerStylesheet(const char* path)
    {
        getStyleProvider(path);
    }

    // Register element builder.
    void registerElementBuilder(const char* name)
    {
        quadKeyBuilder_.registerElementBuilder(name, [&](const utymap::builders::BuilderContext& context) {
            return std::shared_ptr<utymap::builders::ElementBuilder>(new utymap::builders::ExternalBuilder(context));
        });
    }

    // Adds data to in-memory store.
    void addToPersistentStore(const char* styleFile, const char* path, const utymap::QuadKey& quadKey, OnError* errorCallback)
    {
    }

    // Adds data to persistent store.
    void addToPersistentStore(const char* styleFile, const char* path, const utymap::LodRange& range, OnError* errorCallback)
    {
        try {
            geoStore_.add(PersistentStorageKey, path, range, *getStyleProvider(styleFile).get());
        }
        catch (std::exception& ex) {
            errorCallback(ex.what());
        }
    }

    // Adds data to in-memory store.
    void addToInMemoryStore(const char* styleFile, const char* path, const utymap::QuadKey& quadKey, OnError* errorCallback)
    {
        try {
            geoStore_.add(InMemoryStorageKey, path, quadKey, *getStyleProvider(styleFile).get());
        }
        catch (std::exception& ex) {
            errorCallback(ex.what());
        }
    }

    // Adds data to in-memory store.
    void addToInMemoryStore(const char* styleFile, const char* path, const utymap::BoundingBox& bbox, const utymap::LodRange& range, OnError* errorCallback)
    {
        try {
            geoStore_.add(InMemoryStorageKey, path, bbox, range, *getStyleProvider(styleFile).get());
        }
        catch (std::exception& ex) {
            errorCallback(ex.what());
        }
    }

    // Adds data to in-memory store.
    void addToInMemoryStore(const char* styleFile, const char* path, const utymap::LodRange& range, OnError* errorCallback)
    {
        try {
            geoStore_.add(InMemoryStorageKey, path, range, *getStyleProvider(styleFile).get());
        }
        catch (std::exception& ex) {
            errorCallback(ex.what());
        }
    }

    // Adds element to in-memory store.
    void addInMemoryStore(const char* styleFile, const utymap::entities::Element& element, const utymap::LodRange& range, OnError* errorCallback)
    {
        try {
            geoStore_.add(InMemoryStorageKey, element, range, *getStyleProvider(styleFile).get());
        }
        catch (std::exception& ex) {
            errorCallback(ex.what());
        }
    }

    bool hasData(const utymap::QuadKey& quadKey)
    {
        return geoStore_.hasData(quadKey);
    }

    // Loads quadKey.
    void loadQuadKey(const char* styleFile, const utymap::QuadKey& quadKey, OnMeshBuilt* meshCallback,
                  OnElementLoaded* elementCallback, OnError* errorCallback)
    {
        try {
            auto& eleProvider = quadKey.levelOfDetail <= SrtmElevationLodStart
                ? flatEleProvider_
                : (utymap::heightmap::ElevationProvider&) srtmEleProvider_;

            // TODO: preloading is not thread safe: extract separate API function
            eleProvider.preload(utymap::utils::GeoUtils::quadKeyToBoundingBox(quadKey));

            utymap::mapcss::StyleProvider& styleProvider = *getStyleProvider(styleFile);
            QuadKeyElementVisitor elementVisitor(stringTable_, styleProvider, quadKey.levelOfDetail, elementCallback);
            quadKeyBuilder_.build(quadKey, styleProvider, eleProvider,
                [&meshCallback](const utymap::meshing::Mesh& mesh) {
                meshCallback(mesh.name.data(),
                    mesh.vertices.data(), mesh.vertices.size(),
                    mesh.triangles.data(), mesh.triangles.size(),
                    mesh.colors.data(), mesh.colors.size());
            }, [&elementVisitor](const utymap::entities::Element& element) {
                element.accept(elementVisitor);
            });
        }
        catch (std::exception& ex) {
            errorCallback(ex.what());
        }
    }

    // Gets id for the string.
    inline std::uint32_t getStringId(const char* str)
    {
        return stringTable_.getId(str);
    }

private:
    std::shared_ptr<utymap::mapcss::StyleProvider> getStyleProvider(const std::string& filePath)
    {
        auto pair = styleProviders_.find(filePath);
        if (pair != styleProviders_.end())
            return pair->second;

        std::ifstream styleFile(filePath);
        if (!styleFile.good())
            throw std::invalid_argument(std::string("Cannot read mapcss file:") + filePath);

        // NOTE not safe, but don't want to use boost filesystem only for this task.
        std::string dir = filePath.substr(0, filePath.find_last_of("\\/") + 1);
        utymap::mapcss::MapCssParser parser(dir);
        utymap::mapcss::StyleSheet stylesheet = parser.parse(styleFile);
        styleProviders_[filePath] = std::shared_ptr<utymap::mapcss::StyleProvider>(
            new utymap::mapcss::StyleProvider(stylesheet, stringTable_));
        return styleProviders_[filePath];
    }

    void registerDefaultBuilders()
    {
        quadKeyBuilder_.registerElementBuilder("terrain", [&](const utymap::builders::BuilderContext& context) {
            return std::shared_ptr<utymap::builders::ElementBuilder>(new utymap::builders::TerraBuilder(context));
        });

        quadKeyBuilder_.registerElementBuilder("building", [&](const utymap::builders::BuilderContext& context) {
            return std::shared_ptr<utymap::builders::ElementBuilder>(new utymap::builders::LowPolyBuildingBuilder(context));
        });

        quadKeyBuilder_.registerElementBuilder("tree", [&](const utymap::builders::BuilderContext& context) {
            return std::shared_ptr<utymap::builders::ElementBuilder>(new utymap::builders::TreeBuilder(context));
        });
    }

    utymap::index::StringTable stringTable_;
    utymap::index::InMemoryElementStore inMemoryStore_;
    utymap::index::PersistentElementStore persistentStore_;
    utymap::index::GeoStore geoStore_;
    utymap::heightmap::FlatElevationProvider flatEleProvider_;
    utymap::heightmap::SrtmElevationProvider srtmEleProvider_;

    utymap::builders::QuadKeyBuilder quadKeyBuilder_;
    std::unordered_map<std::string, std::shared_ptr<utymap::mapcss::StyleProvider>> styleProviders_;
};

#endif // APPLICATION_HPP_DEFINED
