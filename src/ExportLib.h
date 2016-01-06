#if _MSC_VER
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif

#include "QuadKey.hpp"
#include "TileLoader.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "index/GeoStore.hpp"
#include "index/InMemoryElementStore.hpp"
#include "index/PersistentElementStore.hpp"
#include "index/StringTable.hpp"
#include "mapcss/MapCssParser.hpp"
#include "mapcss/StyleProvider.hpp"
#include "mapcss/StyleSheet.hpp"
#include "meshing/MeshTypes.hpp"

#include <cstdint>
#include <string>
#include <fstream>

static utymap::TileLoader* tileLoaderPtr = nullptr;
static utymap::index::GeoStore* geoStorePtr = nullptr;
static utymap::index::InMemoryElementStore* inMemoryStorePtr = nullptr;
static utymap::index::PersistentElementStore* persistentStorePtr = nullptr;
static utymap::index::StringTable* stringTablePtr = nullptr;
static utymap::mapcss::StyleProvider* styleProviderPtr = nullptr;
static utymap::heightmap::ElevationProvider<double>* eleProviderPtr = nullptr;

const std::string inMemoryStorageKey;
const std::string persistentStorageKey;

extern "C"
{
    // Called when mesh is built.
    typedef void OnMeshBuilt(const char* name,
                             const double* vertices, int vertexCount,
                             const int* triangles, int triCount,
                             const int* colors, int colorCount);
    // Called when element is loaded.
    typedef void OnElementLoaded(uint64_t id, const char* name,
                                 const char** tags, int size,
                                 const double* vertices, int vertexCount);
    // Called when operation is completed
    typedef void OnError(const char* errorMessage);

    void EXPORT_API configure(const char* stringPath, // path to string table directory
                             const char* stylePath,   // path to mapcss file
                             const char* dataPath,    // path to index directory
                             OnError* errorCallback)
    {
        std::ifstream styleFile(stylePath);
        utymap::mapcss::Parser parser;
        utymap::mapcss::StyleSheet stylesheet = parser.parse(styleFile);
        if (!parser.getError().empty()) {
            errorCallback(parser.getError().c_str());
            return;
        }

        stringTablePtr = new utymap::index::StringTable(stringPath);
        styleProviderPtr = new utymap::mapcss::StyleProvider(stylesheet, *stringTablePtr);

        inMemoryStorePtr = new utymap::index::InMemoryElementStore(*styleProviderPtr, *stringTablePtr);
        persistentStorePtr = new utymap::index::PersistentElementStore(dataPath, *styleProviderPtr, *stringTablePtr);
        
        geoStorePtr = new utymap::index::GeoStore(*styleProviderPtr, *stringTablePtr);
        geoStorePtr->registerStore(inMemoryStorageKey, *inMemoryStorePtr);
        geoStorePtr->registerStore(persistentStorageKey, *persistentStorePtr);

        eleProviderPtr = new utymap::heightmap::FlatElevationProvider<double>();
        tileLoaderPtr = new utymap::TileLoader(*geoStorePtr, *styleProviderPtr, *stringTablePtr, *eleProviderPtr);
    }

    void EXPORT_API cleanup()
    {
        delete tileLoaderPtr;
        delete geoStorePtr;
        delete persistentStorePtr;
        delete inMemoryStorePtr;
        delete stringTablePtr;
        delete styleProviderPtr;
        delete eleProviderPtr;
    }

    // TODO for single element as well

    void EXPORT_API addToPersistentStore(const char* path, OnError* errorCallback)
    {
        geoStorePtr->add(persistentStorageKey, path);
    }

    void EXPORT_API addToInMemoryStore(const char* path, OnError* errorCallback)
    {
        geoStorePtr->add(inMemoryStorageKey, path);
    }

    void EXPORT_API loadTile(int tileX, int tileY, int levelOfDetail,
                             OnMeshBuilt* meshCallback,
                             OnElementLoaded* elementCallback,
                             OnError* errorCallback)
    {
        utymap::QuadKey quadKey;
        quadKey.tileX = tileX;
        quadKey.tileY = tileY;
        quadKey.levelOfDetail = levelOfDetail;

        tileLoaderPtr->loadTile(quadKey, [&meshCallback](utymap::meshing::Mesh<double>& mesh) {
            meshCallback(mesh.name.data(),
                mesh.vertices.data(), mesh.vertices.size(),
                mesh.triangles.data(), mesh.triangles.size(),
                mesh.colors.data(), mesh.colors.size());
        }, [&elementCallback](utymap::entities::Element& element) {
            // TODO call elementCallback
        });
    }

    void EXPORT_API search(double latitude, double longitude, double radius,
                           OnElementLoaded* elementCallback,
                           OnError* errorCallback)
    {
        // TODO
    }
}
