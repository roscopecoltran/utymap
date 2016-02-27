#ifndef APPLICATION_HPP_DEFINED
#define APPLICATION_HPP_DEFINED

#include "QuadKey.hpp"
#include "builders/BuilderContext.hpp"
#include "builders/ExternalBuilder.hpp"
#include "builders/TileBuilder.hpp"
#include "builders/buildings/LowPolyBuildingBuilder.hpp"
#include "builders/terrain/TerraBuilder.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "index/GeoStore.hpp"
#include "index/InMemoryElementStore.hpp"
#include "index/PersistentElementStore.hpp"
#include "index/LodRange.hpp"
#include "index/StringTable.hpp"
#include "mapcss/MapCssParser.hpp"
#include "mapcss/StyleProvider.hpp"
#include "mapcss/StyleSheet.hpp"
#include "meshing/MeshTypes.hpp"

#include <cstdint>
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

    // Visits element in tile.
    struct TileElementVisitor : public utymap::entities::ElementVisitor
    {
        using Tags = std::vector<utymap::formats::Tag>;
        using Coordinates = std::vector<utymap::GeoCoordinate>;

        TileElementVisitor(utymap::index::StringTable& stringTable, utymap::mapcss::StyleProvider& styleProvider,
                           int levelOfDetail, OnElementLoaded* elementCallback) :
            stringTable_(stringTable), styleProvider_(styleProvider), levelOfDetail_(levelOfDetail),
                elementCallback_(elementCallback)
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
                styleStrings_.push_back(*pair.second);
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
    Application(const char* stringPath, const char* dataPath, OnError* errorCallback) :
        stringTable_(stringPath), inMemoryStore_(stringTable_), persistentStore_(dataPath, stringTable_),
        geoStore_(stringTable_), eleProvider_(), tileBuilder_(geoStore_, stringTable_, eleProvider_)
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
        tileBuilder_.registerElementBuilder(name, [&](const utymap::builders::BuilderContext& context) {
            return std::shared_ptr<utymap::builders::ElementBuilder>(new utymap::builders::ExternalBuilder(context));
        });
    }

    // Adds data to persistent store.
    void addToPersistentStore(const char* styleFile, const char* path, const utymap::index::LodRange& range, OnError* errorCallback)
    {
        geoStore_.add(PersistentStorageKey, path, range, *getStyleProvider(styleFile).get());
    }

    // Adds data to in-memory store.
    void addToInMemoryStore(const char* styleFile, const char* path, const utymap::index::LodRange& range, OnError* errorCallback)
    {
        geoStore_.add(InMemoryStorageKey, path, range, *getStyleProvider(styleFile).get());
    }

    // Loads tile.
    void loadTile(const char* styleFile, const utymap::QuadKey& quadKey, OnMeshBuilt* meshCallback,
                  OnElementLoaded* elementCallback, OnError* errorCallback)
    {
        utymap::mapcss::StyleProvider& styleProvider = *getStyleProvider(styleFile);
        TileElementVisitor elementVisitor(stringTable_, styleProvider, quadKey.levelOfDetail, elementCallback);
        tileBuilder_.build(quadKey, styleProvider,
            [&meshCallback](const utymap::meshing::Mesh& mesh) {
                meshCallback(mesh.name.data(),
                    mesh.vertices.data(), mesh.vertices.size(),
                    mesh.triangles.data(), mesh.triangles.size(),
                    mesh.colors.data(), mesh.colors.size());
        }, [&elementVisitor](const utymap::entities::Element& element) {
            element.accept(elementVisitor);
        });
    }

private:
    std::shared_ptr<utymap::mapcss::StyleProvider> getStyleProvider(const std::string& path)
    {
        auto pair = styleProviders_.find(path);
        if (pair != styleProviders_.end())
            return pair->second;

        std::ifstream styleFile(path);
        utymap::mapcss::MapCssParser parser;
        utymap::mapcss::StyleSheet stylesheet = parser.parse(styleFile);
        styleProviders_[path] = std::shared_ptr<utymap::mapcss::StyleProvider>(
            new utymap::mapcss::StyleProvider(stylesheet, stringTable_));
        return styleProviders_[path];
    }

    void registerDefaultBuilders()
    {
        tileBuilder_.registerElementBuilder("terrain", [&](const utymap::builders::BuilderContext& context) {
            return std::shared_ptr<utymap::builders::ElementBuilder>(new utymap::builders::TerraBuilder(context));
        });

        tileBuilder_.registerElementBuilder("building", [&](const utymap::builders::BuilderContext& context) {
            return std::shared_ptr<utymap::builders::ElementBuilder>(new utymap::builders::LowPolyBuildingBuilder(context));
        });
    }

    utymap::index::StringTable stringTable_;
    utymap::index::InMemoryElementStore inMemoryStore_;
    utymap::index::PersistentElementStore persistentStore_;
    utymap::index::GeoStore geoStore_;
    utymap::heightmap::FlatElevationProvider eleProvider_;
    utymap::builders::TileBuilder tileBuilder_;

   
    std::unordered_map<std::string, std::shared_ptr<utymap::mapcss::StyleProvider>> styleProviders_;
};

#endif // APPLICATION_HPP_DEFINED
