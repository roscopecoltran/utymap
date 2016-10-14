using System;
using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Core.Utils;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Maps.Data;
using UtyRx;

namespace Assets.Scripts
{
    sealed class BirdEyeLevelBehaviour : MonoBehaviour
    {
        private const int LevelOfDetails = 14;
        private ApplicationManager _appManager;

        public string MapzenApiKey = "";

        #region Unity lifecycle events

        /// <summary> Performs framework initialization once, before any Start() is called. </summary>
        void Awake()
        {
            _appManager = ApplicationManager.Instance;
            _appManager.InitializeFramework(ConfigBuilder
                .GetDefault()
                // NOTE Please use your api key here: I'm invalidating it time to rime
                .SetMapzenMapData("http://tile.mapzen.com/mapzen/vector/v1/{0}/{1}/{2}/{3}.json?api_key={4}",
                                  "all", "json", MapzenApiKey),
                (compositionRoot) =>
                {
                    compositionRoot.RegisterAction((c, _) => c.Register(UtyDepend.Component
                            .For<IProjection>()
                            .Use<CartesianProjection>(PositionConfiguration.StartPosition)));
                });
            _appManager.CreateDebugConsole();
            
            CheckApiKey();
        }

        /// <summary> Runs game after all Start() methods are called. </summary>
        void OnEnable()
        {
            // utymap is better to start on non-UI thread
            Observable.Start(LoadTiles, Scheduler.ThreadPool).Subscribe();
        }

        #endregion

        /// <summary> Loads quadkeys near location specified. </summary>
        private void LoadTiles()
        {
            var tileController = _appManager.GetService<ITileController>();
            var modelBuilder = _appManager.GetService<IModelBuilder>();
            var mapDataLoader = _appManager.GetService<IMapDataLoader>();

            var centerQuadKey = GeoUtils.CreateQuadKey(PositionConfiguration.StartPosition, LevelOfDetails);
            var tile = new Tile(centerQuadKey, tileController.Stylesheet, tileController.Projection);
            mapDataLoader
                .Load(tile)
                .SubscribeOn(Scheduler.CurrentThread)
                .ObserveOn(Scheduler.MainThread)
                .Subscribe(u => u.Match(e => modelBuilder.BuildElement(tile, e), m => modelBuilder.BuildMesh(tile, m)));
        }

        private void CheckApiKey()
        {
            if (String.IsNullOrEmpty(MapzenApiKey))
                _appManager.GetService<ITrace>().Warn("validation", "Please use your mapzen developer key!");
        }
    }
}
