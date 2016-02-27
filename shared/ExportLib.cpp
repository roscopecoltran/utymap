#if _MSC_VER
#define EXPORT_API __declspec(dllexport)
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
                              OnError* errorCallback)  // completion callback.
   {
       applicationPtr = new Application(stringPath, dataPath, errorCallback);
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
        applicationPtr->addToPersistentStore(styleFile, path, utymap::index::LodRange(startLod, endLod), errorCallback);
    }

    // Adds data to in-memory store.
    void EXPORT_API addToInMemoryStore(const char* styleFile,     // style file
                                       const char* path,          // path to data
                                       int startLod,              // start zoom level
                                       int endLod,                // end zoom level
                                       OnError* errorCallback)    // Completion callback
   {
        applicationPtr->addToInMemoryStore(styleFile, path, utymap::index::LodRange(startLod, endLod), errorCallback);
   }

    // Loads tile.
    void EXPORT_API loadTile(const char* styleFile,                   // style file
                             int tileX, int tileY, int levelOfDetail, // quadkey info
                             OnMeshBuilt* meshCallback,               // mesh callback
                             OnElementLoaded* elementCallback,        // element callback
                             OnError* errorCallback)                  // completion callback
    {
        utymap::QuadKey quadKey;
        quadKey.tileX = tileX;
        quadKey.tileY = tileY;
        quadKey.levelOfDetail = levelOfDetail;
        applicationPtr->loadTile(styleFile, quadKey, meshCallback, elementCallback, errorCallback);
    }
}
