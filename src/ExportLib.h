#if _MSC_VER
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif

#include "QuadKey.hpp"
#include "TileLoader.hpp"
#include "heightmap/FlatElevationProvider.hpp"
#include "index/GeoStore.hpp"
#include "index/StringTable.hpp"
#include "index/StyleFilter.hpp"
#include "mapcss/MapCssParser.hpp"
#include "mapcss/StyleSheet.hpp"
#include "meshing/MeshTypes.hpp"

#include <cstdint>
#include <fstream>

static utymap::TileLoader* tileLoader = nullptr;
static utymap::index::GeoStore* geoStore = nullptr;
static utymap::index::StringTable* stringTable = nullptr;
static utymap::heightmap::ElevationProvider<double>* eleProvider = nullptr;

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
    typedef void OnCompleted(int resultCode);

    int EXPORT_API configure(const char* stringPath, // path to string table directory
                              const char* stylePath, // path to mapcss file
                              const char* dataPath)  // path to index directory
    {
        std::ifstream styleFile(stylePath);
        utymap::mapcss::Parser parser;
        utymap::mapcss::StyleSheet stylesheet = parser.parse(styleFile);
        if (!parser.getError().empty())
            return 1;

        eleProvider = new utymap::heightmap::FlatElevationProvider<double>();

        stringTable = new utymap::index::StringTable(stringPath, stringPath);
        geoStore = new utymap::index::GeoStore(dataPath, stylesheet, *stringTable);
        tileLoader = new utymap::TileLoader(*geoStore, stylesheet, *stringTable, *eleProvider);
        return 0;
    }

    void EXPORT_API cleanup()
    {
        delete tileLoader;
        delete geoStore;
        delete stringTable;
        delete eleProvider;
    }

    void EXPORT_API loadTile(int tileX, int tileY, int levelOfDetail,
                             OnMeshBuilt* meshCallback, 
                             OnElementLoaded* elementCallback, 
                             OnCompleted* completedCallback)
    {
        utymap::QuadKey quadKey;
        quadKey.tileX = tileX;
        quadKey.tileY = tileY;
        quadKey.levelOfDetail = levelOfDetail;

        tileLoader->loadTile(quadKey, [&meshCallback](utymap::meshing::Mesh<double>& mesh) {
            meshCallback(mesh.name.data(),
                mesh.vertices.data(), mesh.vertices.size(),
                mesh.triangles.data(), mesh.triangles.size(),
                mesh.colors.data(), mesh.colors.size());
        }, [&elementCallback](utymap::entities::Element& element) {
            // TODO call elementCallback
        });

        completedCallback(0);
    }

    void EXPORT_API search(double latitude, double longitude, double radius, 
                           OnElementLoaded* elementCallback,
                           OnCompleted* completedCallback)
    {
        // TODO
    }
}
