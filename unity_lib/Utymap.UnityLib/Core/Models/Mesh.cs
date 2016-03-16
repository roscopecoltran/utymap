using UnityEngine;

namespace Utymap.UnityLib.Core.Models
{
    /// <summary> Represents mesh with all geometry needed. </summary>
    public class Mesh
    {
        public readonly string Name;
        public readonly Vector3[] Vertices;
        public readonly int[] Triangles;
        public readonly Color32[] Colors;

        public Mesh(string name, Vector3[] vertices, int[] triangles, Color32[] colors)
        {
            Name = name;
            Vertices = vertices;
            Triangles = triangles;
            Colors = colors;
        }

    }
}
