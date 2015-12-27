#if _MSC_VER
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif

#include "QuadKey.hpp"
#include "TileLoader.hpp"
#include "meshing/MeshTypes.hpp"

#include <cstdint>

static TileLoader tileLoader;

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


    void EXPORT_API configure(const char* configPath)
    {
        tileLoader.configure(std::string(configPath));
    }

    void EXPORT_API loadTile(int tileX, int tileY, int levelOfDetail,
                             OnMeshBuilt* meshCallback, OnElementLoaded* elementCallback)
    {
        utymap::QuadKey quadKey;
        quadKey.tileX = tileX;
        quadKey.tileY = tileY;
        quadKey.levelOfDetail = levelOfDetail;

        // TODO
        tileLoader.loadTile(quadKey,
            [](utymap::meshing::Mesh<double>& mesh) -> {
                //  meshCallback()
            },
            [](utymap::entities::Element& element) -> {
                // elementCallback
            });
    }
}
