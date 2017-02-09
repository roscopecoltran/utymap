using System;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Utils;

namespace Assets.Scenes.Surface.Scripts
{
    internal static class SurfaceCalculator
    {
        /// <summary> Max distance from origin before moving back. </summary>
        public const float MaxOriginDistance = 3000;

        /// <summary> Minimal LOD. </summary>
        public const int MinLod = 10;

        /// <summary> Maximal LOD. </summary>
        public const int MaxLod = 14;

        /// <summary> Origin in world coordinates. </summary>
        public static readonly Vector3 Origin = Vector3.zero;

        /// <summary> Origin in geo coordinates. </summary>
        public static GeoCoordinate GeoOrigin = GeoUtils.QuadKeyToBoundingBox(
            GeoUtils.CreateQuadKey(new GeoCoordinate(28.2684, -16.6432), MinLod)).Center();

        /// <summary> Checks whether position is far from origin. </summary>
        public static bool IsFar(Vector3 position)
        {
            return Vector2.Distance(new Vector2(position.x, position.z), Origin) < MaxOriginDistance;
        }

        /// <summary> Gets range tree with LODs </summary>
        /// <remarks> Modifies camera's field of view. </remarks>
        public static RangeTree<float, int> GetLodTree(Camera camera, Vector3 position)
        {
            const float sizeRatio = 1f;
            var tree = new RangeTree<float, int>();

            var maxDistance = position.y - 1;

            var aspectRatio = sizeRatio * Screen.height < Screen.width ? 1 / camera.aspect : 1;

            var fov = GetFieldOfView(GeoUtils.CreateQuadKey(GeoOrigin, MinLod), maxDistance, aspectRatio);
            maxDistance = float.MaxValue;

            for (int lod = MinLod; lod <= MaxLod; ++lod)
            {
                var frustumHeight = GetFrustumHeight(GeoUtils.CreateQuadKey(GeoOrigin, lod), aspectRatio);
                var distance = frustumHeight * 0.5f / Mathf.Tan(fov * 0.5f * Mathf.Deg2Rad);
                tree.Add(distance, maxDistance, lod);
                maxDistance = distance;
            }

            camera.fieldOfView = fov;

            return tree;
        }

        private static float GetFrustumHeight(QuadKey quadKey, float aspectRatio)
        {
            return GetGridSize(quadKey) * aspectRatio;
        }

        /// <summary> Gets field of view for given quadkey and distance. </summary>
        private static float GetFieldOfView(QuadKey quadKey, float distance, float aspectRatio)
        {
            return 2.0f * Mathf.Rad2Deg * Mathf.Atan(GetFrustumHeight(quadKey, aspectRatio) * 0.5f / distance);
        }

        /// <summary> Get side size of grid consists of 9 quadkeys. </summary>
        private static float GetGridSize(QuadKey quadKey)
        {
            var bbox = GeoUtils.QuadKeyToBoundingBox(quadKey);
            var bboxWidth = bbox.MaxPoint.Longitude - bbox.MinPoint.Longitude;
            var minPoint = new GeoCoordinate(bbox.MinPoint.Latitude, bbox.MinPoint.Longitude - bboxWidth);
            var maxPoint = new GeoCoordinate(bbox.MinPoint.Latitude, bbox.MaxPoint.Longitude + bboxWidth);
            return (float) GeoUtils.Distance(minPoint, maxPoint);
        }
    }
}
