#ifndef BUILDERS_MESHCONTEXT_HPP_DEFINED
#define BUILDERS_MESHCONTEXT_HPP_DEFINED

#include "mapcss/ColorGradient.hpp"
#include "mapcss/Style.hpp"
#include "meshing/MeshTypes.hpp"

namespace utymap { namespace builders {

// Provides the way to access mesh specific data.
struct MeshContext
{
  utymap::meshing::Mesh& mesh;
  const utymap::mapcss::Style& style;
  const utymap::mapcss::ColorGradient& gradient;

  MeshContext(utymap::meshing::Mesh& mesh,
              const utymap::mapcss::Style& style,
              const utymap::mapcss::ColorGradient& gradient) :
      mesh(mesh), style(style), gradient(gradient)
  {
  }
};

}}
#endif // BUILDERS_MESHCONTEXT_HPP_DEFINED
