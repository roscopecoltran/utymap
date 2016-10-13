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
    /// <summary> Downloads data from openstreemap servers. </summary>
    public class OpenStreetMapDataProvider : IMapDataProvider, IConfigurable
    {
        private const string TraceCategory = "mapdata.provider.osm";
        private readonly object _lockObj = new object();

        private readonly IFileSystemService _fileSystemService;
        private readonly INetworkService _networkService;
        private readonly ITrace _trace;

        private string _cachePath;
        private string _mapDataFormatExtension;
        private string _mapDataServerQuery;
        private string _mapDataServerUri;

        [Dependency]
        public OpenStreetMapDataProvider(IFileSystemService fileSystemService, 
            INetworkService networkService, ITrace trace)
        {
            _fileSystemService = fileSystemService;
            _networkService = networkService;
            _trace = trace;
        }

        /// <inheritdoc />
        public IObservable<string> Get(Tile tile)
        {
            var filePath = GetCacheFilePath(tile);
            if (_fileSystemService.Exists(filePath))
                return Observable.Return<string>(filePath);

            return Observable.Create<string>(observer =>
            {
                var padding = 0.001;
                var query = tile.BoundingBox;
                var queryString = String.Format(_mapDataServerQuery,
                    query.MinPoint.Latitude - padding, query.MinPoint.Longitude - padding,
                    query.MaxPoint.Latitude + padding, query.MaxPoint.Longitude + padding);
                var uri = String.Format("{0}{1}", _mapDataServerUri, Uri.EscapeDataString(queryString));
                _trace.Warn(TraceCategory, Strings.NoPresistentElementSourceFound, tile.QuadKey.ToString(), uri);
                _networkService.GetAndGetBytes(uri)
                    .ObserveOn(Scheduler.ThreadPool)
                    .Subscribe(bytes =>
                    {
                        _trace.Debug(TraceCategory, "saving bytes: {0}", bytes.Length.ToString());
                        lock (_lockObj)
                        {
                            if (!_fileSystemService.Exists(filePath))
                                using (var stream = _fileSystemService.WriteStream(filePath))
                                    stream.Write(bytes, 0, bytes.Length);
                        }
                        observer.OnNext(filePath);
                        observer.OnCompleted();
                    });

                return Disposable.Empty;
            });
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _mapDataServerUri = configSection.GetString(@"data/remote/server", null);
            _mapDataServerQuery = configSection.GetString(@"data/remote/query", null);
            _mapDataFormatExtension = "." + configSection.GetString(@"data/remote/format", "xml");
            _cachePath = configSection.GetString(@"data/cache", null);
        }

        #region Private methods

        /// <summary> Returns cache file name for given tile. </summary>
        private string GetCacheFilePath(Tile tile)
        {
            var cacheFileName = tile.QuadKey + _mapDataFormatExtension;
            return Path.Combine(_cachePath, cacheFileName);
        }
        
        #endregion
    }
}