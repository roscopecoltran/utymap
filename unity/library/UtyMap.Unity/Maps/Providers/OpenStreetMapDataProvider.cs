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
    /// <summary> Downloads map data from openstreemap servers. </summary>
    public class OpenStreetMapDataProvider : MapDataProvider, IConfigurable
    {
        private string _cachePath;
        private string _mapDataFormatExtension;
        private string _mapDataServerQuery;
        private string _mapDataServerUri;

        [Dependency]
        public OpenStreetMapDataProvider(IFileSystemService fileSystemService, 
                                         INetworkService networkService,
                                         ITrace trace)
            : base(fileSystemService, networkService, trace)
        {
        }

        /// <inheritdoc />
        public override IObservable<string> Get(Tile tile)
        {
            var filePath = Path.Combine(_cachePath, tile.QuadKey + _mapDataFormatExtension);
            var padding = 0.001;
            var query = tile.BoundingBox;
            var queryString = String.Format(_mapDataServerQuery,
                query.MinPoint.Latitude - padding, query.MinPoint.Longitude - padding,
                query.MaxPoint.Latitude + padding, query.MaxPoint.Longitude + padding);
            var uri = String.Format("{0}{1}", _mapDataServerUri, Uri.EscapeDataString(queryString));

            return Get(tile, uri, filePath);
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _mapDataServerUri = configSection.GetString(@"data/osm/server", null);
            _mapDataServerQuery = configSection.GetString(@"data/osm/query", null);
            _mapDataFormatExtension = "." + configSection.GetString(@"data/osm/format", "xml");
            _cachePath = configSection.GetString(@"data/cache", null);
        }
    }
}
