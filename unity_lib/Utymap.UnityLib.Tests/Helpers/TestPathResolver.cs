using System.IO;
using Utymap.UnityLib.Infrastructure.IO;

namespace Utymap.UnityLib.Tests.Helpers
{
    public class TestPathResolver : IPathResolver
    {
        public string Resolve(string path)
        {
            return path.StartsWith("..") 
                ? path 
                : Path.Combine(TestHelper.TestAssetsFolder, path);
        }
    }
}
