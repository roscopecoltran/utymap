using UnityEngine;
using Utymap.UnityLib.Core;
using Utymap.UnityLib.Core.Utils;

namespace Utymap.UnityLib.Tests.Helpers
{
    public class CartesianProjection : IProjection
    {
        private readonly GeoCoordinate _worldZeroPoint;

        public CartesianProjection(GeoCoordinate worldZeroPoint)
        {
            _worldZeroPoint = worldZeroPoint;
        }

        public Vector3 Project(GeoCoordinate coordinate, double height)
        {
            var point =  GeoUtils.ToMapCoordinate(_worldZeroPoint, coordinate);
            return new Vector3(point.x, (float) height, point.y);
        }
    }
}
