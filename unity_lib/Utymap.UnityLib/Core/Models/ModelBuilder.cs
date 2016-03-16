namespace Utymap.UnityLib.Core.Models
{
    /// <summary> Responsible for building models. </summary>
    public interface IModelBuilder
    {
        /// <summary> Builds element. </summary>
        void BuildElement(Element element);

        /// <summary> Builds mesh. </summary>
        void BuildMesh(Mesh mesh);
    }

    /// <summary> Empty implementation which does nothing. </summary>
    internal class ModelBuilder : IModelBuilder
    {
        /// <inheritdoc />
        public void BuildElement(Element element)
        {
        }

        /// <inheritdoc />
        public void BuildMesh(Mesh mesh)
        {
        }
    }
}
