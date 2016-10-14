using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
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
        
        [Dependency]
        public AggregateMapDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace)
        {
            _osmMapDataProvider = new OpenStreetMapDataProvider(fileSystemService, networkService, trace);
            _mapzenMapDataProvider = new MapzenMapDataProvider(fileSystemService, networkService, trace);
        }

        /// <inheritdoc />
        public IObservable<string> Get(Tile tile)
        {
            if (OsmTileRange.Contains(tile.QuadKey.LevelOfDetail)) 
                _osmMapDataProvider.Get(tile);

            return _mapzenMapDataProvider.Get(tile);
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _osmMapDataProvider.Configure(configSection);
            _mapzenMapDataProvider.Configure(configSection);
        }
    }
}
