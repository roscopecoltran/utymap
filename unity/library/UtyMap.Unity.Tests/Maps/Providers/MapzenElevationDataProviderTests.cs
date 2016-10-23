using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using Moq;
using NUnit.Framework;
using UtyDepend.Config;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Core.Utils;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Maps.Providers;
using UtyMap.Unity.Tests.Helpers;
using UtyRx;

namespace UtyMap.Unity.Tests.Maps.Providers
{
    [TestFixture]
    public class MapzenElevationDataProviderTests
    {
        private Mock<IFileSystemService> _fileSystemService;
        private Mock<INetworkService> _networkService;
        private Mock<ITrace> _trace;
        private Mock<IConfigSection> _config;
        private Mock<Stream> _writeStream;
        
        private Tile _tile;
        private byte[] _responseBytes;

        private MapzenElevationDataProvider _eleProvider;

        [TestFixtureSetUp]
        public void SetUp()
        {
            //{"encoded_polyline":"kzcecBqdapX?sjDmgBrjD?sjD","height":[43,38,37,37]}
            _fileSystemService = new Mock<IFileSystemService>();
            _networkService = new Mock<INetworkService>();
            _trace = new Mock<ITrace>();

            _config = new Mock<IConfigSection>();
            _config.Setup(c => c.GetString("data/mapzen/ele_server", It.IsAny<string>())).Returns("elevation.mapzen.com/height?json={0}&api_key={1}");
            _config.Setup(c => c.GetString("data/mapzen/api_key", It.IsAny<string>())).Returns("ggg");
            _config.Setup(c => c.GetString("data/mapzen/ele_format", It.IsAny<string>())).Returns("ele");
            _config.Setup(c => c.GetInt("data/mapzen/ele_grid", It.IsAny<int>())).Returns(2);
            _config.Setup(c => c.GetString("data/cache", It.IsAny<string>())).Returns("Cache");

            _writeStream = new Mock<Stream>();
            _responseBytes = Encoding.UTF8.GetBytes("{\"encoded_polyline\":\"kzcecBqdapX?sjDmgBrjD?sjD\",\"height\":[43,38,37,37]}");

            _tile = new Tile(GeoUtils.CreateQuadKey(TestHelper.WorldZeroPoint, 16), new Stylesheet(""), 
                new CartesianProjection(TestHelper.WorldZeroPoint));

            _networkService
                .Setup(ns => ns.GetAndGetBytes(It.IsAny<string>(), It.IsAny<Dictionary<string,string>>()))
                .Returns(Observable.Return(_responseBytes));

            _fileSystemService
                .Setup(fs => fs.WriteStream(It.IsAny<string>()))
                .Returns(_writeStream.Object);
            
            _eleProvider = new MapzenElevationDataProvider(_fileSystemService.Object, _networkService.Object, _trace.Object);

            _eleProvider.Configure(_config.Object);
        }

        [Test]
        public void CanRequestPolyline()
        {
            _eleProvider.Get(_tile).Wait(TimeSpan.FromSeconds(5));

            _networkService.Verify(ns => ns.GetAndGetBytes("elevation.mapzen.com/height?json={\"range\":false,\"encoded_polyline\":\"kzcecBqdapX?sjDmgBrjD?sjD\"}&api_key=ggg", It.IsAny<Dictionary<string, string>>()));
        }

        [Test]
        public void CanCreateDataFile()
        {
            _eleProvider.Get(_tile).Wait(TimeSpan.FromSeconds(5));

            _fileSystemService.Verify(fs => fs.WriteStream(Path.Combine("Cache", "1202102332220103.ele")));
        }

        [Test]
        public void CanWriteCorrectEleData()
        {
            _eleProvider.Get(_tile).Wait(TimeSpan.FromSeconds(5));

            _writeStream.Verify(ws => ws.Write(_responseBytes, 0, _responseBytes.Length));
        }
    }
}
