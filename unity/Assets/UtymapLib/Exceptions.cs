using System;
using System.Diagnostics.CodeAnalysis;

namespace Assets.UtymapLib
{
    [SuppressMessage("Microsoft.Usage", "CA2240:ImplementISerializableCorrectly")]
    [Serializable]
    internal class MapDataException : Exception
    {
        /// <summary> Creates <see cref="MapDataException" />. </summary>
        /// <param name="message">Exception message.</param>
        public MapDataException(string message)
            : base(message)
        {
        }
    }
}