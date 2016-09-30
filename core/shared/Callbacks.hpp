#ifndef CALLBACKS_HPP_DEFINED
#define CALLBACKS_HPP_DEFINED

#include <cstdint>

/// Callback which is called when mesh is built.
typedef void OnMeshBuilt(const char* name,
                         const double* vertices, int vertexSize,
                         const int* triangles, int triSize,
                         const int* colors, int colorSize,
                         const double* uvs, int uvSize);

/// Callback which is called when element is loaded.
typedef void OnElementLoaded(std::uint64_t id, const char** tags, int tagsSize,
                             const double* vertices, int vertexSize,
                             const char** style, int styleSize);

/// Callback which is called when operation is completed.
typedef void OnError(const char* errorMessage);

#endif // CALLBACKS_HPP_DEFINED
