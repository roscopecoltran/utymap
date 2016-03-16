using UnityEngine;

namespace Utymap.UnityLib.Core.Utils
{
    internal static class ColorUtils
    {
        public static Color32 FromInt(int rgba)
        {
            var r = (byte) ((rgba >> 24) & 0xff);
            var g = (byte) ((rgba >> 16) & 0xff);
            var b = (byte) ((rgba >> 8) & 0xff);
            var a = (byte) ((rgba >> 0) & 0xff);

            return new Color32(r, g, b, a);
        }
    }
}
