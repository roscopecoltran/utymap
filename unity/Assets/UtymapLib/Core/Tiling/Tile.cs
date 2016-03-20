using UnityEngine;
using Assets.UtymapLib.Core.Utils;
using Assets.UtymapLib.Infrastructure.Primitives;

namespace Assets.UtymapLib.Core.Tiling
{
    /// <summary> Represents map tile. </summary>
    public class Tile
    {
        /// <summary> Tile rectamgle in world coordinates. </summary>
        public Rectangle Rectangle { get; private set; }

        /// <summary> Tile geo bounding box. </summary>
        public BoundingBox BoundingBox { get; private set; }

        /// <summary> Corresponding quadkey. </summary>
        public QuadKey QuadKey { get; private set; }

        /// <summary> Stylesheet. </summary>
        public Stylesheet Stylesheet { get; private set; }

        /// <summary> Used projection. </summary>
        public IProjection Projection { get; private set; }

        /// <summary> Sets game object which holds all children objects. </summary>
        public GameObject GameObject { get; set; }

        /// <summary> Creates <see cref="Tile"/>. </summary>
        /// <param name="quadKey"></param>
        /// <param name="stylesheet"></param>
        /// <param name="projection"> Projection. </param>
        internal Tile(QuadKey quadKey, Stylesheet stylesheet, IProjection projection)
        {
            QuadKey = quadKey;
            Stylesheet = stylesheet;
            Projection = projection;

            BoundingBox = GeoUtils.QuadKeyToBoundingBox(quadKey);
            Rectangle = GeoUtils.QuadKeyToRect(projection, quadKey);
        }

        /// <summary> Checks whether absolute position locates in tile with bound offset. </summary>
        /// <param name="position">Absolute position in game.</param>
        /// <param name="offset">offset from bounds.</param>
        /// <returns>Tres if position in tile</returns>
        public bool Contains(Vector2 position, double offset)
        {
            return (position.x > Rectangle.Left + offset) && (position.y < Rectangle.Top - offset) &&
                   (position.x < Rectangle.Right - offset) && (position.y > Rectangle.Bottom + offset);
        }
    }
}
