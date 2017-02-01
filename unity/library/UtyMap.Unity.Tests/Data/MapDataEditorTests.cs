using System;
using System.Collections.Generic;
using System.Linq;
using NUnit.Framework;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyMap.Unity.Tests.Helpers;
using UtyRx;

using Return = UtyRx.Tuple<UtyMap.Unity.Tile, UtyMap.Unity.Infrastructure.Primitives.Union<UtyMap.Unity.Element, UtyMap.Unity.Mesh>>;

namespace UtyMap.Unity.Tests.Data
{
    [TestFixture(Category = TestHelper.IntegrationTestCategory)]
    public class MapDataEditorTests
    {
        private CompositionRoot _compositionRoot;
        private IMapDataEditor _mapDataEditor;
        private IMapDataStore _dataStore;
        private Stylesheet _stylesheet;
        private IProjection _projection;

        [TestFixtureSetUp]
        public void Setup()
        {
            _compositionRoot = TestHelper.GetCompositionRoot(TestHelper.WorldZeroPoint);
            _mapDataEditor = _compositionRoot.GetService<IMapDataEditor>();
            _dataStore = _compositionRoot.GetService<IMapDataStore>();
            _stylesheet = _compositionRoot.GetService<Stylesheet>();
            _projection = _compositionRoot.GetService<IProjection>();
        }

        [TestFixtureTearDown]
        public void TearDown()
        {
            _compositionRoot.Dispose();
        }

        [Test]
        public void CanAddNode()
        {
            // ARRANGE
            var levelOfDetails = new Range<int>(1, 1);
            var node = new Element(7,
                new GeoCoordinate[] { new GeoCoordinate(5, 5) },
                new double[] { 0 }, 
                new Dictionary<string, string>() { { "featurecla", "Populated place" } }, 
                new Dictionary<string, string>());
            var result = default(Return);

            // ACT
            _mapDataEditor.Add(MapDataStorageType.InMemory, node, levelOfDetails);

            // ASSERT
            _dataStore
                .SubscribeOn(Scheduler.CurrentThread)
                .Subscribe(u => result = u);
            _dataStore.OnNext(new Tile(new QuadKey(1, 0, 1), _stylesheet, _projection, ElevationDataType.Flat));
            result.Item2.Match(
                e => CompareElements(node, e),
                mesh => { throw new ArgumentException(); });
        }

        private void CompareElements(Element expected, Element actual)
        {
            Assert.AreEqual(expected.Id, actual.Id);
            // geometry
            Assert.AreEqual(expected.Geometry.Length, actual.Geometry.Length);
            for(int i = 0; i < expected.Geometry.Length; ++i)
                Assert.AreEqual(expected.Geometry[i], actual.Geometry[i]);
            // tags
            Assert.AreEqual(expected.Tags.Count, actual.Tags.Count);
            var keys = expected.Tags.Keys.ToArray();
            for (int i = 0; i < keys.Length; ++i)
                Assert.AreEqual(expected.Tags[keys[i]], actual.Tags[keys[i]]); 
        }
    }
}
