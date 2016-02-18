#if _MSC_VER
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif

#include "QuadKey.hpp"
#include "builders/ExternalBuilder.hpp"
#include "builders/TerraBuilder.hpp"
#include "builders/TileBuilder.hpp"
#include "entities/ElementVisitor.hpp"
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

static utymap::TileBuilder* tileLoaderPtr = nullptr;
static utymap::index::GeoStore* geoStorePtr = nullptr;
static utymap::index::InMemoryElementStore* inMemoryStorePtr = nullptr;
static utymap::index::PersistentElementStore* persistentStorePtr = nullptr;
static utymap::index::StringTable* stringTablePtr = nullptr;
static utymap::heightmap::ElevationProvider* eleProviderPtr = nullptr;

static std::unordered_map<std::string, std::shared_ptr<utymap::mapcss::StyleProvider>> styleProviders;

const std::string InMemoryStorageKey = "InMemory";
const std::string PersistentStorageKey = "OnDisk";


std::shared_ptr<utymap::mapcss::StyleProvider> getStyleProvider(const std::string& path)
{
    auto pair = styleProviders.find(path);
    if (pair != styleProviders.end())
        return pair->second;

    std::ifstream styleFile(path);
    utymap::mapcss::MapCssParser parser;
    utymap::mapcss::StyleSheet stylesheet = parser.parse(styleFile);
    styleProviders[path] = std::shared_ptr<utymap::mapcss::StyleProvider>(new utymap::mapcss::StyleProvider(stylesheet, *stringTablePtr));
    return styleProviders[path];
}

extern "C"
{
    // Called when mesh is built.
    typedef void OnMeshBuilt(const char* name,
                             const double* vertices, int vertexCount,
                             const int* triangles, int triCount,
                             const int* colors, int colorCount);
    // Called when element is loaded.
    typedef void OnElementLoaded(uint64_t id, const char** tags, int size,
                                 const double* vertices, int vertexCount,
                                 const char** style, int styleSize);
    // Called when operation is completed
    typedef void OnError(const char* errorMessage);

    // Composes object graph.
    void EXPORT_API configure(const char* stringPath, // path to string table directory
                              const char* dataPath,    // path to index directory
                              OnError* errorCallback)
    {
        stringTablePtr = new utymap::index::StringTable(stringPath);
        inMemoryStorePtr = new utymap::index::InMemoryElementStore(*stringTablePtr);
        persistentStorePtr = new utymap::index::PersistentElementStore(dataPath, *stringTablePtr);

        geoStorePtr = new utymap::index::GeoStore(*stringTablePtr);
        geoStorePtr->registerStore(InMemoryStorageKey, *inMemoryStorePtr);
        geoStorePtr->registerStore(PersistentStorageKey, *persistentStorePtr);

        eleProviderPtr = new utymap::heightmap::FlatElevationProvider();
        tileLoaderPtr = new utymap::TileBuilder(*geoStorePtr, *stringTablePtr, *eleProviderPtr);
    }

    // Registers external element visitor.
    void EXPORT_API registerElementVisitor(const char* name)
    {
        tileLoaderPtr->registerElementVisitor(name, [&](const utymap::QuadKey& quadKey,
                                                        const utymap::mapcss::StyleProvider& styleProvider, 
                                                        const utymap::TileBuilder::MeshCallback& meshFunc,
                                                        const utymap::TileBuilder::ElementCallback& elementFunc) {
            return std::shared_ptr<utymap::entities::ElementVisitor>(new utymap::builders::ExternalBuilder(elementFunc));
        });
    }

    // Registers stylesheet in advance.
    void EXPORT_API registerStylesheet(const char* path) {
        getStyleProvider(path);
    }

    void EXPORT_API cleanup()
    {
        delete tileLoaderPtr;
        delete geoStorePtr;
        delete persistentStorePtr;
        delete inMemoryStorePtr;
        delete stringTablePtr;
        delete eleProviderPtr;
        styleProviders.clear();
    }

    // TODO for single element as well

    void EXPORT_API addToPersistentStore(const char* styleFile, const char* path, int startLod, int endLod, OnError* errorCallback)
    {
        geoStorePtr->add(PersistentStorageKey, path, utymap::index::LodRange(startLod, endLod), *getStyleProvider(styleFile).get());
    }

    void EXPORT_API addToInMemoryStore(const char* styleFile, const char* path, int startLod, int endLod, OnError* errorCallback)
    {
        geoStorePtr->add(InMemoryStorageKey, path, utymap::index::LodRange(startLod, endLod), *getStyleProvider(styleFile).get());
    }

    void EXPORT_API loadTile(const char* styleFile,
                             int tileX, int tileY, int levelOfDetail,
                             OnMeshBuilt* meshCallback,
                             OnElementLoaded* elementCallback,
                             OnError* errorCallback)
    {
        utymap::QuadKey quadKey;
        quadKey.tileX = tileX;
        quadKey.tileY = tileY;
        quadKey.levelOfDetail = levelOfDetail;

        utymap::mapcss::StyleProvider& styleProvider = *getStyleProvider(styleFile).get();

        struct ElementVisitor : public utymap::entities::ElementVisitor
        {
            using Tags = std::vector<utymap::entities::Tag>;
            using Coordinates = std::vector<utymap::GeoCoordinate> ;

            ElementVisitor(utymap::index::StringTable& stringTable, 
                           utymap::mapcss::StyleProvider& styleProvider, 
                           int levelOfDetail,
                           OnElementLoaded* elementCallback) :
                stringTable_(stringTable), 
                styleProvider_(styleProvider), 
                levelOfDetail_(levelOfDetail),
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
        } elementVisitor(*stringTablePtr, styleProvider, levelOfDetail, elementCallback);

        tileLoaderPtr->build(quadKey, styleProvider,
            [&meshCallback](const utymap::meshing::Mesh& mesh) {
                meshCallback(mesh.name.data(),
                    mesh.vertices.data(), mesh.vertices.size(),
                    mesh.triangles.data(), mesh.triangles.size(),
                    mesh.colors.data(), mesh.colors.size());
        }, [&elementVisitor](const utymap::entities::Element& element) {
            element.accept(elementVisitor);
        });
    }

    void EXPORT_API search(double latitude, double longitude, double radius,
                           OnElementLoaded* elementCallback,
                           OnError* errorCallback)
    {
        // TODO
    }
}
