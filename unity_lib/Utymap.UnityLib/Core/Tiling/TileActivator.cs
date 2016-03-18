namespace Utymap.UnityLib.Core.Tiling
{
    /// <summary> Defines logic for tile activation, deactivation and destroy. </summary>
    public interface ITileActivator
    {
        /// <summary> Called before load is called. </summary>
        void PreLoad(Tile tile);

        /// <summary> Activate given tile. It means show tile if it is hidden (deactivated) before. </summary>
        /// <param name="tile">Tile.</param>
        void Activate(Tile tile);

        /// <summary> Deactivate given tile. It means hide tile if it is shown (activated). </summary>
        /// <param name="tile">Tile</param>
        void Deactivate(Tile tile);

        /// <summary> Destroy tile and its childred. </summary>
        /// <param name="tile">Tile.</param>
        void Destroy(Tile tile);
    }

    #region Default implementation

    /// <summary> Default implementation does nothing. </summary>
    internal class TileAcivator : ITileActivator
    {
        /// <inheritdoc />
        public void PreLoad(Tile tile)
        {
        }

        /// <inheritdoc />
        public void Activate(Tile tile)
        {
        }

        /// <inheritdoc />
        public void Deactivate(Tile tile)
        {
        }

        /// <inheritdoc />
        public void Destroy(Tile tile)
        {
        }
    }

    #endregion
}
