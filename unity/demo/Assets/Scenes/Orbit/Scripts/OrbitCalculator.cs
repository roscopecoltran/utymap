using System;
using UnityEngine;

namespace Assets.Scenes.Orbit.Scripts
{
    /// <summary> Responsible for calculating LOD of orbit scene. </summary>
    internal static class OrbitCalculator
    {
        /// <summary> Scaled radius of Earth in meters. </summary>
        /// <remarks> So far, we use 1:1000 scale. </remarks>
        public static readonly float Radius = 6371;

        /// <summary> Closest distance to sphere's surface. </summary>
        public static readonly float MinDistance = Radius * 1.1f;

        /// <summary> Minimal supported LOD. </summary>
        public static readonly int MinLod = 1;

        /// <summary> Maximal supported LOD. </summary>
        public static readonly int MaxLod = 7;

        /// <summary> Specifies LOD step. </summary>
        private static readonly float LodStep = (2 * Radius - MinDistance) / (MaxLod - MinLod);

        /// <summary> Calculates LOD for given position </summary>
        public static int CalculateLevelOfDetail(Vector3 position)
        {
            // TODO make it better: handle non-uniformly
            var distance = Vector3.Distance(position, Vector3.zero) - MinDistance;
            return Math.Max(MaxLod - (int) Math.Round(distance / LodStep), MinLod);
        }
    }
}
