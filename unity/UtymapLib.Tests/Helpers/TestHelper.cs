using Assets.UtymapLib.Core;
using Assets.UtymapLib.Infrastructure.Config;
using Assets.UtymapLib.Infrastructure.Diagnostic;
using Assets.UtymapLib.Infrastructure.IO;

namespace UtymapLib.Tests.Helpers
{
    internal static class TestHelper
    {
        #region Constants values

        public static GeoCoordinate WorldZeroPoint = new GeoCoordinate(52.5317429, 13.3871987);

        public const string TestAssetsFolder = @"../../../Assets/Resources";
        public const string ConfigTestRootFile = TestAssetsFolder + @"/Config/test.json";
        public const string BerlinXmlData = TestAssetsFolder + @"/Osm/berlin.osm.xml";
        public const string NmeaFilePath = TestAssetsFolder + @"/Nmea/invalidenstrasse_borsigstrasse.nme";
        public const string DefaultMapCss = TestAssetsFolder + @"/MapCss/default/default.mapcss";

        #endregion

        public static JsonConfigSection GetJsonConfig(string configPath)
        {
            return new JsonConfigSection
                (new FileSystemService(new PathResolver(), new DefaultTrace()).ReadText(configPath));
        }
    }
}
