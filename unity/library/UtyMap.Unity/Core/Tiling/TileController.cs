using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using UtyMap.Unity.Core.Utils;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyRx;

namespace UtyMap.Unity.Core.Tiling
{
    /// <summary> Controls flow of loading/unloading tiles. </summary>
    public interface ITileController : IObservable<Tile>
    {
        /// <summary> Stylesheet. </summary>
        Stylesheet Stylesheet { get; set; }

        /// <summary> Projection. </summary>
        IProjection Projection { get; set; }

        /// <summary> Should be called when character position is updated. </summary>
        void OnPosition(Vector3 position, int levelOfDetails);

        /// <summary> Should be called when character position is updated. </summary>
        void OnPosition(GeoCoordinate coordinate, int levelOfDetails);

        /// <summary> Loads all tiles for given region and level of details. </summary>
        void OnRegion(Rectangle boundaries, int levelOfDetails);

        /// <summary> Loads all tiles for given region and level of details. </summary>
        void OnRegion(BoundingBox boundaries, int levelOfDetails);

        /// <summary> Loads all tiles for given region and level of details. </summary>
        void OnRegion(IEnumerable<QuadKey> quadKeys);
    }

    #region Default implementation

    /// <summary> Default implementation of tile controller. </summary>
    /// <remarks> Not thread safe. </remarks>
    internal class TileController : ITileController, IConfigurable, IDisposable
    {
        private readonly object _lockObj = new object();

        private double _offsetRatio;
        private double _moveSensitivity;
        private int _maxTileDistance;

        private Vector3 _lastUpdatePosition = new Vector3(float.MinValue, float.MinValue, float.MinValue);

        private QuadKey _currentQuadKey;
        private readonly Dictionary<QuadKey, Tile> _loadedTiles = new Dictionary<QuadKey, Tile>();
        private readonly List<IObserver<Tile>> _tileObservers = new List<IObserver<Tile>>();

        #region Public members

        /// <inheritdoc />
        [Dependency]
        public Stylesheet Stylesheet { get; set; }

        /// <inheritdoc />
        [Dependency]
        public IProjection Projection { get; set; }

        /// <inheritdoc />
        public void OnPosition(Vector3 position, int levelOfDetails)
        {
            OnPosition(Projection.Project(position), position, levelOfDetails);
        }

        /// <inheritdoc />
        public void OnPosition(GeoCoordinate coordinate, int levelOfDetails)
        {
            OnPosition(coordinate, Projection.Project(coordinate, 0), levelOfDetails);
        }

        /// <inheritdoc />
        public void OnRegion(Rectangle boundaries, int levelOfDetails)
        {
            OnRegion(GeoUtils.RectToBoundingBox(Projection, boundaries), levelOfDetails);
        }

        /// <inheritdoc />
        public void OnRegion(BoundingBox boundaries, int levelOfDetails)
        {
            OnRegion(GeoUtils.BoundingBoxToQuadKeys(boundaries, levelOfDetails));
        }

        /// <inheritdoc />
        public void OnRegion(IEnumerable<QuadKey> quadKeys)
        {
            lock (_lockObj)
            {
                foreach (QuadKey quadKey in quadKeys)
                {
                    if (!_loadedTiles.ContainsKey(quadKey))
                        Load(quadKey);
                }
            }
        }

        /// <inheritdoc />
        public IDisposable Subscribe(IObserver<Tile> observer)
        {
            _tileObservers.Add(observer);
            return Disposable.Empty;
        }

        private void OnPosition(GeoCoordinate geoPosition, Vector3 position, int levelOfDetails)
        {
            // call update logic only if threshold is reached
            if (Vector3.Distance(position, _lastUpdatePosition) > _moveSensitivity || _currentQuadKey.LevelOfDetail != levelOfDetails)
            {
                lock (_lockObj)
                {
                    _lastUpdatePosition = position;
                    _currentQuadKey = GeoUtils.CreateQuadKey(geoPosition, levelOfDetails);

                    UnloadFarTiles(_currentQuadKey);

                    if (_loadedTiles.ContainsKey(_currentQuadKey))
                    {
                        var tile = _loadedTiles[_currentQuadKey];
                        if (ShouldPreload(tile, position))
                            PreloadNextTile(tile, position);
                        return;
                    }

                    Load(_currentQuadKey);
                }
            }
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _moveSensitivity = configSection.GetFloat(@"tile/sensitivity", 30);
            _offsetRatio = configSection.GetFloat(@"tile/offset", 10); // percentage of tile size
            _maxTileDistance = configSection.GetInt(@"tile/max_tile_distance", 2);
        }

        #endregion

        #region Loading

        /// <summary> Loads tile for given quadKey. </summary>
        private void Load(QuadKey quadKey)
        {
            var tile = new Tile(quadKey, Stylesheet, Projection);
            _loadedTiles.Add(quadKey, tile);

            NotifyOnNextObservers(tile);
        }

        #endregion

        #region Preloading

        private void PreloadNextTile(Tile tile, Vector3 position)
        {
            var quadKey = GetNextQuadKey(tile, position);
            if (!_loadedTiles.ContainsKey(quadKey))
                Load(quadKey);
        }

        private bool ShouldPreload(Tile tile, Vector3 position)
        {
            return !tile.Contains(position, tile.Rectangle.Width * _offsetRatio);
        }

        /// <summary> Gets next quadkey. </summary>
        private QuadKey GetNextQuadKey(Tile tile, Vector3 position)
        {
            var quadKey = tile.QuadKey;
            var position2D = new Vector2(position.x, position.z);

            // NOTE left-right and top-bottom orientation
            Vector2 topLeft = new Vector2(tile.Rectangle.Left, tile.Rectangle.Top);
            Vector2 topRight = new Vector2(tile.Rectangle.Right, tile.Rectangle.Top);

            // top
            if (IsPointInTriangle(position2D, tile.Rectangle.Center, topLeft, topRight))
                return new QuadKey(quadKey.TileX, quadKey.TileY - 1, quadKey.LevelOfDetail);

            Vector2 bottomLeft = new Vector2(tile.Rectangle.Left, tile.Rectangle.Bottom);

            // left
            if (IsPointInTriangle(position2D, tile.Rectangle.Center, topLeft, bottomLeft))
                return new QuadKey(quadKey.TileX - 1, quadKey.TileY, quadKey.LevelOfDetail);

            Vector2 bottomRight = new Vector2(tile.Rectangle.Right, tile.Rectangle.Bottom);

            // right
            if (IsPointInTriangle(position2D, tile.Rectangle.Center, topRight, bottomRight))
                return new QuadKey(quadKey.TileX + 1, quadKey.TileY, quadKey.LevelOfDetail);

            // bottom
            return new QuadKey(quadKey.TileX, quadKey.TileY + 1, quadKey.LevelOfDetail);
        }

        #endregion

        #region Unloading tiles

        /// <summary> Removes far tiles from list of loaded and sends corresponding message. </summary>
        private void UnloadFarTiles(QuadKey currentQuadKey)
        {
            var tiles = _loadedTiles.ToArray();

            foreach (var loadedTile in tiles)
            {
                var quadKey = loadedTile.Key;
                if ((Math.Abs(quadKey.TileX - currentQuadKey.TileX) + 
                     Math.Abs(quadKey.TileY - currentQuadKey.TileY)) <= _maxTileDistance)
                    continue;
                
                loadedTile.Value.Dispose();
                _loadedTiles.Remove(quadKey);
                
                NotifyOnNextObservers(loadedTile.Value);
            }
        }

        #endregion

        private void NotifyOnNextObservers(Tile tile)
        {
            foreach (var tileObserver in _tileObservers)
                tileObserver.OnNext(tile);
        }

        /// <summary>
        ///     Checks whether point is located in triangle.
        ///     http://stackoverflow.com/questions/13300904/determine-whether-point-lies-inside-triangle
        /// </summary>
        private static bool IsPointInTriangle(Vector2 p, Vector2 p1, Vector2 p2, Vector2 p3)
        {
            var alpha = ((p2.y - p3.y) * (p.x - p3.x) + (p3.x - p2.x) * (p.y - p3.y)) /
                          ((p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y));
            var beta = ((p3.y - p1.y) * (p.x - p3.x) + (p1.x - p3.x) * (p.y - p3.y)) /
                         ((p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y));
            var gamma = 1.0f - alpha - beta;

            return alpha > 0 && beta > 0 && gamma > 0;
        }

        /// <inheritdoc />
        public void Dispose()
        {
            foreach (var loadedTile in _loadedTiles)
                loadedTile.Value.Dispose();

            foreach (var tileObserver in _tileObservers)
                tileObserver.OnCompleted();
        }
    }

    #endregion
}
