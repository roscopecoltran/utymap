#if _MSC_VER
#define EXPORT_API __declspec(dllexport) 
#else
#define EXPORT_API
#endif

extern "C"
{
    typedef void OnMeshBuilt(const double* vertices, int vertexCount, const int* colors, int colorCount);

    void EXPORT_API loadTile(double minLat, double minLon, double maxLat, double maxLon,
                             int levelOfDetail, OnMeshBuilt* callback)
    {
        // TODO
    }
}
