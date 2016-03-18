using System;
using System.Collections.Generic;
using Utymap.UnityLib.Infrastructure.Reactive;
using UnityEngine;
using Utymap.UnityLib.Core.Models;
using Utymap.UnityLib.Core.Utils;
using Utymap.UnityLib.Infrastructure;
using Utymap.UnityLib.Infrastructure.Config;
using Utymap.UnityLib.Infrastructure.Dependencies;
using Utymap.UnityLib.Maps.Loader;

namespace Utymap.UnityLib.Core.Tiling
{
    /// <summary> Controls flow of loading/unloading tiles. </summary>
    /// <summary> Tested for cartesian projection only. </summary>
    public interface ITileController
    {
        /// <summary> Current position on map in world coordinates. </summary>
        Vector2 CurrentMapPoint { get; }

        /// <summary> Current position on map in geo coordinates. </summary>
        GeoCoordinate CurrentPosition { get; }

        /// <summary> Gets current tile. </summary>
        Tile CurrentTile { get; }

        /// <summary> Stylesheet. </summary>
        Stylesheet Stylesheet { get; set; }

        /// <summary> Projection. </summary>
        IProjection Projection { get; set; }

        /// <summary> Called when position is changed. </summary>
        void OnPosition(Vector2 position, int levelOfDetails);

        /// <summary> Called when position is changed. </summary>
        void OnPosition(GeoCoordinate coordinate, int levelOfDetails);
    }

    #region Default implementation

    /// <summary> Default implementation of tile controller. </summary>
    /// <remarks> Not thread safe. </remarks>
    internal class TileController : ITileController, IConfigurable
    {
        private readonly IModelBuilder _modelBuilder;
        private readonly object _lockObj = new object();

        private readonly IMapDataLoader _tileLoader;
        private readonly ITileActivator _tileActivator;

        private double _offsetRatio;
        private double _moveSensitivity;

        private Vector2 _lastUpdatePosition = new Vector2(float.MinValue, float.MinValue);

        private QuadKey _currentQuadKey;
        private readonly Dictionary<QuadKey, Tile> _loadedTiles = new Dictionary<QuadKey, Tile>();

        #region Public members

        /// <summary> Creates instance of <see cref="TileController"/>. </summary>
        [Dependency]
        public TileController(IModelBuilder modelBuilder, IMapDataLoader tileLoader, ITileActivator tileActivator)
        {
            _modelBuilder = modelBuilder;
            _tileLoader = tileLoader;
            _tileActivator = tileActivator;
        }

        /// <inheritdoc />
        public Vector2 CurrentMapPoint { get; private set; }

        /// <inheritdoc />
        public GeoCoordinate CurrentPosition { get; private set; }

        /// <inheritdoc />
        public Tile CurrentTile { get { return _loadedTiles[_currentQuadKey]; } }

        /// <inheritdoc />
        [Dependency]
        public Stylesheet Stylesheet { get; set; }

        /// <inheritdoc />
        [Dependency]
        public IProjection Projection { get; set; }

        /// <inheritdoc />
        public void OnPosition(Vector2 position, int levelOfDetails)
        {
            OnPosition(Projection.Project(position), position, levelOfDetails);
        }

        /// <inheritdoc />
        public void OnPosition(GeoCoordinate coordinate, int levelOfDetails)
        {
            var vector3D = Projection.Project(coordinate, 0);
            OnPosition(coordinate, new Vector2(vector3D.x, vector3D.z), levelOfDetails);
        }

        private void OnPosition(GeoCoordinate geoPosition, Vector2 position, int levelOfDetails)
        {
            CurrentMapPoint = position;
            CurrentPosition = geoPosition;

            // call update logic only if threshold is reached
            if (Math.Abs(position.x - _lastUpdatePosition.x) > _moveSensitivity ||
                Math.Abs(position.y - _lastUpdatePosition.y) > _moveSensitivity)
            {
                lock (_lockObj)
                {
                    _lastUpdatePosition = position;

                    _currentQuadKey = GeoUtils.CreateQuadKey(geoPosition, levelOfDetails);

                    if (_loadedTiles.ContainsKey(_currentQuadKey))
                    {
                        var tile = _loadedTiles[_currentQuadKey];
                        if (ShouldPreload(tile, position))
                            PreloadNextTile(tile, position);
                        return;
                    }

                    Load(_currentQuadKey);

                    // TODO add unload old tiles logic
                }
            }
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _moveSensitivity = configSection.GetFloat("sensitivity", 50);
            _offsetRatio = configSection.GetFloat("offset", 5); // percentage of tile size
        }

        #endregion

        #region Loading

        /// <summary> Loads tile for given quadKey. </summary>
        private void Load(QuadKey quadKey)
        {
            Tile tile = new Tile(quadKey, Stylesheet, Projection);
            _loadedTiles.Add(quadKey, tile); // TODO remove tile from hashmap if exception is raised
            _tileLoader
                .Load(tile)
                .SubscribeOn(Scheduler.ThreadPool)
                .ObserveOnMainThread()
                .Subscribe(
                    u => u.Match(_modelBuilder.BuildElement, _modelBuilder.BuildMesh),
                    () => _tileActivator.Activate(tile));
        }

        #endregion

        #region Preloading

        private void PreloadNextTile(Tile tile, Vector2 position)
        {
            var quadKey = GetNextQuadKey(tile, position);
            if (!_loadedTiles.ContainsKey(quadKey))
                Load(quadKey);
        }

        private bool ShouldPreload(Tile tile, Vector2 position)
        {
            return !tile.Contains(position, tile.Rectangle.width * _offsetRatio);
        }

        /// <summary> Gets next quadkey. </summary>
        private QuadKey GetNextQuadKey(Tile tile, Vector2 position)
        {
            var quadKey = tile.QuadKey;

            Vector2 topLeft = new Vector2(tile.Rectangle.xMin, tile.Rectangle.yMax);
            Vector2 topRight = new Vector2(tile.Rectangle.xMax, tile.Rectangle.yMax);

            // top
            if (IsPointInTriangle(position, tile.Rectangle.center, topLeft, topRight))
                return new QuadKey(quadKey.TileX, quadKey.TileY + 1, quadKey.LevelOfDetail);

            Vector2 bottomLeft = new Vector2(tile.Rectangle.xMin, tile.Rectangle.yMin);

            // left
            if (IsPointInTriangle(position, tile.Rectangle.center, topLeft, bottomLeft))
                return new QuadKey(quadKey.TileX - 1, quadKey.TileY, quadKey.LevelOfDetail);

            Vector2 bottomRight = new Vector2(tile.Rectangle.xMax, tile.Rectangle.yMin);

            // right
            if (IsPointInTriangle(position, tile.Rectangle.center, topRight, bottomRight))
                return new QuadKey(quadKey.TileX + 1, quadKey.TileY, quadKey.LevelOfDetail);

            // bottom
            return new QuadKey(quadKey.TileX, quadKey.TileY - 1, quadKey.LevelOfDetail);
        }

        #endregion

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
    }

    #endregion
}
