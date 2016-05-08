#if _MSC_VER
#define EXPORT_API __declspec(dllexport)
#elif _GCC
#define EXPORT_API __attribute__((visibility("default")))
#else
#define EXPORT_API
#endif

#include "Application.hpp"

static Application* applicationPtr = nullptr;

extern "C"
{
    // Composes object graph.
    void EXPORT_API configure(const char* stringPath,  // path to string table directory
                              const char* dataPath,    // path to index directory
                              const char* elePath,     // path to elevation directory
                              OnError* errorCallback)  // completion callback.
    {
        applicationPtr = new Application(stringPath, dataPath, elePath, errorCallback);
    }

    // Performs cleanup.
    void EXPORT_API cleanup()
    {
        delete applicationPtr;
    }

    // Register stylesheet.
    void EXPORT_API registerStylesheet(const char* path)
    {
        applicationPtr->registerStylesheet(path);
    }

    // Register element visitor.
    void EXPORT_API registerElementBuilder(const char* name)
    {
        applicationPtr->registerElementBuilder(name);
    }

    // Adds data to persistent store.
    void EXPORT_API addToPersistentStore(const char* styleFile,   // style file
                                         const char* path,        // path to data
                                         int startLod,            // start zoom level
                                         int endLod,              // end zoom level
                                         OnError* errorCallback)  // completion callback
    {
        applicationPtr->addToPersistentStore(styleFile, path, utymap::LodRange(startLod, endLod), errorCallback);
    }

    // Adds data to in-memory store to specific level of details range.
    void EXPORT_API addToInMemoryStoreInRange(const char* styleFile,     // style file
                                              const char* path,          // path to data
                                              int startLod,              // start zoom level
                                              int endLod,                // end zoom level
                                              OnError* errorCallback)    // completion callback
   {
        applicationPtr->addToInMemoryStore(styleFile, path, utymap::LodRange(startLod, endLod), errorCallback);
   }

    // Adds data to in-memory store to specific level of details range.
    void EXPORT_API addToInMemoryStoreInBoundingBox(const char* styleFile,     // style file
                                                    const char* path,          // path to data
                                                    double minLat,             // minimal latitude
                                                    double minLon,             // minimal longitude
                                                    double maxLat,             // maximal latitude
                                                    double maxLon,             // maximal longitude
                                                    int startLod,              // start zoom level
                                                    int endLod,                // end zoom level
                                                    OnError* errorCallback)    // completion callback
    {
        utymap::BoundingBox bbox(utymap::GeoCoordinate(minLat, minLon), utymap::GeoCoordinate(maxLat,maxLon));
        utymap::LodRange lod(startLod, endLod);
        applicationPtr->addToInMemoryStore(styleFile, path, bbox, lod, errorCallback);
    }

    // Adds data to in-memory store to specific quadkey only.
    void EXPORT_API addToInMemoryStoreInQuadKey(const char* styleFile,     // style file
                                                const char* path,          // path to data
                                                int tileX,                 // tile x
                                                int tileY,                 // tile y,
                                                int levelOfDetail,         // level of detail
                                                OnError* errorCallback)    // completion callback
    {
        applicationPtr->addToInMemoryStore(styleFile, path, utymap::QuadKey{ levelOfDetail, tileX, tileY }, errorCallback);
    }


    // Adds element to in-memory store. NOTE: relation is not supported yet.
    void EXPORT_API addElementToInMemoryStore(const char* styleFile,     // style file
                                              std::uint64_t id,          // element id
                                              const double* vertices,    // vertex array
                                              int vertexLength,          // vertex array length,
                                              const char** tags,          // tag array
                                              int tagLength,             // tag array length
                                              int startLod,              // start zoom level
                                              int endLod,                // end zoom level
                                              OnError* errorCallback)    // completion callback
    {
        utymap::LodRange lod(startLod, endLod);
        std::vector<utymap::entities::Tag> elementTags;
        elementTags.reserve(tagLength / 2);
        for (int i = 0; i < tagLength; i+=2) {
            auto keyId = applicationPtr->getStringId(tags[i]);
            auto valueId = applicationPtr->getStringId(tags[i + 1]);
            elementTags.push_back(utymap::entities::Tag(keyId, valueId));
        }

        // Node
        if (vertexLength / 2 == 1) {
            utymap::entities::Node node;
            node.id = id;
            node.tags = elementTags;
            node.coordinate = utymap::GeoCoordinate(vertices[0], vertices[1]);
            applicationPtr->addInMemoryStore(styleFile, node, lod, errorCallback);
            return;
        }

        std::vector<utymap::GeoCoordinate> coordinates;
        coordinates.reserve(vertexLength / 2);
        for (int i = 0; i < vertexLength; i+=2) {
            coordinates.push_back(utymap::GeoCoordinate(vertices[i], vertices[i + 1]));
        }

        // Way or Area
        if (coordinates[0] == coordinates[coordinates.size() - 1]) {
            utymap::entities::Area area;
            area.id = id;
            area.coordinates = coordinates;
            area.tags = elementTags;
            applicationPtr->addInMemoryStore(styleFile, area, lod, errorCallback);
        } else {
            utymap::entities::Way way;
            way.id = id;
            way.coordinates = coordinates;
            way.tags = elementTags;
            applicationPtr->addInMemoryStore(styleFile, way, lod, errorCallback);
        }
    }

    // Loads quadkey.
    void EXPORT_API loadQuadKey(const char* styleFile,                   // style file
                                int tileX, int tileY, int levelOfDetail, // quadkey info
                                OnMeshBuilt* meshCallback,               // mesh callback
                                OnElementLoaded* elementCallback,        // element callback
                                OnError* errorCallback)                  // completion callback
    {
        utymap::QuadKey quadKey = { levelOfDetail, tileX, tileY };
        applicationPtr->loadQuadKey(styleFile, quadKey, meshCallback, elementCallback, errorCallback);
    }

    // Checks whether there is data for given quadkey
    bool EXPORT_API hasData(int tileX, int tileY, int levelOfDetail) // quadkey info
    {
        return applicationPtr->hasData(utymap::QuadKey{ levelOfDetail, tileX, tileY });
    }
}
