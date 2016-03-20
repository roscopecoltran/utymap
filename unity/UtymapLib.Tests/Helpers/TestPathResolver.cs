using System.IO;
using Assets.UtymapLib.Infrastructure.IO;

namespace UtymapLib.Tests.Helpers
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
