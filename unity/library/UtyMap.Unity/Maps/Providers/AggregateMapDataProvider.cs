using System;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyRx;

namespace UtyMap.Unity.Maps.Providers
{
    internal class AggregateMapDataProvider : IMapDataProvider, IConfigurable
    {
        private readonly OpenStreetMapDataProvider _osmMapDataProvider;
        
        [Dependency]
        public AggregateMapDataProvider(IFileSystemService fileSystemService, INetworkService networkService, ITrace trace)
        {
            _osmMapDataProvider = new OpenStreetMapDataProvider(fileSystemService, networkService, trace);
        }

        /// <inheritdoc />
        public IObservable<string> Get(Tile tile)
        {
            return _osmMapDataProvider.Get(tile);
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _osmMapDataProvider.Configure(configSection);
        }
    }
}
