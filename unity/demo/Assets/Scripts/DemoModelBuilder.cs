using UnityEngine;
using UtyDepend;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Explorer.Customization;
using Mesh = UtyMap.Unity.Core.Models.Mesh;

namespace Assets.Scripts
{
    /// <summary>
    ///     Responsible for building Unity game objects from meshes and elements.
    /// </summary>
    public class DemoModelBuilder : IModelBuilder
    {
        private readonly CustomizationService _customizationService;

        [Dependency]
        public DemoModelBuilder(CustomizationService customizationService)
        {
            _customizationService = customizationService;
        }

        /// <inheritdoc />
        public void BuildElement(Tile tile, Element element)
        {
            // TODO Use this method to build your objects in scene from map data
        }

        /// <inheritdoc />
        public void BuildMesh(Tile tile, Mesh mesh)
        {
            var gameObject = new GameObject(mesh.Name);

            var uMesh = new UnityEngine.Mesh();
            uMesh.vertices = mesh.Vertices;
            uMesh.triangles = mesh.Triangles;
            uMesh.colors = mesh.Colors;
            uMesh.uv = new Vector2[mesh.Vertices.Length];

            uMesh.RecalculateNormals();

            gameObject.isStatic = true;
            gameObject.AddComponent<MeshFilter>().mesh = uMesh;
            gameObject.AddComponent<MeshRenderer>().sharedMaterial =
                _customizationService.GetSharedMaterial(@"Materials/Default");
            gameObject.AddComponent<MeshCollider>();
            gameObject.transform.parent = tile.GameObject.transform;
        }
    }
}