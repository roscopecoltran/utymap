using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Core.Utils;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyRx;

namespace Assets.Scripts
{
    sealed class BirdEyeLevelBehaviour : MonoBehaviour
    {
        private const int LevelOfDetails = 14;

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
                            .Use<CartesianProjection>(PositionConfiguration.StartPosition)));
                });
            _appManager.CreateDebugConsole();
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
    }
}
