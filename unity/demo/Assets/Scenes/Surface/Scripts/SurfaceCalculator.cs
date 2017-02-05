using System;
using UnityEngine;
using UtyMap.Unity;

namespace Assets.Scenes.Surface.Scripts
{
    internal static class SurfaceCalculator
    {
        /// <summary> Max distance from origin before moving back. </summary>
        public const float MaxOriginDistance = 3000;

        /// <summary> Minimal LOD. </summary>
        public const int MinLod = 10;

        /// <summary> Maximal LOD. </summary>
        public const int MaxLod = 10;

        /// <summary> LOD step. </summary>
        public const float LodStep = 10000f / (MaxLod - MinLod);

        /// <summary> Origin in world coordinates. </summary>
        public static readonly Vector3 Origin = Vector3.zero;

        /// <summary> Origin in geo coordinates. </summary>
        public static GeoCoordinate GeoOrigin = new GeoCoordinate(28.2684, -16.6432);

        /// <summary> Checks whether position is far from origin. </summary>
        public static bool IsFar(Vector3 position)
        {
            return Vector2.Distance(new Vector2(position.x, position.z), Origin) < MaxOriginDistance;
        }

        /// <summary> Calculates LOD for given position. </summary>
        public static int CalculateLevelOfDetail(Vector3 position)
        {
            var distance = position.y;
            return Mathf.Clamp(MaxLod - (int)Math.Round(distance / LodStep), MinLod, MaxLod);
        }
    }
}
