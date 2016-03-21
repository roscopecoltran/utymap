using System;
using Assets.UtymapLib.Infrastructure.IO;

namespace Assets.Scripts
{
    class DemoPathResolver : IPathResolver
    {
        public string Resolve(string path)
        {
#if UNITY_ANDROID
            if (path.StartsWith(PathPrefix))
                return path;
            return String.Format("{0}/{1}", PathPrefix, path.Replace(@"\", "/"));
#else
            return "Assets//Resources//" + path;
#endif
        }
    }
}
