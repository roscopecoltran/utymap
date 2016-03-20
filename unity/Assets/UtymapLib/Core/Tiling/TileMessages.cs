﻿namespace Assets.UtymapLib.Core.Tiling
{
    #region Loading

    /// <summary> Defines "Tile load start" message. </summary>
    public sealed class TileLoadStartMessage
    {
        /// <summary> QuadKey of tile being loaded. </summary>
        public Tile Tile { get; private set; }

        /// <summary> Creates message. </summary>
        public TileLoadStartMessage(Tile tile)
        {
            Tile = tile;
        }
    }

    /// <summary> Defines "Tile load finish" message. </summary>
    public sealed class TileLoadFinishMessage
    {
        /// <summary> Creates message. </summary>
        /// <param name="tile">Tile.</param>
        public TileLoadFinishMessage(Tile tile)
        {
            Tile = tile;
        }

        /// <summary> Gets tile. </summary>
        public Tile Tile { get; private set; }
    }

    /// <summary> Defines "Tile destroy" message. </summary>
    public sealed class TileDestroyMessage
    {
        /// <summary> Creates message. </summary>
        /// <param name="tile">Tile.</param>
        public TileDestroyMessage(Tile tile)
        {
            Tile = tile;
        }

        /// <summary> Gets tile. </summary>
        public Tile Tile { get; private set; }
    }

    #endregion
}
