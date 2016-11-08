#ifndef BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED
#define BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED

#include "builders/generators/AbstractGenerator.hpp"
#include "builders/generators/CylinderGenerator.hpp"
#include "builders/generators/IcoSphereGenerator.hpp"

namespace utymap { namespace builders {

/// Generates tree.
class TreeGenerator
{
public:
    TreeGenerator(const utymap::builders::BuilderContext& builderContext,
                  utymap::math::Mesh& mesh,
                  const utymap::mapcss::Style& style,
                  const utymap::mapcss::ColorGradient& trunkGradient,
                  const utymap::mapcss::ColorGradient& foliageGradient,
                  const utymap::mapcss::TextureRegion& trunkTextureRegion,
                  const utymap::mapcss::TextureRegion& foliageTextureRegion) :
            builderContext_(builderContext),
            trunkGeneratorMeshContext(mesh, style, trunkGradient, trunkTextureRegion),
            foliageGeneratorMeshContext(mesh, style, foliageGradient, foliageTextureRegion),
            trunkGenerator(builderContext, trunkGeneratorMeshContext),
            foliageGenerator(builderContext, foliageGeneratorMeshContext),
            position_(),
            foliageSize_(),
            trunkHeight_(0),
            trunkRadius_(0)
    {
    }

    TreeGenerator(const TreeGenerator&) = delete;

    /// Sets position of tree
    TreeGenerator& setPosition(const utymap::math::Vector3& position)
    {
        position_ = position;
        return *this;
    }

    /// Sets heigh of trunk
    TreeGenerator& setTrunkHeight(double height)
    {
        trunkHeight_ = height;
        return *this;
    }

    /// Sets radius of trunk
    TreeGenerator& setTrunkRadius(double radius)
    {
        trunkRadius_ = radius;
        return *this;
    }

    /// Sets radius of foliage
    TreeGenerator& setFoliageSize(const utymap::math::Vector3& size)
    {
        foliageSize_ = size;
        return *this;
    }

    /// Sets trunk color noise freq
    TreeGenerator& setTrunkColorNoiseFreq(double noiseFreq)
    {
        trunkGenerator.setColorNoiseFreq(noiseFreq);
        return *this;
    }

    /// Sets foliage color noise freq.
    TreeGenerator& setFoliageColorNoiseFreq(double noiseFreq)
    {
        foliageGenerator.setColorNoiseFreq(noiseFreq);
        return *this;
    }

    /// Sets trunk texture scale
    TreeGenerator& setTrunkTextureScale(double scale)
    {
        trunkGeneratorMeshContext.appearanceOptions.textureScale = scale;
        return *this;
    }

    /// Sets foliage texture scale.
    TreeGenerator& setFoliageTextureScale(double scale)
    {
        foliageGeneratorMeshContext.appearanceOptions.textureScale = scale;
        return *this;
    }

    void generate()
    {
        // generate trunk
        trunkGenerator
            .setCenter(position_)
            .setHeight(trunkHeight_)
            .setRadius(trunkRadius_)
            .setMaxSegmentHeight(5)
            .setRadialSegments(7)
            .setVertexNoiseFreq(0.1f)
            .generate();

        builderContext_.meshBuilder.writeTextureMappingInfo(trunkGeneratorMeshContext.mesh, 
                                                            trunkGeneratorMeshContext.appearanceOptions);

        // generate foliage
        foliageGenerator
            .setCenter(utymap::math::Vector3(
                position_.x,
                position_.y + trunkHeight_ + foliageSize_.y,
                position_.z))
            .setSize(foliageSize_)
            .setRecursionLevel(1)
            .setVertexNoiseFreq(0.1f)
            .generate();

        builderContext_.meshBuilder.writeTextureMappingInfo(foliageGeneratorMeshContext.mesh,
                                                            foliageGeneratorMeshContext.appearanceOptions);
    }

private:
    const utymap::builders::BuilderContext& builderContext_;
    utymap::builders::MeshContext trunkGeneratorMeshContext;
    utymap::builders::MeshContext foliageGeneratorMeshContext;
    CylinderGenerator trunkGenerator;
    IcoSphereGenerator foliageGenerator;
    utymap::math::Vector3 position_;
    utymap::math::Vector3 foliageSize_;
    double trunkHeight_, trunkRadius_;
};
}}

#endif // BUILDERS_GENERATORS_TREEGENERATOR_HPP_DEFINED
