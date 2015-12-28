#if _MSC_VER
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif

#include "QuadKey.hpp"
#include "TileLoader.hpp"
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
static utymap::index::StyleFilter* styleFilter;

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

    void EXPORT_API configure(const char* stringPath, const char* stylePath, const char* dataPath)
    {
        std::ifstream styleFile(stylePath);
        utymap::mapcss::Parser parser;
        utymap::mapcss::StyleSheet stylesheet = parser.parse(styleFile);
        if (!parser.getError().empty())
            throw std::domain_error("Cannot parse mapcss.");

        stringTable = new utymap::index::StringTable(stringPath, stringPath);
        styleFilter = new utymap::index::StyleFilter(stylesheet, *stringTable);

        geoStore = new utymap::index::GeoStore(dataPath, *stringTable);
        tileLoader = new utymap::TileLoader(*geoStore);
    }

    void EXPORT_API cleanup()
    {
        delete tileLoader;
        delete geoStore;
        delete stringTable;
        delete styleFilter;
    }

    void EXPORT_API loadTile(int tileX, int tileY, int levelOfDetail,
        OnMeshBuilt* meshCallback, OnElementLoaded* elementCallback)
    {
        utymap::QuadKey quadKey;
        quadKey.tileX = tileX;
        quadKey.tileY = tileY;
        quadKey.levelOfDetail = levelOfDetail;

        // TODO
        tileLoader->loadTile(quadKey, [&meshCallback](utymap::meshing::Mesh<double>& mesh) {
            meshCallback(mesh.name.data(),
                mesh.vertices.data(), mesh.vertices.size(),
                mesh.triangles.data(), mesh.triangles.size(),
                mesh.colors.data(), mesh.colors.size());
        }, [&elementCallback](utymap::entities::Element& element) {
            // TODO call elementCallback
        });
    }
}
