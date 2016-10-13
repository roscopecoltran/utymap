using UtyMap.Unity.Core.Tiling;
using UtyRx;

namespace UtyMap.Unity.Maps.Providers
{
    /// <summary> Provides file path to map data. </summary>
    public interface IMapDataProvider
    {
        /// <summary> Called when new tile is loading. </summary>
        IObservable<string> Get(Tile tile);
    }
}
