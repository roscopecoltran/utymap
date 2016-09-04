using System.IO;
using UtyMap.Unity.Infrastructure.IO;

namespace Assets.Scripts.Environment
{
    class DemoPathResolver : IPathResolver
    {
        private readonly string _root = EnvironmentApi.ExternalDataPath;

        /// <inheritdoc />
        public string Resolve(string path)
        {
            return path.StartsWith(_root) ? path : Path.Combine(_root, path);
        }
    }
}
