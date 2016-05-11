#ifndef BUILDERS_MESHCONTEXT_HPP_DEFINED
#define BUILDERS_MESHCONTEXT_HPP_DEFINED

#include "mapcss/Style.hpp"
#include "meshing/MeshTypes.hpp"

namespace utymap { namespace builders {

// Provides the way to access mesh specific data.
struct MeshContext
{
  utymap::meshing::Mesh& mesh;
  const utymap::mapcss::Style& style;

  MeshContext(utymap::meshing::Mesh& mesh,
              const utymap::mapcss::Style& style) :
      mesh(mesh), style(style)
  {
  }
};

}}
#endif // BUILDERS_MESHCONTEXT_HPP_DEFINED
