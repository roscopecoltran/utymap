#if _MSC_VER
#define EXPORT_API __declspec(dllexport) 
#else
#define EXPORT_API
#endif

#include "QuadKey.hpp"

extern "C"
{
    typedef void OnMeshBuilt(const double* vertices, int vertexCount, const int* colors, int colorCount);

    void EXPORT_API loadTile(int tileX, int tileY, int levelOfDetail, OnMeshBuilt* callback)
    {
        utymap::QuadKey quadKey;
        quadKey.tileX = tileX;
        quadKey.tileY = tileY;
        quadKey.levelOfDetail = levelOfDetail;

        // TODO
    }
}
