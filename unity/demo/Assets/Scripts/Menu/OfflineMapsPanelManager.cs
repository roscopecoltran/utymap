using System;
using Assets.Scripts.Environment;
using UnityEngine.UI;
using UtyMap.Unity.Core;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyRx;

namespace Assets.Scripts.Menu
{
    public class OfflineMapsPanelManager : PanelManager
    {
        private IMapDataLoader _mapDataLoader;
        private Stylesheet _stylesheet;

        void Start()
        {
            _mapDataLoader = ApplicationManager.Instance.GetService<IMapDataLoader>();
            _stylesheet = ApplicationManager.Instance.GetService<Stylesheet>();

            InitializeImportPaths();
        }

        #region Map import logic

        public InputField SourcePathField;
        public InputField DestinationPathField;
        public Button ImportButton;
        public Toggle StorageTypeToggle;

        public Slider MinSlider;
        public Slider MaxSlider;

        private void InitializeImportPaths()
        {
            DestinationPathField.text = EnvironmentApi.ExternalDataPath;
            DestinationPathField.readOnly = true;
        }

        public void OnImportClick()
        {
            var sourcePath = SourcePathField.text;
            var storageType = StorageTypeToggle.isOn ? MapStorageType.InMemory : MapStorageType.Persistent;
            var range = new Range<int>((int) MinSlider.value, (int) MaxSlider.value);

            // NOTE if button is not enabled then import is still in progress
            // or error occured.
            if (!ImportButton.enabled || String.IsNullOrEmpty(sourcePath))
                return;

            ImportButton.enabled = false;

            Observable
                .Start(() => _mapDataLoader.AddToStore(storageType, sourcePath, _stylesheet, range))
                .SubscribeOn(Scheduler.ThreadPool)
                .ObserveOn(Scheduler.MainThread)
                .Subscribe(_ => { }, () => ImportButton.enabled = true);
        }

        #endregion

        #region Map list logic

        public Text InstalledMapText;

        public void OnUpdateList()
        {
            // TODO update InstalledMapText with relevant data.
        }

        #endregion
    }
}