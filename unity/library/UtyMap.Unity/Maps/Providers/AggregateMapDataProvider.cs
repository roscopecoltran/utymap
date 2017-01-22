using System;
using System.Collections.Generic;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Core;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyMap.Unity.Maps.Providers.Elevation;
using UtyMap.Unity.Maps.Providers.Geo;
using UtyRx;

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

            _eleDataType = (ElevationDataType)configSection.GetInt("data/elevation/type", 2);
        }

        public override void OnNext(QuadKey quadKey)
        {
            DownloadElevationData(quadKey);
            DownloadMapData(quadKey);
        }

        private void DownloadElevationData(QuadKey quadKey)
        {
            if (!ElevationTileRange.Contains(quadKey.LevelOfDetail))
                return;

            if (_eleDataType == ElevationDataType.Grid)
                _mapzenElevationDataProvider.OnNext(quadKey);
            else
                _srtmElevationDataProvider.OnNext(quadKey);
        }

        private void DownloadMapData(QuadKey quadKey)
        {
            if (CoreLibrary.HasData(quadKey))
                return;

            if (OsmTileRange.Contains(quadKey.LevelOfDetail))
                _osmMapDataProvider.OnNext(quadKey);
            else 
                _mapzenMapDataProvider.OnNext(quadKey);
        }
    }
}
