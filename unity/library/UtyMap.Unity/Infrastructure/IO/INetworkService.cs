using UtyRx;

namespace UtyMap.Unity.Infrastructure.IO
{
    public interface INetworkService
    {
        IObservable<string> Get(string url);
        IObservable<byte[]> GetAndGetBytes(string url);
    }
}
