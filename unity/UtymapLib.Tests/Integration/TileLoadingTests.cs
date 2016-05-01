using System;
using Assets.UtymapLib;
using Assets.UtymapLib.Core;
using Assets.UtymapLib.Core.Models;
using Assets.UtymapLib.Core.Tiling;
using Assets.UtymapLib.Infrastructure.Config;
using Assets.UtymapLib.Infrastructure.Dependencies;
using Assets.UtymapLib.Infrastructure.Diagnostic;
using Assets.UtymapLib.Infrastructure.IO;
using Assets.UtymapLib.Infrastructure.Primitives;
using Assets.UtymapLib.Infrastructure.Reactive;
using Assets.UtymapLib.Maps.Loader;
using NUnit.Framework;
using UtymapLib.Tests.Helpers;

namespace UtymapLib.Tests.Integration
{
    [TestFixture(Category = TestHelper.IntegrationTestCategory)]
    public class TileLoadingTests
    {
        private const int LevelOfDetails = 16;

        private readonly GeoCoordinate _worldZeroPoint = TestHelper.WorldZeroPoint;
        private CompositionRoot _compositionRoot;
        private IMapDataLoader _mapDataLoader;
        private ITileController _tileController;

        [TestFixtureSetUp]
        public void Setup()
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
            _mapDataLoader = container.Resolve<IMapDataLoader>();
            _tileController = container.Resolve<ITileController>();
        }

        [TestFixtureTearDown]
        public void TearDown()
        {
            _compositionRoot.Dispose();
        }

        [Test(Description = "This test loads 100 tiles from osm file to ensure that there are no unexpected crashes at least at test region.")]
        public void CanLoadMultipleTiles()
        {
            // ARRANGE
            SetupMapData(TestHelper.BerlinPbfData);
            var count = 10;
            var centerQuadKey = new QuadKey(35205, 21489, LevelOfDetails);

            // ACT & ASSERT
            for (var y = 0; y < count; ++y)
                for (var x = 0; x < count; ++x)
                {
                    var quadKey = new QuadKey(centerQuadKey.TileX + x, centerQuadKey.TileY + y, LevelOfDetails);
                    var tile = new Tile(quadKey, _tileController.Stylesheet, _tileController.Projection);
                    _mapDataLoader
                        .Load(tile)
                        .SubscribeOn(Scheduler.ThreadPool)
                        .ObserveOn(Scheduler.ThreadPool)
                        .Do(AssertData)
                        .Wait(TimeSpan.FromSeconds(5));
                }
        }

        #region Private members

        /// <summary> Setup test map data. </summary>
        private void SetupMapData(string mapDataPath)
        {
            var range = new Range<int>(LevelOfDetails, LevelOfDetails);
            _compositionRoot
                .GetService<IMapDataLoader>()
                .AddToInMemoryStore(mapDataPath, _tileController.Stylesheet, range);
        }

        /// <summary> Checks whether data satisfied minimal correctness critirea. </summary>
        private void AssertData(Union<Element, Mesh> data)
        {
            var elements = 0;
            var meshes = 0;
            data.Match(
                element =>
                {
                    Assert.Greater(element.Geometry.Length, 0);
                    ++elements;
                }, mesh =>
                {
                    Assert.Greater(mesh.Vertices.Length, 0);
                    Assert.Greater(mesh.Triangles.Length, 0);
                    Assert.Greater(mesh.Colors.Length, 0);
                    ++meshes;
                });

            Assert.Greater(elements + meshes, 0);
        }

        #endregion
    }
}
