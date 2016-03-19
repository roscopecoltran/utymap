using Utymap.UnityLib.Core.Tiling;

namespace Utymap.UnityLib.Core.Models
{
    /// <summary> Responsible for building models. </summary>
    public interface IModelBuilder
    {
        /// <summary> Builds element. </summary>
        void BuildElement(Tile tile, Element element);

        /// <summary> Builds mesh. </summary>
        void BuildMesh(Tile tile, Mesh mesh);
    }

    /// <summary> Empty implementation which does nothing. </summary>
    internal class ModelBuilder : IModelBuilder
    {
        /// <inheritdoc />
        public void BuildElement(Tile tile, Element element)
        {
        }

        /// <inheritdoc />
        public void BuildMesh(Tile tile, Mesh mesh)
        {
        }
    }
}
