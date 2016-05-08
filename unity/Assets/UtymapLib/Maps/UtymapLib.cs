using System.Linq;
using System.Runtime.InteropServices;
using Assets.UtymapLib.Core;
using Assets.UtymapLib.Core.Models;
using Assets.UtymapLib.Infrastructure.Primitives;

namespace Assets.UtymapLib.Maps
{
    /// <summary> Provides the way to build tile encapsulating Utymap implementation. </summary>
    internal static class UtymapLib
    {
        /// <summary> Configure utymap. Should be called first. </summary>
        /// <param name="stringPath"> Path to string table. </param>
        /// <param name="dataPath"> Path to map data. </param>
        /// <param name="elePath"> Path to elevation data. </param>
        /// <param name="onError"> OnError callback. </param>
        public static void Configure(string stringPath, string dataPath, string elePath, OnError onError)
        {
            configure(stringPath, dataPath, elePath, onError);
        }

        /// <summary>
        ///     Adds map data to in-memory storage to specific level of detail range.
        ///     Supported formats: shapefile, osm xml, osm pbf.
        /// </summary>
        /// <param name="stylePath"> Stylesheet path. </param>
        /// <param name="path"> Path to file. </param>
        /// <param name="levelOfDetails"> Specifies level of details for which data should be imported. </param>
        /// <param name="onError"> OnError callback. </param>
        public static void AddToInMemoryStore(string stylePath, string path, Range<int> levelOfDetails, OnError onError)
        {
            addToInMemoryStoreInRange(stylePath, path, levelOfDetails.Minimum, levelOfDetails.Maximum, onError);
        }

        /// <summary>
        ///     Adds map data to in-memory storage to specific quadkey.
        ///     Supported formats: shapefile, osm xml, osm pbf.
        /// </summary>
        /// <param name="stylePath"> Stylesheet path. </param>
        /// <param name="path"> Path to file. </param>
        /// <param name="quadKey"> QuadKey. </param>
        /// <param name="onError"> OnError callback. </param>
        public static void AddToInMemoryStore(string stylePath, string path, QuadKey quadKey, OnError onError)
        {
            addToInMemoryStoreInQuadKey(stylePath, path, quadKey.TileX, quadKey.TileY, quadKey.LevelOfDetail, onError);
        }

        public static void AddElementToInMemoryStore(string stylePath, Element element, Range<int> levelOfDetails, OnError onError)
        {
            double[] coordinates = new double[element.Geometry.Length*2];
            for (int i = 0; i < element.Geometry.Length; ++i)
            {
                coordinates[i*2] = element.Geometry[i].Latitude;
                coordinates[i*2 + 1] = element.Geometry[i].Longitude;
            }

            string[] tags = new string[element.Tags.Count * 2];
            var tagKeys = element.Tags.Keys.ToArray();
            for (int i = 0; i < tagKeys.Length; ++i)
            {
                tags[i*2] = tagKeys[i];
                tags[i*2 + 1] = element.Tags[tagKeys[i]];
            }

            addElementToInMemoryStore(stylePath, element.Id,
                coordinates, coordinates.Length,
                tags, tags.Length, levelOfDetails.Minimum, levelOfDetails.Maximum, onError);
        }

        /// <summary> Registers element builder for processing. </summary>
        /// <param name="name"> Name of element builder in stylesheet. </param>
        public static void RegisterElementBuilder(string name)
        {
            registerElementBuilder(name);
        }

        /// <summary> Checks whether there is data for given quadkey. </summary>
        /// <returns> True if there is data for given quadkey. </returns>
        public static bool HasData(QuadKey quadKey)
        {
            return hasData(quadKey.TileX, quadKey.TileY, quadKey.LevelOfDetail);
        }

        /// <summary> Loads quadkey. </summary>
        /// <param name="stylePath"> Stylesheet path. </param>
        /// <param name="quadKey"> QuadKey</param>
        /// <param name="onMeshBuilt"></param>
        /// <param name="onElementLoaded"></param>
        /// <param name="onError"></param>
        public static void LoadQuadKey(string stylePath, QuadKey quadKey,
            OnMeshBuilt onMeshBuilt, OnElementLoaded onElementLoaded, OnError onError)
        {
            loadQuadKey(stylePath, quadKey.TileX, quadKey.TileY, quadKey.LevelOfDetail, 
                onMeshBuilt, onElementLoaded, onError);
        }

        /// <summary> Frees resources. Should be called before application stops. </summary>
        public static void Dispose()
        {
            cleanup();
        }

        #region PInvoke import 

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnMeshBuilt([In] string name,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] [In] double[] vertices, [In] int vertexCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 4)] [In] int[] triangles, [In] int triangleCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 6)] [In] int[] colors, [In] int colorCount);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnElementLoaded([In] long id,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] [In] string[] tags, [In] int tagCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 4)] [In] double[] vertices, [In] int vertexCount,
            [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 6)] [In] string[] styles, [In] int styleCount);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        internal delegate void OnError([In] string message);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern void configure(string stringPath, string dataPath, string elePath, OnError errorHandler);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern void addToInMemoryStoreInRange(string stylePath, string path, int startLod, int endLod,
            OnError errorHandler);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern void addToInMemoryStoreInQuadKey(string stylePath, string path, int tileX, int tileY, int lod, 
            OnError errorHandler);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern void addElementToInMemoryStore(string stylePath, long id, 
            double[] vertices, int vertexLength, string[] tags, int tagLength, 
            int startLod, int endLod, OnError errorHandler);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern void registerElementBuilder(string name);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern bool hasData(int tileX, int tileY, int levelOfDetails);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern int loadQuadKey(string stylePath, int tileX, int tileY, int levelOfDetails,
            OnMeshBuilt meshBuiltHandler, OnElementLoaded elementLoadedHandler, OnError errorHandler);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern void cleanup();

        #endregion
    }
}