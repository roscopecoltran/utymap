using System.Collections;
using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Utils;
using UtyMap.Unity.Maps.Geocoding;
using UtyRx;

namespace Assets.Scripts
{
    /// <summary> Display current address using reverse geocoding API. </summary>
    internal class AddressBehaviour : MonoBehaviour
    {
        // NOTE we cannot request frequently: geo coding server may ban application
        public int UpdateFrequencyInSeconds = 20;
        public int DistanceThreshold = 75;

        private IGeocoder _geoCoder;
        private GeoCoordinate _startCoordinate;
        private GeocoderResult _currentAddress;
        private Vector3 _lastPosition = new Vector3(float.MinValue, float.MinValue, float.MinValue);

        void Start()
        {
            _startCoordinate = PositionConfiguration.StartPosition;

            _geoCoder = ApplicationManager.Instance.GetService<IGeocoder>();
           
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
                        .Subscribe(r => _currentAddress = r);
                }

                yield return new WaitForSeconds(UpdateFrequencyInSeconds);
            }
        }

        void OnGUI()
        {
            var currentAddress = _currentAddress;
            if (currentAddress != null)
            {
                var offset = Screen.width/6;
                GUI.Box(new Rect(offset, 0, Screen.width - offset * 2, 30), currentAddress.DisplayName);
            }
        }
    }
}