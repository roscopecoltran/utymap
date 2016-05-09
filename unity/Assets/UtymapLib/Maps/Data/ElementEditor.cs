using System;
using Assets.UtymapLib.Core;
using Assets.UtymapLib.Core.Models;
using Assets.UtymapLib.Infrastructure.Dependencies;
using Assets.UtymapLib.Infrastructure.Primitives;

namespace Assets.UtymapLib.Maps.Data
{
    /// <summary> Specifies behavior of element editor. </summary>
    public interface IElementEditor
    {
        /// <summary> Adds element. </summary>
        void Add(Element element, Range<int> levelOfDetails);

        /// <summary> Edits element. </summary>
        void Edit(Element element, Range<int> levelOfDetails);

        /// <summary> Marks element with given id. </summary>
        void Delete(long elementId, Range<int> levelOfDetails);
    }

    /// <summary>
    ///     Default implementation of <see cref="IElementEditor"/> which
    ///     works with in-memory store.
    /// </summary>
    internal class ElementEditor : IElementEditor
    {
        private readonly Stylesheet _stylesheet;

        [Dependency]
        public ElementEditor(Stylesheet stylesheet)
        {
            _stylesheet = stylesheet;
        }

        /// <inheritdoc />
        public void Add(Element element, Range<int> levelOfDetails)
        {
            UtymapLib.AddElementToInMemoryStore(_stylesheet.Path, element, levelOfDetails, message =>
            {
                if (!String.IsNullOrEmpty(message))
                    throw new MapDataException(message);
            });
        }

        /// <inheritdoc />
        public void Edit(Element element, Range<int> levelOfDetails)
        {
            throw new NotImplementedException();
        }

        /// <inheritdoc />
        public void Delete(long elementId, Range<int> levelOfDetails)
        {
            throw new NotImplementedException();
        }
    }
}
