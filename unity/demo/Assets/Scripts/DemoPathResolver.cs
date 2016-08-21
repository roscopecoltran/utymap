using System;
using UtyMap.Unity.Infrastructure.IO;

namespace Assets.Scripts
{
    class DemoPathResolver : IPathResolver
    {
        public string Resolve(string path)
        {
#if UNITY_ANDROID
            // TODO detect real path somehow
            return @"/storage/sdcard0/UtyMap/" + path;
#else
            return "Assets//Resources//" + path;
#endif
        }
    }
}
