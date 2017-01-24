using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Data.Providers.Elevation;
using UtyMap.Unity.Data.Providers.Geo;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyRx;

namespace UtyMap.Unity.Data.Providers
{
    /// <summary> 
    ///     Aggregates different map data providers and decides which one to use for specific tiles.
    /// </summary>
    internal sealed class AggregateMapDataProvider : MapDataProvider
    {
        private IMapDataProvider _eleProvider;
        private IMapDataProvider _dataProvider;
       
        [Dependency]
        public AggregateMapDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace)
        {
            _eleProvider = new ElevationProvider(
                new MapzenElevationDataProvider(fileSystemService, networkService, trace),
                new SrtmElevationDataProvider(fileSystemService, networkService, trace));

            _dataProvider = new DataProvider(
                new OpenStreetMapDataProvider(fileSystemService, networkService, trace),
                new MapzenMapDataProvider(fileSystemService, networkService, trace));

            _eleProvider.Subscribe(t => _dataProvider.OnNext(t.Item1));
            _dataProvider.Subscribe(Notify);
        }

        /// <inheritdoc />
        public override void OnNext(Tile value)
        {
            _eleProvider.OnNext(value);
        }

        /// <inheritdoc />
        public override void Configure(IConfigSection configSection)
        {
            _eleProvider.Configure(configSection);
            _dataProvider.Configure(configSection);
        }

        #region Nested classes

        /// <summary> Encapsulates elevation processing. </summary>
        private class ElevationProvider : MapDataProvider
        {
            private readonly Range<int> ElevationTileRange = new Range<int>(15, 16);

            private ElevationDataType _eleDataType = ElevationDataType.Flat;
            
            private readonly IMapDataProvider _mapzenEleProvider;
            private readonly IMapDataProvider _srtmEleProvider;

            public ElevationProvider(IMapDataProvider mapzenEleProvider, IMapDataProvider srtmEleProvider)
            {
                _mapzenEleProvider = mapzenEleProvider;
                _srtmEleProvider = srtmEleProvider;
            }

            /// <inheritdoc />
            public override void OnNext(Tile value)
            {
                if (_eleDataType != ElevationDataType.Flat &&  ElevationTileRange.Contains(value.QuadKey.LevelOfDetail))
                {
                    if (_eleDataType == ElevationDataType.Grid)
                        _mapzenEleProvider.OnNext(value);
                    else
                        _srtmEleProvider.OnNext(value);
                }
                else
                    Notify(new Tuple<Tile, string>(value, ""));
            }

            /// <inheritdoc />
            public override void Configure(IConfigSection configSection)
            {
                _eleDataType = (ElevationDataType) configSection.GetInt("data/elevation/type", 2);
                
                _mapzenEleProvider.Configure(configSection);
                _mapzenEleProvider.Subscribe(Notify);

                _srtmEleProvider.Configure(configSection);
                _srtmEleProvider.Subscribe(Notify);
            }
        }

        /// <summary> Encapsulates map data processing. </summary>
        class DataProvider : MapDataProvider
        {
            private readonly Range<int> OsmTileRange = new Range<int>(16, 16);

            private readonly IMapDataProvider _osmMapDataProvider;
            private readonly IMapDataProvider _mapzenMapDataProvider;

            public DataProvider(IMapDataProvider osmMapDataProvider, IMapDataProvider mapzenMapDataProvider)
            {
                _osmMapDataProvider = osmMapDataProvider;
                _mapzenMapDataProvider = mapzenMapDataProvider;
            }

            /// <inheritdoc />
            public override void OnNext(Tile value)
            {
                if (CoreLibrary.HasData(value.QuadKey))
                {
                    Notify(new Tuple<Tile, string>(value, ""));
                    return;
                }

                if (OsmTileRange.Contains(value.QuadKey.LevelOfDetail))
                    _osmMapDataProvider.OnNext(value);
                else
                    _mapzenMapDataProvider.OnNext(value);
            }

            /// <inheritdoc />
            public override void Configure(IConfigSection configSection)
            {
                _osmMapDataProvider.Configure(configSection);
                _osmMapDataProvider.Subscribe(Notify);

                _mapzenMapDataProvider.Configure(configSection);
                _mapzenMapDataProvider.Subscribe(Notify);
            }
        }

        #endregion
    }
}
