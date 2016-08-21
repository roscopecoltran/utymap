using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Core.Utils;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Maps.Data;
using UtyRx;

namespace Assets.Scripts
{
    sealed class BirdEyeLevelBehaviour : MonoBehaviour
    {
        private const int LevelOfDetails = 14;

        /// <summary> Start latitude. </summary>
        public double StartLatitude = 52.5169051;

        /// <summary> Start longitude. </summary>
        public double StartLongitude = 13.3882853;

        private ApplicationManager _appManager;

        #region Unity lifecycle events

        /// <summary> Performs framework initialization once, before any Start() is called. </summary>
        void Awake()
        {
            _appManager = ApplicationManager.Instance;
            _appManager.InitializeFramework(ConfigBuilder.GetDefault(),
                (compositionRoot) =>
                {
                    compositionRoot.RegisterAction((c, _) => c.Register(UtyDepend.Component
                            .For<IProjection>()
                            .Use<CartesianProjection>(new GeoCoordinate(StartLatitude, StartLongitude))));
                });
        }

        /// <summary> Runs game after all Start() methods are called. </summary>
        void OnEnable()
        {
            // utymap is better to start on non-UI thread
            Observable.Start(() =>
            {
                _appManager.RunGame();

                // NOTE Assume that persistent data is created.
                LoadTiles();
            }, Scheduler.ThreadPool).Subscribe();
        }

        #endregion

        /// <summary> Loads quadkeys near location specified. </summary>
        private void LoadTiles()
        {
            var tileController = _appManager.GetService<ITileController>();
            var modelBuilder = _appManager.GetService<IModelBuilder>();
            var mapDataLoader = _appManager.GetService<IMapDataLoader>();

            var centerQuadKey = GeoUtils.CreateQuadKey(new GeoCoordinate(StartLatitude, StartLongitude), LevelOfDetails);
            int halfCount = 1;
            for (var y = -halfCount; y < halfCount; ++y)
                for (var x = -halfCount; x < halfCount; ++x)
                {
                    var quadKey = new QuadKey(centerQuadKey.TileX + x, centerQuadKey.TileY + y, LevelOfDetails);
                    var tile = new Tile(quadKey, tileController.Stylesheet, tileController.Projection);
                    mapDataLoader
                        .Load(tile)
                        .SubscribeOn(Scheduler.CurrentThread)
                        .ObserveOn(Scheduler.MainThread)
                        .Subscribe(u =>
                        {
                            // NOTE can create game object only on UI thread
                            if (tile.GameObject == null)
                                tile.GameObject = new GameObject("tile");

                            u.Match(e => modelBuilder.BuildElement(tile, e), m => modelBuilder.BuildMesh(tile, m));
                        });
                }
        }
    }
}
