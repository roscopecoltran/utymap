#ifndef CALLBACKS_HPP_DEFINED
#define CALLBACKS_HPP_DEFINED

#include <cstdint>

// Called when mesh is built.
typedef void OnMeshBuilt(const char* name,
                         const double* vertices, int vertexSize,
                         const int* triangles, int triSize,
                         const int* colors, int colorSize);

// Called when element is loaded.
typedef void OnElementLoaded(std::uint64_t id, const char** tags, int tagsSize,
                             const double* vertices, int vertexSize,
                             const char** style, int styleSize);

// Called when operation is completed.
typedef void OnError(const char* errorMessage);

#endif // CALLBACKS_HPP_DEFINED
