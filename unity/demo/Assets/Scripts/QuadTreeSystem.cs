using System;
using System.Collections.Generic;
using ActionStreetMap.Core.Geometry.Triangle.Geometry;
using ActionStreetMap.Core.Geometry.Triangle.Meshing;
using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Utils;

namespace Assets.Scripts
{
    class QuadTreeSystem
    {
        public static GameObject BuildQuadTree(GameObject parent, QuadKey quadKey, IProjection projection)
        {
            var boundingBox = GeoUtils.QuadKeyToBoundingBox(quadKey);
            using (var polygon = new Polygon(256))
            {
                var list = new List<Point>()
                {
                    new Point(boundingBox.MinPoint.Longitude, boundingBox.MinPoint.Latitude),
                    new Point(boundingBox.MinPoint.Longitude, boundingBox.MaxPoint.Latitude),
                    new Point(boundingBox.MaxPoint.Longitude, boundingBox.MaxPoint.Latitude),
                    new Point(boundingBox.MaxPoint.Longitude, boundingBox.MinPoint.Latitude),
                };
                polygon.AddContour(list);

                var mesh = polygon.Triangulate(new ConstraintOptions
                    {
                        ConformingDelaunay = false,
                        SegmentSplitting = 0
                    },
                    new QualityOptions
                    {
                        MaximumArea = quadKey.LevelOfDetail < 10 ? (20f / quadKey.LevelOfDetail) : 0.000001
                    });

                Mesh uMesh = new Mesh();
                List<Vector3> vertices = new List<Vector3>();
                List<int> triangles = new List<int>();
                List<Color> colors = new List<Color>();
                //List<Vector2> uvs = new List<Vector2>();
                var color = GetColor(quadKey);
                foreach (var triangle in mesh.Triangles)
                {
                    for (int i = 2; i >= 0; --i)
                    {
                        var v = triangle.GetVertex(i);
                        vertices.Add(projection.Project(new GeoCoordinate(v.Y, v.X), 0));
                        triangles.Add(triangles.Count);
                       // uvs.Add(new Vector2(
                        //    (float)(Math.Cos(v.Y * Mathf.Deg2Rad) * Math.Cos(v.X * Mathf.Deg2Rad)), 
                        //    (float)(Math.Cos(v.Y * Mathf.Deg2Rad) * Math.Sin(v.X * Mathf.Deg2Rad))));
                        colors.Add(color);
                    }
                }

                uMesh.vertices = vertices.ToArray();
                uMesh.triangles = triangles.ToArray();
                uMesh.colors = colors.ToArray();
                uMesh.uv = new Vector2[vertices.Count];
                uMesh.RecalculateNormals();
                uMesh.RecalculateBounds();

                var gameObject = new GameObject(quadKey.ToString());
                gameObject.AddComponent<MeshFilter>().mesh = uMesh;
                gameObject.AddComponent<MeshRenderer>().sharedMaterial = Resources.Load<Material>(@"Materials/Default");
                gameObject.AddComponent<MeshCollider>();
                gameObject.transform.parent = parent.transform;

                return gameObject;
            }
        }

        private static Color GetColor(QuadKey quadKey)
        {
            var code = quadKey.ToString()[quadKey.ToString().Length - 1];

            if (code == '0')
                return Color.red;
            if (code == '1')
                return Color.blue;
            if (code == '2')
                return Color.yellow;

            return Color.green;
        }
    }
}
