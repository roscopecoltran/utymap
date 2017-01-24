using System.Threading;
using NUnit.Framework;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Tests.Helpers;
using UtyMap.Unity.Utils;
using UtyRx;

namespace UtyMap.Unity.Tests.Data
{
    [TestFixture(Category = TestHelper.IntegrationTestCategory)]
    public class TileLoadingTests
    {
        private readonly GeoCoordinate _worldZeroPoint = TestHelper.WorldZeroPoint;
        private CompositionRoot _compositionRoot;
        private IMapDataStore _mapDataStore;
        private Stylesheet _stylesheet;
        private IProjection _projection;
        private bool _isCalled;

        [TestFixtureSetUp]
        public void Setup()
        {
            // initialize services
            _compositionRoot = TestHelper.GetCompositionRoot(_worldZeroPoint);
            _stylesheet = _compositionRoot.GetService<Stylesheet>();
            _projection = _compositionRoot.GetService<IProjection>();
            // get local references
            _mapDataStore = _compositionRoot.GetService<IMapDataStore>();
            _isCalled = false;
        }

        [TestFixtureTearDown]
        public void TearDown()
        {
            _compositionRoot.Dispose();
        }

        [Test(Description = "This test loads 64 tiles from osm file to ensure that there are no unexpected crashes at least at test region.")]
        public void CanLoadMultipleTilesAtDetailedLevelOfDetails()
        {
            // ARRANGE
            int lod = 16;
            SetupMapData(TestHelper.BerlinPbfData, lod);
            var count = 10;
            var centerQuadKey = new QuadKey(35205, 21489, lod);

            // ACT & ASSERT
            for (var y = 0; y < count; ++y)
                for (var x = 0; x < count; ++x)
                    LoadQuadKeySync(new QuadKey(centerQuadKey.TileX + x, centerQuadKey.TileY + y, lod));
            Assert.IsTrue(_isCalled);
        }

        [Test(Description = "This test loads 4 tiles at zoom level 1.")]
        public void CanLoadGlobeAtLowestLevelOfDetails()
        {
            // ARRANGE
            int lod = 1;
            SetupMapData(TestHelper.NaturalEarth110mAdmin, lod);
            SetupMapData(TestHelper.NaturalEarth110mLakes, lod);
            SetupMapData(TestHelper.NaturalEarth110mLand, lod);
            SetupMapData(TestHelper.NaturalEarth110mRivers, lod);
            var count = 2;

            // ACT & ASSERT
            for (var y = 0; y < count; ++y)
                for (var x = 0; x < count; ++x)
                    LoadQuadKeySync(new QuadKey(x, y, lod));

            Assert.IsTrue(_isCalled);
        }

        [Test(Description = "This test loads 4 tiles at zoom level 14.")]
        public void CanLoadAtBirdEyeLevelOfDetails()
        {
            // ARRANGE
            int lod = 14;
            SetupMapData(TestHelper.BerlinXmlData, lod);
            var count = 1;
            var centerQuadKey = GeoUtils.CreateQuadKey(TestHelper.WorldZeroPoint, lod);

            // ACT & ASSERT
            for (var y = 0; y < count; ++y)
                for (var x = 0; x < count; ++x)
                    LoadQuadKeySync(new QuadKey(centerQuadKey.TileX + x, centerQuadKey.TileY + y, lod));
            
            Assert.IsTrue(_isCalled);
        }

        #region Private members

        /// <summary> Setup test map data. </summary>
        private void SetupMapData(string mapDataPath, int lod)
        {
            var range = new Range<int>(lod, lod);
            _compositionRoot
                .GetService<IMapDataStore>()
                .Add(MapDataStorageType.InMemory, mapDataPath, _stylesheet, range);
        }

        /// <summary> Loads quadkey waiting for completion callback. </summary>
        private void LoadQuadKeySync(QuadKey quadKey)
        {
            var manualResetEvent = new ManualResetEvent(false);
            _mapDataStore
                .SubscribeOn(Scheduler.CurrentThread)
                .ObserveOn(Scheduler.CurrentThread)
                .Subscribe(AssertData, () => manualResetEvent.Set());

            _mapDataStore.OnNext(new Tile(quadKey, _stylesheet, _projection));

            manualResetEvent.WaitOne();
        }

        /// <summary> Checks whether data satisfied minimal correctness critirea. </summary>
        private void AssertData(Tuple<Tile, Union<Element, Mesh>> data)
        {
            _isCalled = true;
            var elements = 0;
            var meshes = 0;
            data.Item2.Match(
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
