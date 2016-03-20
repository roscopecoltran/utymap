using System.Collections.Generic;

namespace Assets.UtymapLib.Core.Models
{
    /// <summary> OSM specific data reprsentation. </summary>
    public class Element
    {
        public readonly long Id;
        public GeoCoordinate[] Geometry;
        public readonly Dictionary<string, string> Tags;
        public readonly Dictionary<string, string> Styles;

        public Element(long id, GeoCoordinate[] geometry, Dictionary<string, string> tags, Dictionary<string, string> styles)
        {
            Id = id;
            Geometry = geometry;
            Tags = tags;
            Styles = styles;
        }
    }
}
