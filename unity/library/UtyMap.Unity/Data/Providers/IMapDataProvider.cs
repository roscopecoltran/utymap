using System;
using System.Collections.Generic;
using System.IO;
using UtyDepend.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyRx;

namespace UtyMap.Unity.Data.Providers
{
    /// <summary> Provides file path to map data. </summary>
    public interface IMapDataProvider : ISubject<Tile, Tuple<Tile, string>>, IConfigurable
    {
    }

    /// <summary> Generalzes subscription processing. </summary>
    internal abstract class MapDataProvider : IMapDataProvider
    {
        private readonly List<IObserver<Tuple<Tile, string>>> _observers = new List<IObserver<Tuple<Tile, string>>>();

        public abstract void OnNext(Tile value);
        public abstract void Configure(IConfigSection configSection);

        public virtual void OnCompleted()
        {
            _observers.ForEach(o => o.OnCompleted());
        }

        public virtual void OnError(Exception error)
        {
            _observers.ForEach(o => o.OnError(error));
        }

        public virtual IDisposable Subscribe(IObserver<Tuple<Tile, string>> observer)
        {
            // TODO handle unsubscribe
            _observers.Add(observer);
            return Disposable.Empty;
        }

        /// <summary> Notifies observers. </summary>
        protected void Notify(Tuple<Tile, string> value)
        {
            _observers.ForEach(o => o.OnNext(value));
        }
    }

    /// <summary> Provides basic functionality for map data downloading and file storing. </summary>
    internal abstract class RemoteMapDataProvider : MapDataProvider
    {
        private const string TraceCategory = "mapdata.provider.osm";
        private readonly object _lockObj = new object();

        private readonly IFileSystemService _fileSystemService;
        private readonly INetworkService _networkService;
        private readonly ITrace _trace;

        protected RemoteMapDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace)
        {
            _fileSystemService = fileSystemService;
            _networkService = networkService;
            _trace = trace;
        }

        public override void OnNext(Tile value)
        {
            var filePath = GetFilePath(value.QuadKey);
            if (_fileSystemService.Exists(filePath))
            {
                Notify(new Tuple<Tile, string>(value, filePath));
                return;
            }

            var uri = GetUri(value.QuadKey);
            _trace.Info(TraceCategory, Strings.NoPresistentElementSourceFound, value.ToString(), uri);
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
                    Notify(new Tuple<Tile, string>(value, filePath));
                });
        }

        /// <summary> Gets download uri for given quad key </summary>
        protected abstract string GetUri(QuadKey quadKey);

        /// <summary> Gets store file path for given quad key. </summary>
        protected abstract string GetFilePath(QuadKey quadKey);

        /// <summary> Writes bytes to file with preprocessing if necessary. </summary>
        protected virtual void WriteBytes(Stream stream, byte[] bytes)
        {
            stream.Write(bytes, 0, bytes.Length);
        }
    }
}
