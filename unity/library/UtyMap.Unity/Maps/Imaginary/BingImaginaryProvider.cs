using System;
using UtyMap.Unity.Core;

namespace UtyMap.Unity.Maps.Imaginary
{
    /// <summary> Fetches map imaginarydata from Bing. </summary>
    public class BingImaginaryProvider : ImaginaryProvider
    {
        private const string UrlSchema = "http://ecn.t0.tiles.virtualearth.net/tiles/a{0}.jpeg?g=5166&shading=hill";

        /// <inheritdoc />
        protected override string GetUrl(QuadKey quadkey)
        {
            return String.Format(UrlSchema, quadkey);
        }

        /// <inheritdoc />
        protected override string FormatExtension
        {
            get { return ".jpg"; }
        }
    }
}