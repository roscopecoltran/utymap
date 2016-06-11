using UnityEngine;

namespace UtyMap.Unity.Core.Utils
{
    internal static class ColorUtils
    {
        public static Color FromInt(int rgba)
        {
            var r = (byte) ((rgba >> 24) & 0xff);
            var g = (byte) ((rgba >> 16) & 0xff);
            var b = (byte) ((rgba >> 8) & 0xff);
            var a = (byte) (rgba & 0xff);

            return new Color(r / 255f, g / 255f, b / 255f, a / 255f);
        }
    }
}
