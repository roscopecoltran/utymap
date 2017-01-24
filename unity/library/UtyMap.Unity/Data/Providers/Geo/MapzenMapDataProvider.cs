using System;
using System.IO;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;

namespace UtyMap.Unity.Data.Providers.Geo
{
    /// <summary> Downloads map data from mapzen servers. </summary>
    internal class MapzenMapDataProvider : RemoteMapDataProvider
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
        public override void Configure(IConfigSection configSection)
        {
            _mapDataServerUri = configSection.GetString(@"data/mapzen/server", null);
            _mapDataFormatExtension = "." + configSection.GetString(@"data/mapzen/format", "json");
            _mapDataApiKey = configSection.GetString(@"data/mapzen/apikey", null);
            _mapDataLayers = configSection.GetString(@"data/mapzen/layers", null);

            _cachePath = configSection.GetString(@"data/cache", null);
        }

        /// <inheritdoc />
        protected override string GetUri(QuadKey quadKey)
        {
            return Path.Combine(_cachePath, quadKey + _mapDataFormatExtension);
        }

        /// <inheritdoc />
        protected override string GetFilePath(QuadKey quadKey)
        {
            return String.Format(_mapDataServerUri, _mapDataLayers, quadKey.LevelOfDetail, quadKey.TileX,
                quadKey.TileY, _mapDataApiKey);
        }
    }
}
