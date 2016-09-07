using System;
using System.Net;
using UtyRx;

namespace UtyMap.Unity.Infrastructure.IO
{
    /// <summary> Provides default implementation of <see cref="INetworkService" />. </summary>
    internal class NetworkService : INetworkService
    {
        /// <inheritdoc />
        public IObservable<string> Get(string url)
        {
            var webClient = new WebClient();
            var query = Observable
                .FromEventPattern<DownloadStringCompletedEventHandler, DownloadStringCompletedEventArgs>
                (
                    eh => new DownloadStringCompletedEventHandler(eh),
                    eh => webClient.DownloadStringCompleted += eh,
                    eh => webClient.DownloadStringCompleted -= eh
                ).Select(o => o.EventArgs.Result);

            webClient.DownloadStringAsync(new Uri(url));
            return query;
        }

        /// <inheritdoc />
        public IObservable<byte[]> GetAndGetBytes(string url)
        {
            var webClient = new WebClient();
            var query = Observable.FromEventPattern<DownloadDataCompletedEventHandler, DownloadDataCompletedEventArgs>
                (
                    eh => new DownloadDataCompletedEventHandler(eh),
                    eh => webClient.DownloadDataCompleted += eh,
                    eh => webClient.DownloadDataCompleted -= eh
                ).Select(o => o.EventArgs.Result);

            webClient.DownloadDataAsync(new Uri(url));
            return query;
        }
    }
}