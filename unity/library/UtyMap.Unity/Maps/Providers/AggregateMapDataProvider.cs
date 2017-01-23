using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyMap.Unity.Maps.Providers.Elevation;
using UtyMap.Unity.Maps.Providers.Geo;

namespace UtyMap.Unity.Maps.Providers
{
    /// <summary> 
    ///     Aggregates different map data providers and decides which one to use for specific tiles.
    /// </summary>
    internal sealed class AggregateMapDataProvider : MapDataProvider, IConfigurable
    {
        // NOTE make these ranges configurable to control what should be
        // loaded on specific at specific LOD.
        private readonly Range<int> OsmTileRange = new Range<int>(16, 16);
        private readonly Range<int> ElevationTileRange = new Range<int>(15, 16);

        private readonly OpenStreetMapDataProvider _osmMapDataProvider;
        private readonly MapzenMapDataProvider _mapzenMapDataProvider;

        private readonly MapzenElevationDataProvider _mapzenElevationDataProvider;
        private readonly SrtmElevationDataProvider _srtmElevationDataProvider;

        private ElevationDataType _eleDataType;
       
        [Dependency]
        public AggregateMapDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace)
        {
            _osmMapDataProvider = new OpenStreetMapDataProvider(fileSystemService, networkService, trace);
            _mapzenMapDataProvider = new MapzenMapDataProvider(fileSystemService, networkService, trace);

            _mapzenElevationDataProvider = new MapzenElevationDataProvider(fileSystemService, networkService, trace);
            _srtmElevationDataProvider = new SrtmElevationDataProvider(fileSystemService, networkService, trace);
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _osmMapDataProvider.Configure(configSection);
            _mapzenMapDataProvider.Configure(configSection);

            _srtmElevationDataProvider.Configure(configSection);
            _mapzenElevationDataProvider.Configure(configSection);

            _eleDataType = (ElevationDataType) configSection.GetInt("data/elevation/type", 2);
        }

        public override void OnNext(Tile value)
        {
            DownloadElevationData(value);
            DownloadMapData(value);
        }

        private void DownloadElevationData(Tile value)
        {
            if (!ElevationTileRange.Contains(value.QuadKey.LevelOfDetail))
                return;

            if (_eleDataType == ElevationDataType.Grid)
                _mapzenElevationDataProvider.OnNext(value);
            else if (_eleDataType == ElevationDataType.Srtm)
                _srtmElevationDataProvider.OnNext(value);
        }

        private void DownloadMapData(Tile value)
        {
            if (CoreLibrary.HasData(value.QuadKey))
                return;

            if (OsmTileRange.Contains(value.QuadKey.LevelOfDetail))
                _osmMapDataProvider.OnNext(value);
            else 
                _mapzenMapDataProvider.OnNext(value);
        }
    }
}
