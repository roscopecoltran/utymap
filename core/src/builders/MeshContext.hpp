#ifndef BUILDERS_MESHCONTEXT_HPP_DEFINED
#define BUILDERS_MESHCONTEXT_HPP_DEFINED

#include "mapcss/ColorGradient.hpp"
#include "mapcss/Style.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"

namespace utymap { namespace builders {

// Provides the way to access mesh specific data.
struct MeshContext
{
  utymap::meshing::Mesh& mesh;
  const utymap::mapcss::Style& style;
  utymap::meshing::MeshBuilder::GeometryOptions geometryOptions;
  utymap::meshing::MeshBuilder::AppearanceOptions appearanceOptions;

  MeshContext(utymap::meshing::Mesh& mesh,
              const utymap::mapcss::Style& style,
              const utymap::mapcss::ColorGradient& gradient) :
      mesh(mesh), 
      style(style),
      geometryOptions(0, 0, std::numeric_limits<double>::lowest(), 0),
      appearanceOptions(gradient, 0, utymap::meshing::Rectangle(), 0)
  {
  }
};

}}
#endif // BUILDERS_MESHCONTEXT_HPP_DEFINED
