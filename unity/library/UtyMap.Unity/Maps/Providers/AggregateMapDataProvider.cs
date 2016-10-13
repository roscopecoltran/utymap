using System;
using UtyDepend;
using UtyMap.Unity.Core.Tiling;
using UtyRx;

namespace UtyMap.Unity.Maps.Providers
{
    internal class AggregateMapDataProvider : IMapDataProvider
    {
        private readonly OpenStreetMapDataProvider _osmMapDataProvider;
        
        [Dependency]
        public AggregateMapDataProvider(OpenStreetMapDataProvider osmMapDataProvider)
        {
            _osmMapDataProvider = osmMapDataProvider;
        }

        /// <inheritdoc />
        public IObservable<string> Get(Tile tile)
        {
            return _osmMapDataProvider.Get(tile);
        }
    }
}
