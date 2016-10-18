using Assets.Scripts.Extensions;
using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyRx;
using Mesh = UtyMap.Unity.Core.Models.Mesh;

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
        private const float PositionUpdateThreshold = 10;

        private int _levelOfDetails = 16;

        private ApplicationManager _appManager;
        private ITileController _tileController;
        private ITrace _trace;

        // Current character position.
        private Vector3 _position = new Vector3(float.MinValue, float.MinValue, float.MinValue);
        private IModelBuilder _modelBuilder;
        private IMapDataLoader _mapDataLoader;
        private IMessageBus _messageBus;

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

            _trace = _appManager.GetService<ITrace>();

            _tileController = _appManager.GetService<ITileController>();
            _modelBuilder = _appManager.GetService<IModelBuilder>();
            _mapDataLoader = _appManager.GetService<IMapDataLoader>();
            _messageBus = _appManager.GetService<IMessageBus>();
        }

        void Start()
        {
            var rigidBody = gameObject.GetComponent<Rigidbody>();
            rigidBody.isKinematic = true;

            _tileController
                .Where(tile => tile.IsDisposed)
                .ObserveOn(Scheduler.MainThread)
                 .Subscribe(tile =>
                 {
                     _trace.Info(LogCategory, "Remove tile: {0}", tile.ToString());
                     Destroy(tile.GameObject);
                 });

            _tileController
                .Where(tile => !tile.IsDisposed)
                .SelectMany(tile => 
                    _mapDataLoader.Load(tile)
                                  .Select(u => new Tuple<Tile, Union<Element, Mesh>>(tile, u))
                                  .ObserveOn(Scheduler.MainThread)
                                  .DoOnCompleted(() =>
                                  {
                                      if (rigidBody.isKinematic)
                                          rigidBody.isKinematic = false;
                                  }))
                .SubscribeOn(Scheduler.ThreadPool)
                .ObserveOn(Scheduler.MainThread)
                .Subscribe(t => t.Item2.Match(e => _modelBuilder.BuildElement(t.Item1, e), m => _modelBuilder.BuildMesh(t.Item1, m)));

            _messageBus
                .AsObservable<OnZoomRequested>()
                .Subscribe(msg =>
                {
                    var newLevel = msg.IsZoomOut ? 14 : 16;
                    if (newLevel == _levelOfDetails)
                        return;
                    
                    _levelOfDetails = newLevel;
                    _position = new Vector3(float.MinValue, float.MinValue, float.MinValue);

                    _messageBus.Send(new OnZoomChanged(newLevel));
                });

            // NOTE: code below loads region tile by tile.
            // The region is specified by rectangle defined in world coordinates.
            //float size = 600; // square side size (in meters)
            //Scheduler.ThreadPool.Schedule(() => _tileController.OnRegion(new Rectangle(-size / 2, -size / 2, size, size), _levelOfDetails));
        }

        /// <summary> Listens for position changes to notify library. </summary>
        void Update()
        {
            if (_appManager.IsInitialized && Vector3.Distance(_position, transform.position) > PositionUpdateThreshold)
            {
                _position = transform.position;
                Scheduler.ThreadPool.Schedule(() => _tileController.OnPosition(_position, _levelOfDetails));
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
