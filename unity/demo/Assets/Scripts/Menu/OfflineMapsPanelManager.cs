using UnityEngine.UI;
using UtyMap.Unity.Core;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyRx;

namespace Assets.Scripts.Menu
{
    public class OfflineMapsPanelManager : PanelManager
    {
        private IMapDataLoader _mapDataLoader;
        private IPathResolver _pathResolver;
        private Stylesheet _stylesheet;

        private void Start()
        {
            _pathResolver = ApplicationManager.Instance.GetService<IPathResolver>();
            _mapDataLoader = ApplicationManager.Instance.GetService<IMapDataLoader>();
            _stylesheet = ApplicationManager.Instance.GetService<Stylesheet>();

            InitializeImportPaths();
        }

        #region Map import logic

        public InputField SourcePathField;
        public InputField DestinationPathField;
        public Button ImportButton;

        private void InitializeImportPaths()
        {
            // TODO Allow user to redefine index location
            DestinationPathField.text = _pathResolver.Resolve("Index/");
            DestinationPathField.readOnly = true;
        }

        public void OnImportClick()
        {
            var sourcePath = SourcePathField.text;
            Observable.Start(() =>
            {
                // TODO Allow user to change import parameters (e.g. zoom)
                _mapDataLoader.AddToStore(MapStorageType.Persistent, sourcePath, _stylesheet, new Range<int>(16, 16));
            }, Scheduler.ThreadPool);
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