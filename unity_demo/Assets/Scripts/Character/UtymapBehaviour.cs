using System;
using UnityEngine;
using Utymap.UnityLib;
using Utymap.UnityLib.Core;
using Utymap.UnityLib.Core.Tiling;
using Utymap.UnityLib.Infrastructure;
using Utymap.UnityLib.Infrastructure.Config;
using Utymap.UnityLib.Infrastructure.Diagnostic;
using Utymap.UnityLib.Infrastructure.Reactive;
using Utymap.UnityLib.Maps.Elevation;
using Utymap.UnityLib.Maps.Geocoding;

using Component = Utymap.UnityLib.Infrastructure.Dependencies.Component;

namespace Assets.Scripts.Character
{
    /// <summary> Performs some initialization and listens for position changes of character.  </summary>
    class UtymapBehaviour : MonoBehaviour
    {
        private float _initialGravity;
        protected ApplicationManager AppManager;

        // Current character position.
        private Vector3 _position = new Vector3(float.MinValue, float.MinValue, float.MinValue);

        /// <summary>
        ///     Place name. Will be resolved to the certain GeoCoordinate via performing reverse
        ///     geocoding request to reverse geocoding server.
        /// </summary>
        public string PlaceName;

        /// <summary> Start latitude. Used if PlaceName is empty. </summary>
        public double StartLatitude = 52.5317429;

        /// <summary> Start longitude. Used if PlaceName is empty. </summary>
        public double StartLongitude = 13.3871987;

        #region Protected methods

        /// <summary> Returns config builder initialized with user defined settings. </summary>
        protected virtual ConfigBuilder GetConfigBuilder()
        {
            return ConfigBuilder.GetDefault();
        }

        /// <summary> Returns init action. </summary>
        protected virtual Action<CompositionRoot> GetInitAction()
        {
            return compositionRoot =>
            {
                compositionRoot.RegisterAction((c, _) => 
                    c.Register(Component.For<IProjection>().Use<CartesianProjection>(GetWorldZeroPoint())));
            };
        }

        #endregion

        #region Unity lifecycle events

        /// <summary> Performs framework initialization once, before any Start() is called. </summary>
        void Awake()
        {
            AppManager = ApplicationManager.Instance;
            AppManager.InitializeFramework(GetConfigBuilder(), GetInitAction());
            AppManager.SetZoomLevel(15);
        }

        void Start()
        {
            // set gravity to zero on start to prevent free fall as terrain loading takes some time.
            // restore it afterwards.
            var thirdPersonController = gameObject.GetComponent<ThirdPersonController>();
            _initialGravity = thirdPersonController.gravity;
            thirdPersonController.gravity = 0;

            // restore gravity and adjust character y-position once first tile is loaded
            AppManager.GetService<IMessageBus>().AsObservable<TileLoadFinishMessage>()
                .Take(1)
                .ObserveOnMainThread()
                .Subscribe(_ =>
                {
                    // TODO expose elevation logic from native or use old managed implementation?
                    // NOTE in second case, we will consume additional memory

                    //var position = transform.position;
                    //var elevation = AppManager.GetService<IElevationProvider>()
                    //    .GetElevation(new Vector2(position.x, position.z));
                    //transform.position = new Vector3(position.x, elevation + 90, position.z);
                    thirdPersonController.gravity = _initialGravity;
                });
        }

        /// <summary> Runs game after all Start() methods are called. </summary>
        void OnEnable()
        {
            // utymap is better to start on non-UI thread
            Observable.Start(() => AppManager.RunGame(), Scheduler.ThreadPool);
        }

        /// <summary> Listens for position changes to notify library. </summary>
        void Update()
        {
            if (AppManager.IsInitialized && _position != transform.position)
            {
                _position = transform.position;
                AppManager.SetPosition(new Vector2(_position.x, _position.z));
            }
        }

        #endregion

        #region Private methods

        /// <summary>
        ///     Gets start geocoordinate using desired method: direct latitude/longitude or
        ///     via reverse geocoding request for given place name.
        /// </summary>
        private GeoCoordinate GetWorldZeroPoint()
        {
            var coordinate = new GeoCoordinate(StartLatitude, StartLongitude);
            if (!String.IsNullOrEmpty(PlaceName))
            {
                // NOTE this will freeze UI thread as we're making web request and should wait for its result
                // TODO improve it
                var place = AppManager.GetService<IGeocoder>().Search(PlaceName)
                    .Wait();

                if (place != null)
                {
                    StartLatitude = place.Coordinate.Latitude;
                    StartLongitude = place.Coordinate.Longitude;
                    // NOTE this prevents name resolution to be done more than once
                    PlaceName = null;
                    return place.Coordinate;
                }

                AppManager.GetService<ITrace>()
                    .Warn("init", "Cannot resolve '{0}', will use default latitude/longitude", PlaceName);
            }
            return coordinate;
        }

        #endregion
    }
}
