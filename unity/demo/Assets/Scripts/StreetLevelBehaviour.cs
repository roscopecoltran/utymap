using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
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
        private const string LogCategory = "scene.street";
        private const int LevelOfDetails = 16;

        private ApplicationManager _appManager;
        private ITileController _tileController;
        private ITrace _trace;

        // Current character position.
        private Vector3 _position = new Vector3(float.MinValue, float.MinValue, float.MinValue);

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
            _trace = _appManager.GetService<ITrace>();
        }

        void Start()
        {
            // Freeze character on start to prevent free fall as terrain loading takes some time.
            // Restore it afterwards.
            gameObject.GetComponent<Rigidbody>().isKinematic = true;
            _appManager.GetService<IMessageBus>().AsObservable<TileLoadFinishMessage>()
                .Take(1)
                .ObserveOn(Scheduler.MainThread)
                .Subscribe(_ =>
                {
                    gameObject.GetComponent<Rigidbody>().isKinematic = false;
                });

            // We want to destroy tile's gameobjects at some point.
            _appManager.GetService<IMessageBus>().AsObservable<TileDestroyMessage>()
                .ObserveOn(Scheduler.MainThread)
                .Subscribe(m =>
                {
                    _trace.Info(LogCategory, "Remove tile: {0}", m.Tile.ToString());
                    Destroy(m.Tile.GameObject);
                });
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
            return PositionConfiguration.StartPosition;
        }

        #endregion
    }
}
