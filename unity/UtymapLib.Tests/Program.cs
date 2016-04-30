using System;
using System.IO;
using System.Threading;
using Assets.UtymapLib;
using Assets.UtymapLib.Core;
using Assets.UtymapLib.Core.Positioning;
using Assets.UtymapLib.Core.Positioning.Nmea;
using Assets.UtymapLib.Core.Tiling;
using Assets.UtymapLib.Infrastructure;
using Assets.UtymapLib.Infrastructure.Config;
using Assets.UtymapLib.Infrastructure.Dependencies;
using Assets.UtymapLib.Infrastructure.Diagnostic;
using Assets.UtymapLib.Infrastructure.IO;
using Assets.UtymapLib.Infrastructure.Primitives;
using Assets.UtymapLib.Infrastructure.Reactive;
using Assets.UtymapLib.Maps.Loader;
using UtymapLib.Tests.Helpers;

namespace UtymapLib.Tests
{
    /// <summary> This class is useful to debug/profile application independenly from Unity Editor. </summary>
    internal class Program
    {
        private const int LevelOfDetails = 16;
        private const string TraceCategory = "test.app";

        private readonly GeoCoordinate _worldZeroPoint = TestHelper.WorldZeroPoint;
        private readonly ManualResetEvent _waitEvent = new ManualResetEvent(false);

        private CompositionRoot _compositionRoot;
        private IMessageBus _messageBus;
        private ITileController _tileController;

        private ITrace _trace;

        private static void Main(string[] args)
        {
            Scheduler.MainThread = Scheduler.CurrentThread;

            var program = new Program();
            program.Initialize();
            program.RunMocker();
        }

        private void Initialize()
        {
            // create default container which should not be exposed outside
            // to avoid Service Locator pattern. 
            IContainer container = new Container();

            // create default application configuration
            var config = ConfigBuilder.GetDefault()
               .Build();

            // initialize services
            _compositionRoot = new CompositionRoot(container, config)
                .RegisterAction((c, _) => c.Register(Component.For<ITrace>().Use<ConsoleTrace>()))
                .RegisterAction((c, _) => c.Register(Component.For<IPathResolver>().Use<TestPathResolver>()))
                .RegisterAction((c, _) => c.Register(Component.For<Stylesheet>().Use<Stylesheet>(TestHelper.DefaultMapCss)))
                .RegisterAction((c, _) => c.Register(Component.For<IProjection>().Use<CartesianProjection>(_worldZeroPoint)))
                .Setup();

            // get local references
            _messageBus = container.Resolve<IMessageBus>();
            _trace = container.Resolve<ITrace>();
            _tileController = container.Resolve<ITileController>();

            SetupMapData();

            // set observer to react on geo position changes
            _messageBus.AsObservable<GeoPosition>()
                .ObserveOn(Scheduler.MainThread)
                .Subscribe(position =>
            {
                _trace.Debug(TraceCategory, "GeoPosition: {0}", position.ToString());
                _tileController.OnPosition(position.Coordinate, LevelOfDetails);
            });
        }

        /// <summary> Reads nmea file and sends fake geo positions via message bus. </summary>
        private void RunMocker()
        {
            Action<TimeSpan> delayAction = Thread.Sleep;
            using (Stream stream = new FileStream(TestHelper.NmeaFilePath, FileMode.Open))
            {
                var mocker = new NmeaPositionMocker(stream, _messageBus);
                mocker.OnDone += (s, e) => _waitEvent.Set();
                mocker.Start(delayAction);
            }
        }

        /// <summary> Setup test map data. </summary>
        private void SetupMapData()
        {
            var mapDataLoader = _compositionRoot.GetService<IMapDataLoader>();
            var stylesheet = _compositionRoot.GetService<Stylesheet>();

            mapDataLoader.AddToInMemoryStore(TestHelper.BerlinXmlData, stylesheet,
                new Range<int>(LevelOfDetails, LevelOfDetails));
        }
    }
}