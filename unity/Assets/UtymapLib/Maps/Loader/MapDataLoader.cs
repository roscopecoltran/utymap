using System;
using System.Collections.Generic;
using System.IO;
using Assets.UtymapLib.Infrastructure.Reactive;
using UnityEngine;
using Assets.UtymapLib.Core;
using Assets.UtymapLib.Core.Models;
using Assets.UtymapLib.Core.Tiling;
using Assets.UtymapLib.Core.Utils;
using Assets.UtymapLib.Infrastructure.Config;
using Assets.UtymapLib.Infrastructure.Dependencies;
using Assets.UtymapLib.Infrastructure.Diagnostic;
using Assets.UtymapLib.Infrastructure.IO;
using Assets.UtymapLib.Infrastructure.Primitives;
using Assets.UtymapLib.Maps.Elevation;
using Mesh = Assets.UtymapLib.Core.Models.Mesh;

namespace Assets.UtymapLib.Maps.Loader
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
        private const string CacheFileNameExtension = ".osm.xml";
        private const string TraceCategory = "mapdata.loader";
        private readonly object _lockObj = new object();

        private readonly IElevationProvider _elevationProvider;
        private readonly IPathResolver _pathResolver;
        private string _mapDataServerUri;
        private string _mapDataServerQuery;
        private string _mapDataFormat;
        private string _cachePath;
        private IFileSystemService _fileSystemService;

        [Dependency]
        public ITrace Trace { get; set; }

        [Dependency]
        public MapDataLoader(IElevationProvider elevationProvider, IFileSystemService fileSystemService, IPathResolver pathResolver)
        {
            _elevationProvider = elevationProvider;
            _fileSystemService = fileSystemService;
            _pathResolver = pathResolver;
        }

        /// <inheritdoc />
        public void AddToInMemoryStore(string dataPath, Stylesheet stylesheet, Range<int> levelOfDetails)
        {
            var dataPathResolved = _pathResolver.Resolve(dataPath);
            var stylesheetPathResolved = _pathResolver.Resolve(stylesheet.Path);

            Trace.Info(TraceCategory, "Add to in-memory storage: data:{0} style: {1}", 
                dataPathResolved, stylesheetPathResolved);

            string errorMsg = null;
            UtymapLib.AddToInMemoryStore(stylesheetPathResolved, dataPathResolved, 
                levelOfDetails.Minimum, levelOfDetails.Maximum, error => errorMsg = error);

            if (errorMsg != null)
                throw new MapDataException(errorMsg);
        }

        /// <inheritdoc />
        public bool HasTile(Tile tile)
        {
            var quadKey = tile.QuadKey;
            return UtymapLib.HasData(quadKey.TileX, quadKey.TileY, quadKey.LevelOfDetail);
        }

        /// <inheritdoc />
        public IObservable<Union<Element, Mesh>> Load(Tile tile)
        {
            return Observable.Create<Tile>(o =>
                {
                    o.OnNext(tile);
                    o.OnCompleted();
                    return Disposable.Empty;
                })
                .SelectMany(t => CreateElevationSequence(t))
                .SelectMany(t => CreateDownloadSequence(t))
                .SelectMany(t => CreateLoadSequence(t));
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _mapDataServerUri = configSection.GetString(@"data/remote/server", null);
            _mapDataServerQuery = configSection.GetString(@"data/remote/query", null);
            _mapDataFormat = configSection.GetString(@"data/remote/format", "xml");
            _cachePath = configSection.GetString(@"data/cache", null);

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

        /// <summary> Downloads elevation data for given tile. </summary>
        private IObservable<Tile> CreateElevationSequence(Tile tile)
        {
            return _elevationProvider.HasElevation(tile.BoundingBox)
                ? Observable.Return(tile)
                : _elevationProvider.Download(tile.BoundingBox).Select(_ => tile);
        }

        /// <summary> Downloads map data for given tile. </summary>
        private IObservable<Tile> CreateDownloadSequence(Tile tile)
        {
            // Data exists in store
            if (UtymapLib.HasData(tile.QuadKey.TileX, tile.QuadKey.TileY, tile.QuadKey.LevelOfDetail))
                return Observable.Return(tile);

            // Data exists in cache
            var filePath = GetCacheFilePath(tile);
            if (_fileSystemService.Exists(filePath))
                return AddToInMemoryStore(tile, filePath);

            // Data should be downloaded first
            BoundingBox query = tile.BoundingBox;
            var queryString = String.Format(_mapDataServerQuery,
               query.MinPoint.Latitude, query.MinPoint.Longitude,
               query.MaxPoint.Latitude, query.MaxPoint.Longitude);
            var uri = String.Format("{0}{1}", _mapDataServerUri, Uri.EscapeDataString(queryString));
            Trace.Warn(TraceCategory, Strings.NoPresistentElementSourceFound, query.ToString(), uri);
            return ObservableWWW.GetAndGetBytes(uri)
                   .Take(1)
                   .SelectMany(bytes =>
                   {
                       lock (_lockObj)
                       {
                           // save downloaded bytes as file
                           if (!_fileSystemService.Exists(filePath))
                               using (var stream = _fileSystemService.WriteStream(filePath))
                                   stream.Write(bytes, 0, bytes.Length);
                       }

                       return AddToInMemoryStore(tile, filePath);
                   });
        }

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

                        Vector3[] worldPoints = new Vector3[count / 3];
                        for (int i = 0; i < vertices.Length; i += 3)
                            worldPoints[i / 3] = tile.Projection
                                .Project(new GeoCoordinate(vertices[i + 1], vertices[i]), vertices[i + 2]);

                        Color[] unityColors = new Color[colorCount];
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

        /// <summary> Adds tile data into in memory storage. </summary>
        private IObservable<Tile> AddToInMemoryStore(Tile tile, string filePath)
        {
            // add to in memory 
            Trace.Info(TraceCategory, "Adding into memory: {0}", tile.QuadKey.ToString());
            string errorMsg = null;
            UtymapLib.AddToInMemoryStore(
                _pathResolver.Resolve(tile.Stylesheet.Path), 
                _pathResolver.Resolve(filePath),
                tile.QuadKey.LevelOfDetail,
                tile.QuadKey.LevelOfDetail, error => errorMsg = error);

            return errorMsg != null 
                ? Observable.Throw<Tile>(new MapDataException(errorMsg)) 
                : Observable.Return(tile);
        }

        /// <summary> Returns cache file name for given tile. </summary>
        private string GetCacheFilePath(Tile tile)
        {
            var cacheFileName = tile.QuadKey + CacheFileNameExtension;
            return Path.Combine(_cachePath, cacheFileName);
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
