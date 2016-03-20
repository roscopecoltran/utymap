using Assets.UtymapLib.Core;
using Assets.UtymapLib.Core.Models;
using Assets.UtymapLib.Core.Tiling;
using Assets.UtymapLib.Core.Utils;
using Assets.UtymapLib.Infrastructure;
using Assets.UtymapLib.Infrastructure.Config;
using Assets.UtymapLib.Infrastructure.Primitives;
using Assets.UtymapLib.Infrastructure.Reactive;
using Assets.UtymapLib.Maps.Loader;
using Moq;
using NUnit.Framework;
using UnityEngine;
using UtymapLib.Tests.Helpers;
using Mesh = Assets.UtymapLib.Core.Models.Mesh;

namespace UtymapLib.Tests.Core.Tiling
{
    [TestFixture]
    public class TileControllerTests
    {
        private const int LevelOfDetails = 15;
        private readonly GeoCoordinate _worldZeroPoint = TestHelper.WorldZeroPoint;

        private TileController _tileController;
        private Mock<IMapDataLoader> _tileLoader;
        private Mock<IMessageBus> _messageBus;
        private Mock<IConfigSection> _configSection;
        private Mock<IObservable<Union<Element, Mesh>>> _loaderResult;

        private float _tileSize;
        
        [SetUp]
        public void Setup()
        {
            _tileSize = GetTileRect(_worldZeroPoint).width; // ~743.9

            _tileLoader = new Mock<IMapDataLoader>();
            _messageBus = new Mock<IMessageBus>();
            _configSection = new Mock<IConfigSection>();
            _loaderResult = new Mock<IObservable<Union<Element, Mesh>>>();

            _configSection.Setup(c => c.GetFloat("sensitivity", It.IsAny<float>())).Returns(20);
            _configSection.Setup(c => c.GetFloat("offset", It.IsAny<float>())).Returns(100);
            _tileLoader.Setup(t => t.Load(It.IsAny<Tile>())).Returns(_loaderResult.Object);

            _tileController = new TileController(new ModelBuilder(), _tileLoader.Object, _messageBus.Object);
            _tileController.Projection = new CartesianProjection(_worldZeroPoint);
            _tileController.Configure(_configSection.Object);
        }

        [Test(Description = "Tests whether first tile can be loaded (done in test setup).")]
        public void CanLoadFirstTile()
        {
            QuadKey quadKey = GeoUtils.CreateQuadKey(_worldZeroPoint, LevelOfDetails);

            _tileController.OnPosition(_worldZeroPoint, LevelOfDetails);

            _tileLoader.Verify(t => t.Load(It.Is<Tile>(tile => CheckQuadKey(tile.QuadKey, quadKey))));
            _messageBus.Verify(mb => mb.Send(It.Is<TileLoadStartMessage>(m => CheckQuadKey(m.Tile.QuadKey, quadKey))));
        }

        [Test(Description = "Tests whether next tile can be loaded when position is changed.")]
        public void CanLoadNextNorthTile()
        {
            var newQuadKey = new QuadKey(17602, 10743, LevelOfDetails);
            _tileController.OnPosition(_worldZeroPoint, LevelOfDetails);
            _tileLoader.ResetCalls();
            _messageBus.ResetCalls();

            _tileController.OnPosition(MovePosition(_worldZeroPoint, new Vector2(0, 1), 400), LevelOfDetails);

            _tileLoader.Verify(t => t.Load(It.Is<Tile>(tile => CheckQuadKey(tile.QuadKey, newQuadKey))));
            _messageBus.Verify(mb => mb.Send(It.Is<TileLoadStartMessage>(m => CheckQuadKey(m.Tile.QuadKey, newQuadKey))));
        }

        #region Helpers

        private GeoCoordinate MovePosition(GeoCoordinate currentPosition, Vector2 direction, float distance)
        {
            var point = GeoUtils.ToMapCoordinate(_worldZeroPoint, currentPosition);
            var newPoint = point + direction*distance;
            return GeoUtils.ToGeoCoordinate(_worldZeroPoint, newPoint);
        }

        private Rect GetTileRect(GeoCoordinate coordinate)
        {
            var quadKey = GeoUtils.CreateQuadKey(coordinate, LevelOfDetails);
            var boundingBox = GeoUtils.QuadKeyToBoundingBox(quadKey);
            var minPoint = GeoUtils.ToMapCoordinate(_worldZeroPoint, boundingBox.MinPoint);
            var maxPoint = GeoUtils.ToMapCoordinate(_worldZeroPoint, boundingBox.MaxPoint);

            return new Rect(minPoint.x, maxPoint.y, maxPoint.x - minPoint.x, maxPoint.y - minPoint.y);
        }

        private bool CheckQuadKey(QuadKey actual, QuadKey expected)
        {
            Assert.AreEqual(expected.LevelOfDetail, actual.LevelOfDetail);
            Assert.AreEqual(expected.TileX, actual.TileX);
            Assert.AreEqual(expected.TileY, actual.TileY);
            return true;
        }

        #endregion
    }
}