using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyRx;

namespace Assets.Scripts
{
    sealed class GlobeBehaviour : MonoBehaviour
    {
        public float GlobeRadius = 100;

        [RangeAttribute(1, 1)]
        public int LevelOfDetails = 1;

        private ApplicationManager _appManager;
        private IMapDataLoader _mapDataLoader;
        private IModelBuilder _modelBuilder;

        #region Unity lifecycle events

        /// <summary> Performs framework initialization once, before any Start() is called. </summary>
        private void Awake()
        {
            _appManager = ApplicationManager.Instance;
            _appManager.InitializeFramework(ConfigBuilder.GetDefault(),
                (compositionRoot) =>
                {
                    compositionRoot.RegisterAction((c, _) => 
                        c.Register(UtyDepend.Component.For<IProjection>().Use<SphericalProjection>(GlobeRadius)));
                });

            _mapDataLoader = _appManager.GetService<IMapDataLoader>();
            _modelBuilder = _appManager.GetService<IModelBuilder>();
        }

        /// <summary> Runs game after all Start() methods are called. </summary>
        void OnEnable()
        {
            // utymap is better to start on non-UI thread
            Observable.Start(() =>
            {
                // convert and load map data into memory
                var stylesheet = _appManager.GetService<Stylesheet>();
                var range = new Range<int>(LevelOfDetails, LevelOfDetails);

                //_mapDataLoader.AddToStore(MapStorageType.InMemory, @"/NaturalEarth/ne_110m_admin_0_scale_rank", stylesheet, range);
                
                _mapDataLoader.AddToStore(MapStorageType.InMemory, @"/NaturalEarth/ne_110m_lakes", stylesheet, range);
                _mapDataLoader.AddToStore(MapStorageType.InMemory, @"/NaturalEarth/ne_110m_land", stylesheet, range);
                _mapDataLoader.AddToStore(MapStorageType.InMemory, @"/NaturalEarth/ne_110m_rivers_lake_centerlines", stylesheet, range);
                _mapDataLoader.AddToStore(MapStorageType.InMemory, @"/NaturalEarth/ne_110m_admin_0_boundary_lines_land", stylesheet, range);

                _appManager.RunGame();

                LoadGlobe();

            }, Scheduler.ThreadPool).Subscribe();
        }

        // Loads all globe level tiles manually.
        void LoadGlobe()
        {
            var tileController = _appManager.GetService<ITileController>();
            int count = 2;
            for (var y = 0; y < count; ++y)
                for (var x = 0; x < count; ++x)
                {
                    var quadKey = new QuadKey(x, y, LevelOfDetails);
                    var tile = new Tile(quadKey, tileController.Stylesheet, tileController.Projection);
                    _mapDataLoader
                        .Load(tile)
                        .SubscribeOn(Scheduler.CurrentThread)
                        .ObserveOn(Scheduler.MainThread)
                        .Subscribe(u =>
                        {
                            // NOTE can create game object only on UI thread
                            if (tile.GameObject == null)
                                tile.GameObject = new GameObject("tile");

                            u.Match(e => _modelBuilder.BuildElement(tile, e), m => _modelBuilder.BuildMesh(tile, m));
                        });
                }
        }

        #endregion
    }
}
