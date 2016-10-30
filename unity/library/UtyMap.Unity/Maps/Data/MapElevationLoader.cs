using UtyDepend.Config;
using UtyMap.Unity.Core;

namespace UtyMap.Unity.Maps.Data
{
    /// <summary> Exposes elevation api for external usage. </summary>
    public class MapElevationLoader : IConfigurable
    {
        private ElevationDataType _eleDataType;

        /// <summary> Loads elevation for given quadkey and point. </summary>
        public double Load(QuadKey quadKey, GeoCoordinate coordinate)
        {
            return CoreLibrary.GetElevation(quadKey, _eleDataType, coordinate);
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _eleDataType = (ElevationDataType)configSection.GetInt("data/elevation/type", 0);
        }
    }
}
