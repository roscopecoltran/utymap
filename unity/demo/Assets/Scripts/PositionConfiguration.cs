using UtyMap.Unity.Core;

namespace Assets.Scripts
{
    /// <summary> Holds position used across multiple scenes. </summary>
    /// <remarks> This is workaround. </remarks>
    internal static class PositionConfiguration
    {
        public static string PlaceName = "Moscow, Red Square";
        public static GeoCoordinate StartPosition = new GeoCoordinate(55.75395, 37.62064);
    }
}
