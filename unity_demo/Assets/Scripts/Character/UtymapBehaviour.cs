using System;
using UnityEngine;
using Utymap.UnityLib;
using Utymap.UnityLib.Core;
using Utymap.UnityLib.Infrastructure.Config;
using Utymap.UnityLib.Infrastructure.Diagnostic;
using Utymap.UnityLib.Infrastructure.Reactive;
using Utymap.UnityLib.Maps.Geocoding;

using Component = Utymap.UnityLib.Infrastructure.Dependencies.Component;

namespace Assets.Scripts.Character
{
    /// <summary> Performs some initialization and listens for position changes of character.  </summary>
    class UtymapBehaviour : MonoBehaviour
    {
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

        /// <summary> Returns bootstrapper plugin. </summary>
        protected virtual Action<CompositionRoot> GetBootInitAction()
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
            AppManager.InitializeFramework(GetConfigBuilder(), GetBootInitAction());
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
