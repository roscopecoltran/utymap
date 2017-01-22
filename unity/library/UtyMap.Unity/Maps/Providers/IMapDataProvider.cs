using System;
using System.Collections.Generic;
using System.IO;
using UtyMap.Unity.Core;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyRx;

namespace UtyMap.Unity.Maps.Providers
{
    /// <summary> Provides file path to map data. </summary>
    public interface IMapDataProvider : ISubject<QuadKey, string>
    {
    }

    internal abstract class MapDataProvider : IMapDataProvider
    {
        private readonly List<IObserver<string>> _observers = new List<IObserver<string>>();

        public abstract void OnNext(QuadKey value);

        public virtual void OnCompleted()
        {
            _observers.ForEach(o => o.OnCompleted());
        }

        public virtual void OnError(Exception error)
        {
            _observers.ForEach(o => o.OnError(error));
        }

        public virtual IDisposable Subscribe(IObserver<string> observer)
        {
            // TODO handle unsubscribe
            _observers.Add(observer);
            return Disposable.Empty;
        }

        /// <summary> Notifies observers. </summary>
        protected void Notify(string filePath)
        {
            _observers.ForEach(o => o.OnNext(filePath));
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

        public override void OnNext(QuadKey value)
        {
            var filePath = GetFilePath(value);
            var uri = GetUri(value);

            if (!_fileSystemService.Exists(filePath))
            {
                _trace.Warn(TraceCategory, Strings.NoPresistentElementSourceFound, value.ToString(), uri);
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
                    });
            }

            Notify(filePath);
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
