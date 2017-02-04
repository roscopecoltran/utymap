using System;
using System.Threading;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyDepend;
using UtyDepend.Config;
using UtyMap.Unity.Data;
using UtyRx;
using Return = UtyRx.Tuple<UtyMap.Unity.Tile, UtyMap.Unity.Infrastructure.Primitives.Union<UtyMap.Unity.Element, UtyMap.Unity.Mesh>>;

namespace UtyMap.Unity.Tests.Helpers
{
    internal static class TestHelper
    {
        #region Constants values

        public const string IntegrationTestCategory = "Integration";

        public static GeoCoordinate WorldZeroPoint = new GeoCoordinate(52.5317429, 13.3871987);

        public const string TestAssetsFolder = @"../../../../demo/Assets/StreamingAssets";
        
        public const string BerlinXmlData = TestAssetsFolder + @"/Osm/berlin.osm.xml";
        public const string BerlinPbfData = TestAssetsFolder + @"/Osm/berlin.osm.pbf";
        public const string DefaultMapCss = TestAssetsFolder + @"/MapCss/default/default.mapcss";

        #endregion

        public static CompositionRoot GetCompositionRoot(GeoCoordinate worldZeroPoint)
        {
            return GetCompositionRoot(worldZeroPoint, (container, section) => { });
        }

        public static CompositionRoot GetCompositionRoot(GeoCoordinate worldZeroPoint,
            Action<IContainer, IConfigSection> action)
        {
            // create default container which should not be exposed outside
            // to avoid Service Locator pattern.
            IContainer container = new Container();

            // create default application configuration
            var config = ConfigBuilder.GetDefault()
                .SetStringIndex("Index/")
                .SetSpatialIndex("Index/")
                .Build();

            // initialize services
            return new CompositionRoot(container, config)
                .RegisterAction((c, _) => c.Register(Component.For<ITrace>().Use<ConsoleTrace>()))
                .RegisterAction((c, _) => c.Register(Component.For<IPathResolver>().Use<TestPathResolver>()))
                .RegisterAction((c, _) => c.Register(Component.For<Stylesheet>().Use<Stylesheet>(DefaultMapCss)))
                .RegisterAction((c, _) => c.Register(Component.For<IProjection>().Use<CartesianProjection>(worldZeroPoint)))
                .RegisterAction(action)
                .Setup();
        }

        public static Return GetResultSync(this IMapDataStore store, Tile tile)
        {
            return GetResultSyncImpl(store, tile);
        }

        public static Tuple<Tile, string> GetResultSync(this ISubject<Tile, Tuple<Tile, string>> source, Tile tile)
        {
            return GetResultSyncImpl(source, tile);
        }

        private static T GetResultSyncImpl<T>(this ISubject<Tile, T> source, Tile tile)
        {
            var result = default(T);
            var manualResetEvent = new ManualResetEvent(false);
            source
                .SubscribeOn(Scheduler.CurrentThread)
                .ObserveOn(Scheduler.CurrentThread)
                .Subscribe(r =>
                {
                    result = r;
                    manualResetEvent.Set();
                });

            source.OnNext(tile);

            manualResetEvent.WaitOne(TimeSpan.FromSeconds(5));
            return result;
        }
    }
}
