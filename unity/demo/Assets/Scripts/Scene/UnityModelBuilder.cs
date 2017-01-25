using UnityEngine;
using UtyDepend;
using UtyMap.Unity;
using Mesh = UtyMap.Unity.Mesh;

namespace Assets.Scripts.Scene
{
    /// <summary>
    ///     Responsible for building Unity game objects from meshes and elements.
    /// </summary>
    internal class UnityModelBuilder
    {
        private readonly MaterialProvider _materialProvider;
        private readonly PlaceElementBuilder _placeElementBuilder;

        [Dependency]
        public UnityModelBuilder(MaterialProvider materialProvider)
        {
            _materialProvider = materialProvider;
            _placeElementBuilder = new PlaceElementBuilder(_materialProvider);
        }

        /// <inheritdoc />
        public void BuildElement(Tile tile, Element element)
        {
            // TODO Add more custom builders
            if (element.Styles["builders"].Contains("info"))
                _placeElementBuilder.Build(tile, element).transform.parent = tile.GameObject.transform;
        }

        /// <inheritdoc />
        public void BuildMesh(Tile tile, Mesh mesh)
        {
            var gameObject = new GameObject(mesh.Name);

            var uMesh = new UnityEngine.Mesh();
            uMesh.vertices = mesh.Vertices;
            uMesh.triangles = mesh.Triangles;
            uMesh.colors = mesh.Colors;
            uMesh.uv = mesh.Uvs;
            uMesh.uv2 = mesh.Uvs2;
            uMesh.uv3 = mesh.Uvs3;

            uMesh.RecalculateNormals();

            gameObject.isStatic = true;
            gameObject.AddComponent<MeshFilter>().mesh = uMesh;
            gameObject.AddComponent<MeshRenderer>().sharedMaterial =
                _materialProvider.GetSharedMaterial(@"Materials/Default");
            gameObject.AddComponent<MeshCollider>();
            gameObject.transform.parent = tile.GameObject.transform;
        }
    }
}