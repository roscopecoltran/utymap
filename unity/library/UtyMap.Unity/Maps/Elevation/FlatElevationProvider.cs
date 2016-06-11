using UtyMap.Unity.Core;
using UnityEngine;
using UtyRx;

namespace UtyMap.Unity.Maps.Elevation
{
    /// <summary>
    ///     Elevation provider which returns always height as zero.
    /// </summary>
    public class FlatElevationProvider : IElevationProvider
    {
        /// <inheritdoc />
        public bool HasElevation(BoundingBox bbox)
        {
            return true;
        }

        /// <inheritdoc />
        public IObservable<Unit> Download(BoundingBox bbox)
        {
            return Observable.Empty<Unit>();
        }

        /// <inheritdoc />
        public float GetElevation(GeoCoordinate coordinate)
        {
            return 0;
        }

        /// <inheritdoc />
        public float GetElevation(Vector2 point)
        {
            return 0;
        }

        /// <inheritdoc />
        public float GetElevation(float x, float y)
        {
            return 0;
        }
    }
}