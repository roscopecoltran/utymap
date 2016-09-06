using System;
using System.Collections;
using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Positioning;
using UtyMap.Unity.Infrastructure;
using UtyMap.Unity.Infrastructure.Diagnostic;

namespace Assets.Scripts
{
    internal class DeviceLocation : MonoBehaviour
    {
        private const string TraceCategory = "device.location";

        public int UpdateFrequencyInSeconds = 5;

        private ITrace _trace;
        private IMessageBus _messageBus;
        private bool _isInitialized;

        void Start()
        {
            _trace = ApplicationManager.Instance.GetService<ITrace>();
            _messageBus = ApplicationManager.Instance.GetService<IMessageBus>();

            StartCoroutine(PositionUpdate());
        }

        IEnumerator PositionUpdate()
        {
            // First, check if user has location service enabled
            if (!Input.location.isEnabledByUser)
            {
                _trace.Warn(TraceCategory, "LocationService is not enabled.");
                yield break;
            }

            // Start service before querying location
            Input.location.Start();

            // Wait until service initializes
            int maxWait = 20;
            while (Input.location.status == LocationServiceStatus.Initializing && maxWait > 0)
            {
                yield return new WaitForSeconds(1);
                maxWait--;
            }

            // Service didn't initialize in 20 seconds
            if (maxWait < 1)
            {
                _trace.Warn(TraceCategory, "Timed out");
                yield break;
            }

            // Connection has failed
            if (Input.location.status == LocationServiceStatus.Failed)
            {
                _trace.Warn(TraceCategory, "Unable to determine device location");
                yield break;
            }

            while (true)
            {
                LocationInfo info = Input.location.lastData;

                _trace.Debug(TraceCategory, String.Format("LocationInfo: ({0}, {1}) time: {2} accuracy: {3}",
                    info.latitude, info.longitude, info.timestamp, info.horizontalAccuracy));

                GeoPosition position = new GeoPosition();
                position.Coordinate = new GeoCoordinate(info.latitude, info.longitude);
                position.Time =
                    new TimeSpan(new DateTime(1970, 1, 1, 0, 0, 0, 0, DateTimeKind.Utc).AddSeconds(info.timestamp).Ticks);

                _messageBus.Send(position);

                yield return new WaitForSeconds(UpdateFrequencyInSeconds);
            }
        }
    }
}
