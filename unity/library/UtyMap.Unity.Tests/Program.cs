using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Maps.Data;
using UtyMap.Unity.Tests.Helpers;
using UtyRx;

namespace UtyMap.Unity.Tests
{
    /// <summary> This class is useful to debug/profile application independenly from Unity Editor. </summary>
    internal class Program
    {
        private const int LevelOfDetails = 16;

        private static void Main(string[] args)
        {
            var compositionRoot = TestHelper.GetCompositionRoot(TestHelper.WorldZeroPoint);

            var tileController = compositionRoot.GetService<ITileController>();
            var mapDataLoader = compositionRoot.GetService<IMapDataLoader>();

            mapDataLoader.AddToStore(MapStorageType.InMemory,
                TestHelper.BerlinXmlData,
                compositionRoot.GetService<Stylesheet>(),
                new Range<int>(LevelOfDetails, LevelOfDetails));

            tileController
                .Where(tile => !tile.IsDisposed)
                .SelectMany(tile => mapDataLoader.Load(tile))
                .SubscribeOn(Scheduler.ThreadPool)
                .ObserveOn(Scheduler.MainThread)
                .Subscribe(u => { });

            tileController.OnPosition(TestHelper.WorldZeroPoint, LevelOfDetails);
        }
    }
}