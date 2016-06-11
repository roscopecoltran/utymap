using System;
using UnityEngine;
using UtyMap.Unity.Core.Utils;

namespace UtyMap.Unity.Core
{
    /// <summary> Specifies projection functionality. </summary>
    public interface IProjection
    {
        /// <summary> Converts GeoCoordinate to world coordinates. </summary>
        Vector3 Project(GeoCoordinate coordinate, double height);

        /// <summary> Projects world coordinate to geocoordinate. </summary>
        GeoCoordinate Project(Vector3 worldCoordinate);
    }

    /// <summary> Projects GeoCoordinate to 2D plane. </summary>
    public class CartesianProjection : IProjection
    {
        private readonly GeoCoordinate _worldZeroPoint;

        /// <summary> Creates instance of <see cref="CartesianProjection"/>. </summary>
        /// <param name="worldZeroPoint"> GeoCoordinate for (0, 0) point. </param>
        public CartesianProjection(GeoCoordinate worldZeroPoint)
        {
            _worldZeroPoint = worldZeroPoint;
        }

        /// <inheritdoc />
        public Vector3 Project(GeoCoordinate coordinate, double height)
        {
            var point = GeoUtils.ToMapCoordinate(_worldZeroPoint, coordinate);
            return new Vector3(point.x, (float)height, point.y);
        }

        /// <inheritdoc />
        public GeoCoordinate Project(Vector3 worldCoordinate)
        {
            return GeoUtils.ToGeoCoordinate(_worldZeroPoint, worldCoordinate);
        }
    }

    /// <summary> Projects GeoCoordinate to sphere. </summary>
    public class SphericalProjection : IProjection
    {
        private readonly float _radius;

        /// <summary> Creates instance of <see cref="SphericalProjection"/>. </summary>
        /// <param name="radius"> Radius of sphere. </param>
        public SphericalProjection(float radius)
        {
            _radius = radius;
        }

        /// <inheritdoc />
        public Vector3 Project(GeoCoordinate coordinate, double height)
        {
            // the x-axis goes through long,lat (0,0), so longitude 0 meets the equator;
            // the y-axis goes through (0,90);
            // and the z-axis goes through the pol
            // x = R * cos(lat) * cos(lon)
            // y = R * cos(lat) * sin(lon)
            // z = R * sin(lat)

            double radius = _radius + height;
            double latRad = (Math.PI / 180) * coordinate.Latitude;
            double lonRad = (Math.PI / 180) * coordinate.Longitude;
            float x = (float)(radius * Math.Cos(latRad) * Math.Cos(lonRad));
            float y = (float)(radius * Math.Cos(latRad) * Math.Sin(lonRad));
            float z = (float)(radius * Math.Sin(latRad));

            return new Vector3(x, z, y);
        }

        /// <inheritdoc />
        public GeoCoordinate Project(Vector3 worldCoordinate)
        {
            throw new NotImplementedException();
        }
    }
}
