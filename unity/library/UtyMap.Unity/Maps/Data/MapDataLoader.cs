using System;
using System.IO;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Maps.Providers;
using UtyRx;
using Mesh = UtyMap.Unity.Core.Models.Mesh;

namespace UtyMap.Unity.Maps.Data
{
    /// <summary> Defines behavior of class responsible to mapdata processing. </summary>
    public interface IMapDataLoader
    {
        /// <summary> Adds mapdata to the specific storage. </summary>
        /// <param name="storage"> Storage type. </param>
        /// <param name="dataPath"> Path to mapdata. </param>
        /// <param name="stylesheet"> Stylesheet which to use during import. </param>
        /// <param name="levelOfDetails"> Which level of details to use. </param>
        void AddToStore(MapStorageType storage, string dataPath, Stylesheet stylesheet, Range<int> levelOfDetails);

        /// <summary> Loads given tile. This method triggers real loading and processing osm data. </summary>
        /// <param name="tile">Tile to load.</param>
        IObservable<Union<Element, Mesh>> Load(Tile tile);
    }

    /// <summary> Default implementation of tile loader. </summary>
    internal class MapDataLoader : IMapDataLoader, IDisposable, IConfigurable
    {
        private const string TraceCategory = "mapdata.loader";

        private readonly IMapDataProvider _mapDataProvider;
        private readonly IPathResolver _pathResolver;
        private ElevationDataType _eleDataType;

        [Dependency]
        public ITrace Trace { get; set; }

        [Dependency]
        public MapDataLoader(IMapDataProvider mapDataProvider, IPathResolver pathResolver)
        {
            _mapDataProvider = mapDataProvider;
            _pathResolver = pathResolver;
        }

        #region Interface implementations

        /// <inheritdoc />
        public void AddToStore(MapStorageType storageType, string dataPath, Stylesheet stylesheet, Range<int> levelOfDetails)
        {
            var dataPathResolved = _pathResolver.Resolve(dataPath);
            var stylesheetPathResolved = _pathResolver.Resolve(stylesheet.Path);

            Trace.Info(TraceCategory, String.Format("add to {0} storage: data:{1} using style: {2}",
                storageType, dataPathResolved, stylesheetPathResolved));

            string errorMsg = null;
            CoreLibrary.AddToStore(storageType, stylesheetPathResolved, dataPathResolved, levelOfDetails,
                error => errorMsg = error);

            if (errorMsg != null)
                throw new MapDataException(errorMsg);
        }

        /// <inheritdoc />
        public IObservable<Union<Element, Mesh>> Load(Tile tile)
        {
            if (CoreLibrary.HasData(tile.QuadKey))
                return CreateLoadSequence(tile);

            return _mapDataProvider
                    .Get(tile)
                    .SelectMany(filePath =>
                    {
                        SaveTileDataInMemory(tile, filePath);
                        return CreateLoadSequence(tile);
                    });
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            var stringPath = _pathResolver.Resolve(configSection.GetString("data/index/strings"));
            var mapDataPath = _pathResolver.Resolve(configSection.GetString("data/index/spatial"));
            var elePath = _pathResolver.Resolve(configSection.GetString("data/elevation/local"));
            _eleDataType = (ElevationDataType) configSection.GetInt("data/elevation/type", 0);

            string errorMsg = null;
            CoreLibrary.Configure(stringPath, mapDataPath, elePath, error => errorMsg = error);
            if (errorMsg != null)
                throw new MapDataException(errorMsg);
        }

        /// <inheritdoc />
        public void Dispose()
        {
            CoreLibrary.Dispose();
        }

        #endregion

        #region Private methods

        /// <summary> Creates <see cref="IObservable{T}"/> for loading element of given tile. </summary>
        private IObservable<Union<Element, Mesh>> CreateLoadSequence(Tile tile)
        {
            return Observable.Create<Union<Element, Mesh>>(observer =>
            {
                var stylesheetPathResolved = _pathResolver.Resolve(tile.Stylesheet.Path);

                Trace.Info(TraceCategory, "loading tile: {0} using style: {1}", tile.ToString(), stylesheetPathResolved);
                var adapter = new MapTileAdapter(tile, observer, Trace);

                CoreLibrary.LoadQuadKey(
                    stylesheetPathResolved,
                    tile.QuadKey,
                    _eleDataType,
                    adapter.AdaptMesh,
                    adapter.AdaptElement,
                    adapter.AdaptError);

                Trace.Info(TraceCategory, "tile loaded: {0}", tile.ToString());
                observer.OnCompleted();

                return Disposable.Empty;
            });
        }

        private void SaveTileDataInMemory(Tile tile, string filePath)
        {
            var filePathResolved = _pathResolver.Resolve(filePath);
            var stylesheetPathResolved = _pathResolver.Resolve(tile.Stylesheet.Path);

            Trace.Info(TraceCategory, String.Format("save tile data {0} from {1} using style: {2}",
                tile, filePathResolved, stylesheetPathResolved));

            string errorMsg = null;
            CoreLibrary.AddToStore(MapStorageType.Persistent,
                stylesheetPathResolved,
                filePathResolved,
                tile.QuadKey,
                error => errorMsg = error);

            if (errorMsg != null)
                throw new MapDataException(String.Format(Strings.CannotAddDataToInMemoryStore, errorMsg));
        }

        #endregion
    }
}
