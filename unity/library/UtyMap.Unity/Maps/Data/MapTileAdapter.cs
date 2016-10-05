using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Core.Utils;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.Primitives;
using UnityEngine;
using UtyRx;
using Mesh = UtyMap.Unity.Core.Models.Mesh;

namespace UtyMap.Unity.Maps.Data
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

        private static Regex ElementNameRegex = new Regex("^(building|barrier):([0-9]*)");

        public MapTileAdapter(Tile tile, IObserver<Union<Element, Mesh>> observer, ITrace trace)
        {
            _tile = tile;
            _observer = observer;
            _trace = trace;
        }

        /// <summary> Adapts mesh data received from utymap. </summary>
        public void AdaptMesh(string name, double[] vertices, int vertexCount,
            int[] triangles, int triangleCount, int[] colors, int colorCount,
            double[] uvs, int uvCount, int[] uvMap, int uvMapCount)
        {
            Vector3[] worldPoints;
            Color[] unityColors;

            Vector2[] unityUvs;
            Vector2[] unityUvs2;
            Vector2[] unityUvs3;

            // NOTE process terrain differently to emulate flat shading effect by avoiding 
            // triangles to share the same vertex. Remove "if" branch if you don't need it
            if (name.Contains("terrain"))
            {
                worldPoints = new Vector3[triangleCount];
                unityColors = new Color[triangleCount];

                unityUvs = new Vector2[triangleCount];
                unityUvs2 = new Vector2[triangleCount];
                unityUvs3 = new Vector2[triangleCount];

                var textureMapper = CreateTextureAtlasMapper(unityUvs, unityUvs2, unityUvs3, uvs, uvMap);

                for (int i = 0; i < triangles.Length; ++i)
                {
                    int vertIndex = triangles[i] * 3;
                    worldPoints[i] = _tile.Projection
                        .Project(new GeoCoordinate(vertices[vertIndex + 1], vertices[vertIndex]), vertices[vertIndex + 2]);

                    unityColors[i] = ColorUtils.FromInt(colors[triangles[i]]);
                    textureMapper.SetUvs(i, triangles[i] * 2);
                    triangles[i] = i;
                }
            }
            else
            {
                long id;
                if (!ShouldLoad(name, out id))
                    return;

                worldPoints = new Vector3[vertexCount / 3];
                for (int i = 0; i < vertices.Length; i += 3)
                    worldPoints[i / 3] = _tile.Projection
                        .Project(new GeoCoordinate(vertices[i + 1], vertices[i]), vertices[i + 2]);

                unityColors = new Color[colorCount];
                for (int i = 0; i < colorCount; ++i)
                    unityColors[i] = ColorUtils.FromInt(colors[i]);

                if (uvCount > 0)
                {
                    unityUvs = new Vector2[uvCount/2];
                    unityUvs2 = new Vector2[uvCount/2];
                    unityUvs3 = new Vector2[uvCount/2];

                    var textureMapper = CreateTextureAtlasMapper(unityUvs, unityUvs2, unityUvs3, uvs, uvMap);
                    for (int i = 0; i < uvCount; i += 2)
                    {
                        unityUvs[i/2] = new Vector2((float) uvs[i], (float) uvs[i + 1]);
                        textureMapper.SetUvs(i/2, i);
                    }
                }
                else
                {
                    unityUvs = new Vector2[worldPoints.Length];
                    unityUvs2 = new Vector2[worldPoints.Length];
                    unityUvs3 = new Vector2[worldPoints.Length];
                }

                _tile.Register(id);
            }

            if (worldPoints.Length >= 65000)
                _trace.Warn(TraceCategory, "Mesh '{0}' has more vertices than allowed: {1}. " +
                                           "It should be split but this is missing functionality in UtyMap.Unity.", 
                                           name, worldPoints.Length.ToString());
            Mesh mesh = new Mesh(name, 0, worldPoints, triangles, unityColors, unityUvs, unityUvs2, unityUvs3);
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

        private bool ShouldLoad(string name, out long id)
        {
            var match = ElementNameRegex.Match(name);
            if (!match.Success)
            {
                id = 0;
                return true;
            }

            id = long.Parse(match.Groups[2].Value);
            return !_tile.Has(id);
        }

        private static TextureAtlasMapper CreateTextureAtlasMapper(Vector2[] unityUvs, Vector2[] unityUvs2, Vector2[] unityUvs3,
                double[] uvs, int[] uvMap)
        {
            const int infoEntrySize = 8;
            var count = uvMap == null ? 0 : uvMap.Length;
            List<TextureAtlasInfo> infos = new List<TextureAtlasInfo>(count / infoEntrySize);
            int lastIndex = 0;
            for (int i = 0; i < count; )
            {
                var info = new TextureAtlasInfo();
                info.UvIndexRange = new Range<int>(lastIndex == 0 ? 0 : infos[lastIndex].UvIndexRange.Maximum, uvMap[i++]);
                info.TextureIndex = uvMap[i++];

                int atlasWidth = uvMap[i++];
                int atlasHeight = uvMap[i++];
                float x = uvMap[i++];
                float y = uvMap[i++];
                float width = uvMap[i++];
                float height = uvMap[i++];

                bool isEmpty = atlasWidth == 0 || atlasHeight == 0;
                info.TextureSize = new Vector2(isEmpty ? 0 : width / atlasWidth, isEmpty ? 0 : height / atlasHeight);
                info.TextureOffset = new Vector2(isEmpty ? 0 : x / atlasWidth, isEmpty ? 0 : y / atlasHeight);

                infos.Add(info);
                lastIndex++;
            }

            return new TextureAtlasMapper(unityUvs, unityUvs2, unityUvs3, uvs, infos);
        }

        #endregion

        #region Nested class

        private class TextureAtlasMapper
        {
            private readonly Vector2[] _unityUvs;
            private readonly Vector2[] _unityUvs2;
            private readonly Vector2[] _unityUvs3;
            private readonly double[] _uvs;
            private readonly List<TextureAtlasInfo> _infos;

            public TextureAtlasMapper(Vector2[] unityUvs, Vector2[] unityUvs2, Vector2[] unityUvs3, double[] uvs,
                List<TextureAtlasInfo> infos)
            {
                _unityUvs = unityUvs;
                _unityUvs2 = unityUvs2;
                _unityUvs3 = unityUvs3;
                _uvs = uvs;
                _infos = infos;
            }

            public void SetUvs(int resultIndex, int origIindex)
            {
                int begin = 0;
                int end = _infos.Count;

                while (begin < end)
                {
                    int middle = begin + (end - begin) / 2;
                    var info = _infos[middle];
                    if (info.UvIndexRange.Contains(origIindex))
                    {
                        _unityUvs[resultIndex] = new Vector2((float)_uvs[origIindex], (float)_uvs[origIindex + 1]);
                        _unityUvs2[resultIndex] = info.TextureSize;
                        _unityUvs3[resultIndex] = info.TextureOffset;
                        return;
                    }
                    if (info.UvIndexRange.Minimum > origIindex)
                        end = middle;
                    else
                        begin = middle + 1;
                }
            }
        }

        private struct TextureAtlasInfo
        {
            public int TextureIndex;
            public Range<int> UvIndexRange;
            public Vector2 TextureSize;
            public Vector2 TextureOffset;
        }

        #endregion
    }
}
