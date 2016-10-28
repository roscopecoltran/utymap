using System;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyRx;

namespace UtyMap.Unity.Maps.Providers
{
    /// <summary> 
    ///     Aggregates different map data providers and decides which one to use for specific tiles. 
    /// </summary>
    internal sealed class AggregateMapDataProvider : IMapDataProvider, IConfigurable
    {
        private readonly Range<int> OsmTileRange = new Range<int>(16, 16);

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
        public IObservable<string> Get(Tile tile)
        {
            return CreateEleDataObservable(tile)
                    .SelectMany(t => CreateMapDataObservable(tile));
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _osmMapDataProvider.Configure(configSection);
            _mapzenMapDataProvider.Configure(configSection);
            _eleDataType = (ElevationDataType)configSection.GetInt("data/elevation/type", 0);
        }

        private IObservable<string> CreateEleDataObservable(Tile tile)
        {
            switch (_eleDataType)
            {
                case ElevationDataType.Grid:
                    return _mapzenElevationDataProvider.Get(tile);
                case ElevationDataType.Srtm:
                    return _srtmElevationDataProvider.Get(tile);
            }
            return Observable.Return("");
        }

        private IObservable<string> CreateMapDataObservable(Tile tile)
        {
            if (OsmTileRange.Contains(tile.QuadKey.LevelOfDetail))
                return _osmMapDataProvider.Get(tile);

            return _mapzenMapDataProvider.Get(tile);
        }
    }
}
