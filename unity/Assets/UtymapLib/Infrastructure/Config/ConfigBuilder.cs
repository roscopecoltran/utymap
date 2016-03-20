namespace Assets.UtymapLib.Infrastructure.Config
{
    /// <summary> Represents a builder responsible for application configuration creation. </summary>
    public class ConfigBuilder
    {
        private readonly CodeConfigSection _configSection = new CodeConfigSection();

        /// <summary> Adds value to configuration with path provided. </summary>
        /// <remarks> Use this method to extend application with your custom settings. </remarks>
        /// <typeparam name="T">Type of value.</typeparam>
        /// <param name="path">Path.</param>
        /// <param name="value">Value.</param>
        protected void Add<T>(string path, T value)
        {
            _configSection.Add(path, value);
        }

        /// <summary> Builds application specific configuration. </summary>
        public IConfigSection Build()
        {
            return _configSection;
        }

        #region Application specific

        /// <summary> Sets settings to get elevation data from remote server. </summary>
        public ConfigBuilder SetRemoteElevationData(string url, string schema)
        {
            Add<string>("data/elevation/remote.server", url);
            Add<string>("data/elevation/remote.schema", schema);
            return this;
        }

        /// <summary> Sets geocoding server's url. </summary>
        public ConfigBuilder SetGeocodingServer(string url)
        {
            Add<string>("geocoding", url);
            return this;
        }

        /// <summary> Sets local path to elevation data. </summary>
        public ConfigBuilder SetLocalElevationData(string path)
        {
            Add<string>("data/elevation/local", path);
            return this;
        }

        /// <summary> Sets string index. </summary>
        public ConfigBuilder SetStringIndex(string path)
        {
            Add<string>("data/index/strings", path);
            return this;
        }

        /// <summary> Sets string index. </summary>
        public ConfigBuilder SetSpatialIndex(string path)
        {
            Add<string>("data/index/spatial", path);
            return this;
        }

        #endregion

        #region Default instance

        /// <summary> Gets ConfigBuilder with default settings. </summary>
        /// <remarks> You can call methods to override settings with custom ones. </remarks>
        public static ConfigBuilder GetDefault()
        {
            return new ConfigBuilder()
                .SetLocalElevationData("Elevation")
                .SetRemoteElevationData("http://dds.cr.usgs.gov/srtm/version2_1/SRTM3", "Config/srtm.schema.txt")
                .SetGeocodingServer("http://nominatim.openstreetmap.org/search");
        }

        #endregion
    }
}
