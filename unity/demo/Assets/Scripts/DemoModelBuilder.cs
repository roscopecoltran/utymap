using System;
using System.Linq;
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
        private readonly PlaceElementBuilder _placeElementBuilder;

        [Dependency]
        public DemoModelBuilder(CustomizationService customizationService)
        {
            _customizationService = customizationService;
            _placeElementBuilder = new PlaceElementBuilder(customizationService);
        }

        /// <inheritdoc />
        public void BuildElement(Tile tile, Element element)
        {
            EnsureTile(tile);

            if (element.Styles["builders"].Contains("info"))
                _placeElementBuilder.Build(tile, element).transform.parent = tile.GameObject.transform;
        }

        /// <inheritdoc />
        public void BuildMesh(Tile tile, Mesh mesh)
        {
            EnsureTile(tile);

            var gameObject = new GameObject(mesh.Name);

            var uMesh = new UnityEngine.Mesh();
            uMesh.vertices = mesh.Vertices;
            uMesh.triangles = mesh.Triangles;
            uMesh.colors = mesh.Colors;
            uMesh.uv = mesh.Textures;

            uMesh.RecalculateNormals();

            gameObject.isStatic = true;
            gameObject.AddComponent<MeshFilter>().mesh = uMesh;
            gameObject.AddComponent<MeshRenderer>().sharedMaterial =
                _customizationService.GetSharedMaterial(@"Materials/Default");
            gameObject.AddComponent<MeshCollider>();
            gameObject.transform.parent = tile.GameObject.transform;
        }

        /// <summary> Ensures that tile's game object is created. </summary>
        private void EnsureTile(Tile tile)
        {
            if (tile.GameObject == null)
                tile.GameObject = new GameObject("tile");
        }
    }
}