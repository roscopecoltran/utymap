using System.Collections;
using Assets.Scripts.Console;
using UnityEngine;
using UnityEngine.UI;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Utils;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Maps.Geocoding;
using UtyRx;

namespace Assets.Scripts.Extensions
{
    /// <summary> Display current address using reverse geocoding API. </summary>
    internal class AddressBehaviour : MonoBehaviour
    {
        // NOTE we cannot request frequently: geo coding server may ban application
        public int UpdateFrequencyInSeconds = 20;
        public int DistanceThreshold = 75;
        public Text AddressText;
        public Text StatusText;

        private IGeocoder _geoCoder;
        private GeoCoordinate _startCoordinate;
        private Vector3 _lastPosition = new Vector3(float.MinValue, float.MinValue, float.MinValue);
        private DebugConsoleTrace _trace;

        void Start()
        {
            _startCoordinate = PositionConfiguration.StartPosition;
            _geoCoder = ApplicationManager.Instance.GetService<IGeocoder>();

            _trace = ApplicationManager.Instance.GetService<ITrace>() as DebugConsoleTrace;
            if (_trace != null)
                _trace.SetUIText(StatusText);
           
            StartCoroutine(RequestAddress());
        }

        IEnumerator RequestAddress()
        {
            while (true)
            {
                var position = transform.position;
                if (Vector3.Distance(position, _lastPosition) > DistanceThreshold)
                {
                    _lastPosition = position;
                    _geoCoder
                        .Search(GeoUtils.ToGeoCoordinate(_startCoordinate, position.x, position.z))
                        .Subscribe(r => AddressText.text = r.DisplayName);
                }

                yield return new WaitForSeconds(UpdateFrequencyInSeconds);
            }
        }

        void OnDestroy()
        {
            if (_trace != null)
                _trace.SetUIText(null);
        }
    }
}