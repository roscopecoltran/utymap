using System.Collections.Generic;
using Assets.UtymapLib.Core;
using Assets.UtymapLib.Core.Models;
using Assets.UtymapLib.Core.Tiling;
using Assets.UtymapLib.Core.Utils;
using Assets.UtymapLib.Infrastructure.Diagnostic;
using Assets.UtymapLib.Infrastructure.Primitives;
using Assets.UtymapLib.Infrastructure.Reactive;
using UnityEngine;
using Mesh = Assets.UtymapLib.Core.Models.Mesh;

namespace Assets.UtymapLib.Maps.Loader
{
    /// <summary>
    /// Adapts map tile data received from utymap API to the type used by the app.
    /// </summary>
    internal class MapTileAdapter
    {
        private const string TraceCategory = "mapdata.loader";

        private readonly Tile _tile;
        private readonly IObserver<Union<Element, Mesh>> _observer;
        private readonly ITrace _trace;

        public MapTileAdapter(Tile tile, IObserver<Union<Element, Mesh>> observer, ITrace trace)
        {
            _tile = tile;
            _observer = observer;
            _trace = trace;
        }

        /// <summary> Adapts mesh data received from utymap. </summary>
        public void AdaptMesh(string name, double[] vertices, int vertexCount, int[] triangles, 
            int triangleCount, int[] colors, int colorCount)
        {
            Vector3[] worldPoints;
            Color[] unityColors;

            // NOTE process terrain differently to emulate flat shading effect by avoiding 
            // triangles to share the same vertex. Remove "if" branch if you don't need it
            if (name.Contains("terrain"))
            {
                worldPoints = new Vector3[triangleCount];
                unityColors = new Color[triangleCount];
                for (int i = 0; i < triangles.Length; ++i)
                {
                    int vertIndex = triangles[i] * 3;
                    worldPoints[i] = _tile.Projection
                        .Project(new GeoCoordinate(vertices[vertIndex + 1], vertices[vertIndex]), vertices[vertIndex + 2]);
                    unityColors[i] = ColorUtils.FromInt(colors[triangles[i]]);
                    triangles[i] = i;
                }
            }
            else
            {
                worldPoints = new Vector3[vertexCount / 3];
                for (int i = 0; i < vertices.Length; i += 3)
                    worldPoints[i / 3] = _tile.Projection
                        .Project(new GeoCoordinate(vertices[i + 1], vertices[i]), vertices[i + 2]);

                unityColors = new Color[colorCount];
                for (int i = 0; i < colorCount; ++i)
                    unityColors[i] = ColorUtils.FromInt(colors[i]);
            }

            Mesh mesh = new Mesh(name, worldPoints, triangles, unityColors);
            _observer.OnNext(new Union<Element, Mesh>(mesh));
        }

        /// <summary> Adapts element data received from utymap. </summary>
        public void AdaptElement(long id, string[] tags, int tagCount, double[] vertices, int vertexCount, 
            string[] styles, int styleCount)
        {
            var geometry = new GeoCoordinate[vertexCount / 2];
            for (int i = 0; i < vertexCount / 2; i += 2)
                geometry[i / 2] = new GeoCoordinate(vertices[i + 1], vertices[i]);

            Element element = new Element(id, geometry, ReadDict(tags), ReadDict(styles));
            _observer.OnNext(new Union<Element, Mesh>(element));
        }

        /// <summary> Adapts error message </summary>
        public void AdaptError(string message)
        {
            var exception = new MapDataException(message);
            _trace.Error(TraceCategory, exception, "cannot load tile: {0}", _tile.ToString());
            _observer.OnError(exception);
        }

        #region Private members

        private static Dictionary<string, string> ReadDict(string[] data)
        {
            var map = new Dictionary<string, string>(data.Length / 2);
            for (int i = 0; i < data.Length; i += 2)
                map.Add(data[i], data[i + 1]);
            return map;
        }

        #endregion
    }
}
