using System;

namespace UtyMap.Unity.Core
{
    /// <summary>
    ///     Represent geo position which is produced by location services (e.g. GLONASS, GPS).
    /// </summary>
    public class GeoPosition
    {
        /// <summary> Geo coordinate. </summary>
        public GeoCoordinate Coordinate;

        /// <summary> Time.</summary>
        public TimeSpan Time = TimeSpan.MinValue;
    }
}