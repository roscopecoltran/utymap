using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure;
using UtyMap.Unity.Infrastructure.Config;
using UtyRx;

namespace Assets.Scripts
{
    /// <summary> 
    ///     This behavior is used in Street Level scene where you have 3D buildings and the highest 
    ///     level of detalization. Add it to Character. 
    ///     In general, it performs some initialization and listens for position changes of character.
    /// </summary>
    sealed class StreetLevelBehaviour : MonoBehaviour
    {
        private const int LevelOfDetails = 16;

        private ApplicationManager _appManager;
        private ITileController _tileController;

        // Current character position.
        private Vector3 _position = new Vector3(float.MinValue, float.MinValue, float.MinValue);

        /// <summary> Start latitude. </summary>
        public double StartLatitude = PositionConfiguration.StartPosition.Latitude;

        /// <summary> Start longitude. </summary>
        public double StartLongitude = PositionConfiguration.StartPosition.Longitude;

        #region Unity lifecycle events

        /// <summary> Performs framework initialization once, before any Start() is called. </summary>
        void Awake()
        {
            _appManager = ApplicationManager.Instance;
            _appManager.InitializeFramework(ConfigBuilder.GetDefault(),
                (compositionRoot) =>
                {
                    compositionRoot.RegisterAction((c, _) => 
                        c.Register(UtyDepend.Component.For<IProjection>().Use<CartesianProjection>(GetWorldZeroPoint())));
                });
            _appManager.CreateDebugConsole();

            _tileController = _appManager.GetService<ITileController>();
        }

        void Start()
        {
            // Need to wrap by conditional compilation symbols due to issues with Unity classes.
#if !CONSOLE
            // set gravity to zero on start to prevent free fall as terrain loading takes some time.
            // restore it afterwards.
            gameObject.GetComponent<Rigidbody>().isKinematic = true;

            // restore gravity and adjust character y-position once first tile is loaded
            _appManager.GetService<IMessageBus>().AsObservable<TileLoadFinishMessage>()
                .Take(1)
                .ObserveOn(Scheduler.MainThread)
                .Subscribe(_ =>
                {
                    // TODO expose elevation logic from native or use old managed implementation?
                    // NOTE in second case, we will consume additional memory

                    //var position = transform.position;
                    //var elevation = AppManager.GetService<IElevationProvider>()
                    //    .GetElevation(new Vector2(position.x, position.z));
                    //transform.position = new Vector3(position.x, elevation + 90, position.z);
                    gameObject.GetComponent<Rigidbody>().isKinematic = false;
                });
#endif
        }

        /// <summary> Runs game after all Start() methods are called. </summary>
        void OnEnable()
        {
            // utymap is better to start on non-UI thread
            Observable.Start(() =>
            {
                // NOTE this is just example: you can load your regions into memory once
                // game is started. Also you can specify different zoom level if you
                // have valid mapcss stylesheet
                //_appManager
                //    .GetService<IMapDataLoader>()
                //    .AddToStore(MapStorageType.InMemory,
                //                @"Osm/berlin.osm.xml",
                //                _appManager.GetService<Stylesheet>(),
                //                new Range<int>(LevelOfDetails, LevelOfDetails));

                _appManager.RunGame();
            }, Scheduler.ThreadPool).Subscribe();
        }

        /// <summary> Listens for position changes to notify library. </summary>
        void Update()
        {
            if (_appManager.IsInitialized && _position != transform.position)
            {
                _position = transform.position;
                Scheduler.ThreadPool.Schedule(() => 
                    _tileController.OnPosition(new Vector2(_position.x, _position.z), LevelOfDetails));
            }
        }

        #endregion

        #region Private methods

        /// <summary> Gets Geocoordinate which correspons to (0, 0) in world coordinates. </summary>
        private GeoCoordinate GetWorldZeroPoint()
        {
            // NOTE allow redefine position in unity editor.
            var editTimePosition = new GeoCoordinate(StartLatitude, StartLongitude);

            return editTimePosition != PositionConfiguration.StartPosition
                ? editTimePosition
                : PositionConfiguration.StartPosition;
        }

        #endregion
    }
}
