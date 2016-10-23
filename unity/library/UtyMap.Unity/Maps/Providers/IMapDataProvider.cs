using System.IO;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyRx;

namespace UtyMap.Unity.Maps.Providers
{
    /// <summary> Provides file path to map data. </summary>
    public interface IMapDataProvider
    {
        /// <summary> Called when new tile is loading. </summary>
        IObservable<string> Get(Tile tile);
    }

    /// <summary> Provides basic functionality for map data downloading and file storing. </summary>
    public abstract class MapDataProvider : IMapDataProvider
    {
        private const string TraceCategory = "mapdata.provider.osm";
        private readonly object _lockObj = new object();

        private readonly IFileSystemService _fileSystemService;
        private readonly INetworkService _networkService;
        private readonly ITrace _trace;

        protected MapDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace)
        {
            _fileSystemService = fileSystemService;
            _networkService = networkService;
            _trace = trace;
        }

        /// <inheritdoc />
        public abstract IObservable<string> Get(Tile tile);

        protected IObservable<string> Get(Tile tile, string uri, string filePath)
        {
            if (_fileSystemService.Exists(filePath))
                return Observable.Return<string>(filePath);

            return Observable.Create<string>(observer =>
            {
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
                                    WriteBytes(stream, bytes);
                        }
                        observer.OnNext(filePath);
                        observer.OnCompleted();
                    });

                return Disposable.Empty;
            });
        }

        /// <summary> Writes bytes to file with preprocessing if necessary. </summary>
        protected virtual void WriteBytes(Stream stream, byte[] bytes)
        {
            stream.Write(bytes, 0, bytes.Length);
        }
    }
}
