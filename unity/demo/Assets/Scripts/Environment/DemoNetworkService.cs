using Assets.Scripts.Reactive;
using UtyMap.Unity.Infrastructure.IO;
using UtyRx;

namespace Assets.Scripts.Environment
{
    internal class DemoNetworkService : INetworkService
    {
        /// <inheritdoc />
        public IObservable<string> Get(string url)
        {
            return ObservableWWW.Get(url);
        }

        /// <inheritdoc />
        public IObservable<byte[]> GetAndGetBytes(string url)
        {
            return ObservableWWW.GetAndGetBytes(url);
        }
    }
}