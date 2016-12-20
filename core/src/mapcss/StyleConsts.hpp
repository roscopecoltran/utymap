#ifndef MAPCSS_STYLECONSTS_HPP_INCLUDED
#define MAPCSS_STYLECONSTS_HPP_INCLUDED

#include <string>

namespace utymap { namespace mapcss
{

/// Specifies mapcss consts used by built-in builders.
struct StyleConsts final {

    static const std::string& EleNoiseFreqKey();
    static const std::string& ColorNoiseFreqKey();

    static const std::string& GradientKey();

    static const std::string& TextureIndexKey();
    static const std::string& TextureTypeKey();
    static const std::string& TextureScaleKey();

    static const std::string& MaxAreaKey();
    static const std::string& HeightOffsetKey();
    static const std::string& LayerPriorityKey();
    static const std::string& MeshNameKey();
    static const std::string& MeshExtrasKey();
    static const std::string& GridCellSize();

    static const std::string& TerrainLayerKey();
    
    static const std::string& HeightKey();
    static const std::string& WidthKey();
    static const std::string& LengthKey();
    static const std::string& RadiusKey();

    static const std::string& LevelKey();
    static const std::string& DimenstionKey();
};

}}

#endif // MAPCSS_STYLECONSTS_HPP_INCLUDED
