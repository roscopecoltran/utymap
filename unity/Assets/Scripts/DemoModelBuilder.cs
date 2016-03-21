using UnityEngine;
using Assets.UtymapLib.Core.Models;
using Assets.UtymapLib.Core.Tiling;
using Mesh = Assets.UtymapLib.Core.Models.Mesh;

namespace Assets.Scripts
{
    public class DemoModelBuilder : IModelBuilder
    {
        public void BuildElement(Tile tile, Element element)
        {
            // TODO
        }

        public void BuildMesh(Tile tile, Mesh mesh)
        {
            var gameObject = new GameObject(mesh.Name);

            var uMesh = new UnityEngine.Mesh();
            uMesh.vertices = mesh.Vertices;
            uMesh.triangles = mesh.Triangles;
            uMesh.colors = mesh.Colors;
            
            uMesh.RecalculateNormals();

            gameObject.AddComponent<MeshFilter>().mesh = uMesh;
            gameObject.AddComponent<MeshRenderer>().material = Resources.Load<Material>(@"Materials/Default");
            gameObject.AddComponent<MeshCollider>();
            gameObject.transform.parent = tile.GameObject.transform;
        }
    }
}
