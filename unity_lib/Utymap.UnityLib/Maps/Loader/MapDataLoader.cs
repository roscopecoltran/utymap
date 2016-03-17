using System;
using System.Collections.Generic;
using Utymap.UnityLib.Infrastructure.Reactive;
using UnityEngine;
using Utymap.UnityLib.Core;
using Utymap.UnityLib.Core.Models;
using Utymap.UnityLib.Core.Tiling;
using Utymap.UnityLib.Core.Utils;
using Utymap.UnityLib.Infrastructure.Config;
using Utymap.UnityLib.Infrastructure.Dependencies;
using Utymap.UnityLib.Infrastructure.Diagnostic;
using Utymap.UnityLib.Infrastructure.IO;
using Utymap.UnityLib.Infrastructure.Primitives;
using Utymap.UnityLib.Maps.Elevation;
using Mesh = Utymap.UnityLib.Core.Models.Mesh;

namespace Utymap.UnityLib.Maps.Loader
{
    /// <summary> Defines behavior of class responsible to mapdata processing. </summary>
    public interface IMapDataLoader
    {
        /// <summary> Adds mapdata to in-memory storage. </summary>
        /// <param name="dataPath"> Path to mapdata. </param>
        /// <param name="stylesheet"> Stylesheet which to use during import. </param>
        /// <param name="levelOfDetails"> Which level of details to use. </param>
        void AddToInMemoryStore(string dataPath, Stylesheet stylesheet, Range<int> levelOfDetails);

        /// <summary> Loads given tile. This method triggers real loading and processing osm data. </summary>
        /// <param name="tile">Tile to load.</param>
        IObservable<Union<Element, Mesh>> Load(Tile tile);
    }

    /// <summary> Default implementation of tile loader. </summary>
    internal class MapDataLoader : IMapDataLoader, IConfigurable, IDisposable
    {
        private readonly IElevationProvider _elevationProvider;
        private readonly IPathResolver _pathResolver;
        private const string TraceCategory = "mapdata.loader";

        [Dependency]
        public ITrace Trace { get; set; }

        [Dependency]
        public MapDataLoader(IElevationProvider elevationProvider, IPathResolver pathResolver)
        {
            _elevationProvider = elevationProvider;
            _pathResolver = pathResolver;
        }

        /// <inheritdoc />
        public void AddToInMemoryStore(string dataPath, Stylesheet stylesheet, Range<int> levelOfDetails)
        {
            var dataPathResolved = _pathResolver.Resolve(dataPath);
            var stylesheetPathResolved = _pathResolver.Resolve(stylesheet.Path);

            Trace.Info(TraceCategory, "Add to in-memory storage: {0}, stylerrore: {1}", 
                dataPathResolved, stylesheetPathResolved);

            string errorMsg = null;
            UtymapLib.AddToInMemoryStore(stylesheetPathResolved, dataPathResolved, 
                levelOfDetails.Minimum, levelOfDetails.Maximum, error => errorMsg = error);

            if (errorMsg != null)
                throw new MapDataException(errorMsg);
        }

        /// <inheritdoc />
        public IObservable<Union<Element, Mesh>> Load(Tile tile)
        {
            return _elevationProvider.HasElevation(tile.BoundingBox)
                ? CreateLoadSequence(tile)
                : _elevationProvider.Download(tile.BoundingBox)
                                    .ContinueWith(() => CreateLoadSequence(tile), Scheduler.ThreadPool);
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            var stringPath = _pathResolver.Resolve(configSection.GetString("data/index/strings", @"/"));
            var dataPath = _pathResolver.Resolve(configSection.GetString("data/index/spatial", @"/"));

            string errorMsg = null;
            UtymapLib.Configure(stringPath, dataPath, error => errorMsg = error);
            if (errorMsg != null)
                throw new MapDataException(errorMsg);
        }

        /// <inheritdoc />
        public void Dispose()
        {
            UtymapLib.Dispose();
        }

        #region Private methods

        /// <summary> Creates <see cref="IObservable{T}"/> for loading element of given tile. </summary>
        private IObservable<Union<Element, Mesh>> CreateLoadSequence(Tile tile)
        {
            return Observable.Create<Union<Element, Mesh>>(observer =>
            {
                Trace.Info(TraceCategory, "Loading tile: {0}", tile.QuadKey.ToString());

                bool noException = true;
                UtymapLib.LoadTile(_pathResolver.Resolve(tile.Stylesheet.Path), tile.QuadKey.TileX,
                    tile.QuadKey.TileY, tile.QuadKey.LevelOfDetail,
                    // mesh callback
                    (name, vertices, count, triangles, triangleCount, colors, colorCount) =>
                    {
                        Trace.Debug(TraceCategory, "receive mesh: {0}", name);

                        Vector3[] worldPoints = new Vector3[count];
                        for (int i = 0; i < vertices.Length; i += 3)
                            worldPoints[i / 3] = tile.Projection
                                .Project(new GeoCoordinate(vertices[i + 1], vertices[i]), vertices[i + 2]);

                        Color32[] unityColors = new Color32[colorCount];
                        for (int i = 0; i < colorCount; ++i)
                            unityColors[i] = ColorUtils.FromInt(colors[i]);

                        Mesh mesh = new Mesh(name, worldPoints, triangles, unityColors);
                        observer.OnNext(new Union<Element, Mesh>(mesh));
                    },
                    // element callback
                    (id, tags, count, vertices, vertexCount, styles, styleCount) =>
                    {
                        Trace.Debug(TraceCategory, "receive element: {0}", id.ToString());

                        var geometry = new GeoCoordinate[vertexCount / 2];
                        for (int i = 0; i < vertexCount / 2; i += 2)
                            geometry[i / 2] = new GeoCoordinate(vertices[i + 1], vertices[i]);

                        Element element = new Element(id, geometry, ReadDict(tags), ReadDict(styles));
                        observer.OnNext(new Union<Element, Mesh>(element));
                    },
                    // error callback
                    message =>
                    {
                        noException = false;
                        var exception = new MapDataException(message);
                        Trace.Error(TraceCategory, exception, "Cannot load tile: {0}", tile.QuadKey.ToString());
                        observer.OnError(exception);
                    });
                if (noException)
                    observer.OnCompleted();

                return Disposable.Empty;
            });
        }

        private static Dictionary<string, string> ReadDict(string[] data)
        {
            var map = new Dictionary<string, string>(data.Length/2);
            for (int i = 0; i < data.Length; i += 2)
                map.Add(data[i], data[i + 1]);
            return map;
        }

        #endregion
    }
}
