using System;
using System.IO;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyRx;

namespace UtyMap.Unity.Maps.Providers
{
    /// <summary> Downloads map data from mapzen servers. </summary>
    public class MapzenMapDataProvider : MapDataProvider, IConfigurable
    {
        private string _cachePath;

        private string _mapDataServerUri;
        private string _mapDataFormatExtension;
        private string _mapDataLayers;
        private string _mapDataApiKey;
        
        [Dependency]
        public MapzenMapDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace) :
            base(fileSystemService, networkService, trace)
        {
        }

        /// <inheritdoc />
        public override IObservable<string> Get(Tile tile)
        {
            var filePath = Path.Combine(_cachePath, tile.QuadKey + _mapDataFormatExtension);
            var uri = String.Format(_mapDataServerUri, _mapDataLayers, tile.QuadKey.LevelOfDetail, tile.QuadKey.TileX, 
                tile.QuadKey.TileY, _mapDataApiKey);

            return Get(tile, uri, filePath);
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _mapDataServerUri = configSection.GetString(@"data/mapzen/server", null);
            _mapDataFormatExtension = "." + configSection.GetString(@"data/mapzen/format", "xml");
            _mapDataApiKey = configSection.GetString(@"data/mapzen/apikey", null);
            _mapDataLayers = configSection.GetString(@"data/mapzen/layers", null);

            _cachePath = configSection.GetString(@"data/cache", null);
        }
    }
}
