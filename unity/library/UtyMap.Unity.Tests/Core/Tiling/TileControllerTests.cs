using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Core.Utils;
using Moq;
using NUnit.Framework;
using UnityEngine;
using UtyDepend.Config;
using UtyMap.Unity.Tests.Helpers;
using UtyRx;

namespace UtyMap.Unity.Tests.Core.Tiling
{
    [TestFixture]
    public class TileControllerTests
    {
        private const int LevelOfDetails = 15;
        private readonly GeoCoordinate _worldZeroPoint = TestHelper.WorldZeroPoint;

        private TileController _tileController;
        private Mock<IObserver<Tile>> _tileObserver;
        private Mock<IConfigSection> _configSection;
        
        [SetUp]
        public void Setup()
        {
            _configSection = new Mock<IConfigSection>();
            _configSection.Setup(c => c.GetFloat("sensitivity", It.IsAny<float>())).Returns(20);
            _configSection.Setup(c => c.GetFloat("offset", It.IsAny<float>())).Returns(100);

            _tileObserver = new Mock<IObserver<Tile>>();

            _tileController = new TileController();
            _tileController.Projection = new CartesianProjection(_worldZeroPoint);
            _tileController.Configure(_configSection.Object);
            _tileController.Subscribe(_tileObserver.Object);
        }

        [Test(Description = "Tests whether first tile can be loaded (done in test setup).")]
        public void CanLoadFirstTile()
        {
            QuadKey quadKey = GeoUtils.CreateQuadKey(_worldZeroPoint, LevelOfDetails);

            _tileController.OnPosition(_worldZeroPoint, LevelOfDetails);

            _tileObserver.Verify(o => o.OnNext(It.Is<Tile>(tile => CheckQuadKey(tile.QuadKey, quadKey))));
        }

        [Test(Description = "Tests whether next tile can be loaded when position is changed.")]
        public void CanLoadNextNorthTile()
        {
            var newQuadKey = new QuadKey(17602, 10743, LevelOfDetails);
            _tileController.OnPosition(_worldZeroPoint, LevelOfDetails);
            _tileObserver.ResetCalls();

            _tileController.OnPosition(MovePosition(_worldZeroPoint, new Vector2(0, 1), 400), LevelOfDetails);

            _tileObserver.Verify(o => o.OnNext(It.Is<Tile>(tile => CheckQuadKey(tile.QuadKey, newQuadKey))));
        }

        [Test (Description = "Tests whether far tile can be disposed.")]
        public void CanUnloadFarTile()
        {
            _configSection.Setup(c => c.GetInt("max_tile_distance", It.IsAny<int>())).Returns(1);
            QuadKey quadKey = GeoUtils.CreateQuadKey(_worldZeroPoint, LevelOfDetails);

            for (int i = 0; i < 2; ++i)
                _tileController.OnPosition(MovePosition(_worldZeroPoint, new Vector2(1, 0), i*400), LevelOfDetails);

            _tileObserver.Verify(o => o.OnNext(It.Is<Tile>(tile => tile.IsDisposed && CheckQuadKey(tile.QuadKey, quadKey))));
        }

        #region Helpers

        private GeoCoordinate MovePosition(GeoCoordinate currentPosition, Vector2 direction, float distance)
        {
            var point = GeoUtils.ToMapCoordinate(_worldZeroPoint, currentPosition);
            var newPoint = point + direction*distance;
            return GeoUtils.ToGeoCoordinate(_worldZeroPoint, newPoint);
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