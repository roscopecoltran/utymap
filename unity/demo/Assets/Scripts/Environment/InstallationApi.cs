using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEngine;
using UtyMap.Unity.Infrastructure.Diagnostic;

namespace Assets.Scripts.Environment
{
    /// <summary> Provides installation API. </summary>
    internal static class InstallationApi
    {
        private const string MarkerFileName = "install_log.txt";

        /// <summary> Ensures required filesystem structure. Should be called from main thread. </summary>
        public static void EnsureFileHierarchy(ITrace trace)
        {
            if (IsInstalled())
                return;

            // NOTE On unity editor is easier for development to use original files.
#if !UNITY_EDITOR
            CopyFiles(GetMapCssFileName(), trace);
            CopyFiles(GetNaturalEarthFileName(), trace);
#endif
            CreateDirectories(GetDirectories());

            MarkAsInstalled();
        }

        #region Hard coded file/directory names

        // NOTE Android platform does not support getting file list from jar.
        private static IEnumerable<string> GetMapCssFileName()
        {
            return new List<string>()
            {
                "mapcss",
                "z1.mapcss",
                "z14.mapcss",
                "z14-lines.mapcss",
                "z14-polygons.mapcss",
                "z16.mapcss",
                "z16-buildings.mapcss",
                "z16-misc.mapcss",
                "z16-poi.mapcss",
                "z16-roads.mapcss",
                "z16-terrain.mapcss",
                "z16-water.mapcss"
            }.Select(f => "MapCss/default/default." + f);
        }

        private static IEnumerable<string> GetNaturalEarthFileName()
        {
            return new List<string>()
            {
                "ne_110m_admin_0_boundary_lines_land",
                "ne_110m_admin_0_scale_rank",
                "ne_110m_lakes",
                "ne_110m_land",
                "ne_110m_populated_places_simple",
                "ne_110m_rivers_lake_centerlines"
            }.SelectMany(f => new List<string>()
            {
                String.Format("{0}.dbf",Path.Combine("NaturalEarth", f)),
                String.Format("{0}.prj",Path.Combine("NaturalEarth", f)),
                String.Format("{0}.shp",Path.Combine("NaturalEarth", f)),
                String.Format("{0}.shx",Path.Combine("NaturalEarth", f))
            });
        }

        private static IEnumerable<string> GetDirectories()
        {
            yield return "Cache";
            yield return "Index";

            for (int i = 1; i <= 16; ++i)
                yield return Path.Combine("Index", i.ToString());
        }

        #endregion

        private static bool IsInstalled()
        {
            return File.Exists(Path.Combine(EnvironmentApi.ExternalDataPath, MarkerFileName));
        }

        private static void MarkAsInstalled()
        {
            File.Create(Path.Combine(EnvironmentApi.ExternalDataPath, MarkerFileName));
        }

        private static void CopyFiles(IEnumerable<string> files, ITrace trace)
        {
            foreach (var fileName in files)
                CopyStreamingAsset(fileName, EnvironmentApi.ExternalDataPath, trace);
        }

        private static void CreateDirectories(IEnumerable<string> directories)
        {
            foreach (var directoryName in directories)
                Directory.CreateDirectory(Path.Combine(EnvironmentApi.ExternalDataPath, directoryName));
        }

        private static void CopyStreamingAsset(string srcAssetRelativePath, string destAbsoluteDirPath, ITrace trace)
        {
            string destAbsolutePath = Path.Combine(destAbsoluteDirPath, srcAssetRelativePath);
            if (File.Exists(destAbsolutePath))
                return;

            // ensure all parent directories exist
            var parentDirectory = Path.GetDirectoryName(destAbsolutePath);
            Directory.CreateDirectory(parentDirectory);

            // read asset
            string srcAssetAbsolutePath = Path.Combine(Application.streamingAssetsPath, srcAssetRelativePath);
            WWW reader = new WWW(srcAssetAbsolutePath);
            while (!reader.isDone) { }

            trace.Info("install", string.Format("copy from {0} to {1} bytes:{2}", srcAssetAbsolutePath, destAbsolutePath, reader.bytes.Length));

            File.WriteAllBytes(destAbsolutePath, reader.bytes);
        }
    }
}
