using System;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Positioning;
using UtyMap.Unity.Infrastructure;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Maps.Geocoding;
using UtyRx;

namespace Assets.Scripts.Menu
{
    public class PlayGamePanelManager: PanelManager
    {
        private const string LogCategory = "PlayGame";

        public InputField NameInputField;
        public InputField CoordinateInputField;
        public Button SearchButton;
        public Button LocateButton;

        private ITrace _trace;
        private IGeocoder _geoCoder;

        private GeocoderResult[] _results;
        private int _currentIndex = 0;
        private bool _isSearchClick = true;
        private GeoPosition _geoPosition;

        void Start()
        {
            _trace = ApplicationManager.Instance.GetService<ITrace>();
            _geoCoder = ApplicationManager.Instance.GetService<IGeocoder>();

            // NOTE Set some defaults to get user an idea what these fields about.
            NameInputField.text = "Moscow, Red Square";
            CoordinateInputField.text = (new GeoCoordinate(55.75396, 37.62050)).ToString();

            NameInputField.onEndEdit.AddListener(_ =>
            {
                SearchButton.GetComponentInChildren<Text>().text = "Search by name";
                _isSearchClick = true;
            });

            LocateButton.enabled = false;
            var messageBus = ApplicationManager.Instance.GetService<IMessageBus>();
            messageBus
                .AsObservable<GeoPosition>()
                .Subscribe(OnGeoPosition);
        }

        private void OnGeoPosition(GeoPosition geoPosition)
        {
            _geoPosition = geoPosition;
            LocateButton.enabled = _geoPosition != null;
        }

        public void OnLocateClick()
        {
            // TODO perform reverse geocoding request
            NameInputField.text = "<Device location>";
            CoordinateInputField.text = _geoPosition.Coordinate.ToString();
            _geoCoder
                  .Search(_geoPosition.Coordinate)
                  .SubscribeOn(Scheduler.MainThread)
                  .ObserveOn(Scheduler.MainThread)
                  .Where(r => r != null && !String.IsNullOrEmpty(r.DisplayName))
                  .Subscribe(result =>
                  {
                      NameInputField.text = result.DisplayName;
                  });
        }

        public void OnSearchClick()
        {
            if (_isSearchClick)
            {
                _currentIndex = 0;
                _isSearchClick = false;
                _geoCoder.Search(NameInputField.text)
                    .ToArray()
                    .SubscribeOn(Scheduler.MainThread)
                    .ObserveOn(Scheduler.MainThread)
                    .Subscribe(results =>
                    {
                        _results = results;
                        ShowResult();
                    });
            }
            else
            {
                _currentIndex++;
                ShowResult();
            }
        }

        private void ShowResult()
        {
            if (_results == null || _results.Length == 0)
                return;

            if (_currentIndex == _results.Length)
                _currentIndex = 0;

            SearchButton.GetComponentInChildren<Text>().text =
                String.Format("Result {0} of {1}", _currentIndex + 1, _results.Length);

            NameInputField.text = _results[_currentIndex].DisplayName;
            CoordinateInputField.text = _results[_currentIndex].Coordinate.ToString();
        }

        public void OnStreetLevelPlayClick()
        {
            var coordText = CoordinateInputField.text;
            _trace.Info(LogCategory, "Parsing GeoCoordinate from: {0}", coordText);

            var coordParts = coordText.Split(',');
            StreetLevelBehaviour.StartCoordinate = new GeoCoordinate(
                double.Parse(coordParts[0]),
                double.Parse(coordParts[1]));

            SceneManager.LoadScene("StreetLevelScene");
        }

        public void OnBirdEyeLevelPlayClick()
        {
            SceneManager.LoadScene("BirdEyeLevelScene");
        }

        public void OnGlobeLevelPlayClick()
        {
            SceneManager.LoadScene("GlobeLevelScene");
        }

        public void Quit()
        {
#if UNITY_EDITOR
            UnityEditor.EditorApplication.isPlaying = false;
#else
		    UnityEngine.Application.Quit();
#endif
        }
    }
}
