using System.IO;
using Assets.UtymapLib.Core;
using Assets.UtymapLib.Core.Tiling;
using Assets.UtymapLib.Infrastructure.Config;
using Assets.UtymapLib.Infrastructure.Dependencies;
using Assets.UtymapLib.Infrastructure.IO;
using Assets.UtymapLib.Infrastructure.Reactive;

namespace Assets.UtymapLib.Maps.Imaginary
{
    /// <summary> Provides the way to fetch remote imaginary data. </summary>
    public abstract class ImaginaryProvider : IConfigurable
    {
        private string _cachePath;

        [Dependency]
        public IFileSystemService FileSystemService { get; set; }

        /// <summary> Gets url for given quadkey. </summary>
        protected abstract string GetUrl(QuadKey quadkey);

        /// <summary> Gets format extension. </summary>
        protected abstract string FormatExtension { get; }

        /// <summary> Tries to download and store in cache remote tile. </summary>
        public IObservable<string> Get(Tile tile)
        {
            string filePath = Path.Combine(_cachePath, tile.QuadKey + FormatExtension);
            return FileSystemService.Exists(filePath)
                ? Observable.Return(filePath)
                : ObservableWWW
                    .GetAndGetBytes(GetUrl(tile.QuadKey))
                    .Select(bytes =>
                    {
                        using(var stream = FileSystemService.WriteStream(filePath))
                            stream.Write(bytes, 0, bytes.Length);
                        return filePath;
                    });
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _cachePath = configSection.GetString(@"data/cache", null);
        }
    }
}