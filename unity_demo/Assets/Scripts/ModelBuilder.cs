using System;
using System.Collections.Generic;
using UnityEngine;
using Utymap.UnityLib.Core.Models;
using Utymap.UnityLib.Core.Tiling;
using Mesh = Utymap.UnityLib.Core.Models.Mesh;

namespace Assets.Scripts
{
    public class ModelBuilder : IModelBuilder
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
