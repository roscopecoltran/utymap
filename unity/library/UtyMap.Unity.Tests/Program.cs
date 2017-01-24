using System;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Tests.Helpers;
using UtyMap.Unity.Utils;
using UtyRx;

namespace UtyMap.Unity.Tests
{
    /// <summary> This class is useful to debug/profile application independenly from Unity Editor. </summary>
    internal class Program
    {
        private const int LevelOfDetails = 16;

        private static void Main(string[] args)
        {
            // select main thread scheduler
            Scheduler.MainThread = Scheduler.CurrentThread;

            // get store and select theme represented by stylesheet
            var store = TestHelper.GetCompositionRoot(TestHelper.WorldZeroPoint)
                                  .GetService<IMapDataStore>();
            var stylesheet = new Stylesheet(TestHelper.DefaultMapCss);

            // 1. add data to store
            store.Add(MapDataStorageType.InMemory,
                      TestHelper.BerlinXmlData,
                      stylesheet,
                      new Range<int>(LevelOfDetails, LevelOfDetails));
            
            // 2. start listening for processed data
            store
                .SubscribeOn(Scheduler.ThreadPool)
                .ObserveOn(Scheduler.MainThread)
                // convert element or mesh into your specific representation
                .Subscribe(u => u.Match(e => Console.WriteLine("Element: {0}", e.Id),
                                        m => Console.WriteLine("Mesh: {0}", m.Name)),
                           ex => Console.WriteLine("Error: {0}", ex));

            // 3. start loading of specific region
            store.OnNext(new Tile(
                GeoUtils.CreateQuadKey(TestHelper.WorldZeroPoint, LevelOfDetails),
                stylesheet,
                new CartesianProjection(TestHelper.WorldZeroPoint)));

            Console.ReadLine();
        }
    }
}