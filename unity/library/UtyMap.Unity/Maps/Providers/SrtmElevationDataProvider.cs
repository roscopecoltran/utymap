using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UtyDepend.Config;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.IO.Compression;
using UtyRx;

namespace UtyMap.Unity.Maps.Providers
{
    /// <summary> Downloads SRTM data from NASA server. </summary>
    internal class SrtmElevationDataProvider : IMapDataProvider, IConfigurable
    {
        private const string TraceCategory = "mapdata.srtm";
        private readonly object _lock = new object();

        private readonly IFileSystemService _fileSystemService;
        private readonly INetworkService _networkService;
        private readonly ITrace _trace;
        private string _server;
        private string _schemaPath;
        private string _elePath;

        private static readonly Dictionary<int, string> ContinentMap = new Dictionary<int, string>
        {
            {0, "Eurasia"},
            {1, "South_America"},
            {2, "Africa"},
            {3, "North_America"},
            {4, "Australia"},
            {5, "Islands"},
        };      

        /// <summary> Creates instance of <see cref="SrtmElevationDataProvider"/>. </summary>
        public SrtmElevationDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace)
        {
            _fileSystemService = fileSystemService;
            _networkService = networkService;
            _trace = trace;
        }

        #region Public methods

        /// <inheritdoc />
        public IObservable<string> Get(Tile tile)
        {
            // TODO tile can cross more than one srtm cell: need load more.
            var prefix = GetFileNamePrefix(tile.BoundingBox.Center());
            var filePath = Path.Combine(_elePath, prefix);

            if (_fileSystemService.Exists(filePath))
                return Observable.Return(filePath);

            return Download(prefix).Select(bytes =>
            {
                lock (_lock)
                {
                    if (!_fileSystemService.Exists(filePath))
                    {
                        _trace.Info(TraceCategory, "Unzipping {0} bytes", bytes.Length.ToString());
                        var hgtData = CompressionUtils.Unzip(bytes).Single().Value;
                        using (var stream = _fileSystemService.WriteStream(filePath))
                            stream.Write(hgtData, 0, hgtData.Length);
                    }
                }

                return filePath;
            });
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _server = configSection.GetString(@"data/srtm/server");
            _schemaPath = configSection.GetString(@"data/srtm/schema");
            _elePath = configSection.GetString(@"data/elevation/local");
        }

        #endregion

        private string GetFileNamePrefix(GeoCoordinate coordinate)
        {
            return String.Format("{0}{1:00}{2}{3:000}",
                coordinate.Latitude > 0 ? 'N' : 'S', Math.Abs((int)coordinate.Latitude),
                coordinate.Longitude > 0 ? 'E' : 'W', Math.Abs((int)coordinate.Longitude));
        }

        /// <summary> Download SRTM data for given coordinate. </summary>
        private IObservable<byte[]> Download(String prefix)
        {
            // TODO do not read schema file for every srtm cell.
            foreach (var line in _fileSystemService.ReadText(_schemaPath).Split('\n'))
            {
                if (line.StartsWith(prefix))
                {
                    var parameters = line.Split(' ');
                    // NOTE some of files miss extension point between name and .hgt.zip
                    var url = String.Format("{0}/{1}/{2}", _server, ContinentMap[int.Parse(parameters[1])],
                        parameters[1].EndsWith("zip") ? "" : parameters[0] + ".hgt.zip");
                    _trace.Warn(TraceCategory, String.Format("Downloading SRTM data from {0}", url));
                    return _networkService.GetAndGetBytes(url);
                }
            }
            return Observable.Throw<byte[]>(new ArgumentException(
                String.Format("Cannot find {0} on {1}", prefix, _schemaPath)));
        }
    }
}
