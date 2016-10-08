#ifndef BUILDERS_MESHCONTEXT_HPP_DEFINED
#define BUILDERS_MESHCONTEXT_HPP_DEFINED

#include "mapcss/ColorGradient.hpp"
#include "mapcss/Style.hpp"
#include "meshing/MeshBuilder.hpp"
#include "meshing/MeshTypes.hpp"
#include "utils/GradientUtils.hpp"

namespace utymap { namespace builders {

/// Provides the way to access mesh specific data.
struct MeshContext
{
  utymap::meshing::Mesh& mesh;
  const utymap::mapcss::Style& style;
  utymap::meshing::MeshBuilder::GeometryOptions geometryOptions;
  utymap::meshing::MeshBuilder::AppearanceOptions appearanceOptions;

  MeshContext(utymap::meshing::Mesh& mesh,
              const utymap::mapcss::Style& style,
              const utymap::mapcss::ColorGradient& gradient,
              const utymap::mapcss::TextureRegion& region) :
      mesh(mesh), 
      style(style),
      geometryOptions(0, 0, std::numeric_limits<double>::lowest(), 0),
      appearanceOptions(gradient, 0, 0, region, 0)
  {
  }

  static MeshContext create(utymap::meshing::Mesh& mesh,
                            const utymap::mapcss::Style& style,
                            const utymap::mapcss::StyleProvider& styleProvider,
                            const std::string& colorKey,
                            const std::string& textureIndexKey,
                            const std::string& textureTypeKey,
                            const std::string& textureScaleKey,
                            std::uint64_t seed = 0)
  {
      auto textureIndex = static_cast<std::uint16_t>(style.getValue(textureIndexKey));
      MeshContext meshContext(
          mesh, 
          style,
          utymap::utils::GradientUtils::evaluateGradient(styleProvider, style, colorKey),
          styleProvider.getTexture(textureIndex, style.getString(textureTypeKey))
                       .random(seed));

      meshContext.appearanceOptions.textureId = textureIndex;
      meshContext.appearanceOptions.textureScale = style.getValue(textureScaleKey);

      return std::move(meshContext);
  }
};

}}
#endif // BUILDERS_MESHCONTEXT_HPP_DEFINED
