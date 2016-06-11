using UtyMap.Unity.Core.Tiling;

namespace UtyMap.Unity.Core.Models
{
    /// <summary> Responsible for building models. </summary>
    public interface IModelBuilder
    {
        /// <summary> Builds element. </summary>
        /// <remarks> Called on UI thread. </remarks>
        void BuildElement(Tile tile, Element element);

        /// <summary> Builds mesh. </summary>
        /// <remarks> Called on UI thread. </remarks>
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
