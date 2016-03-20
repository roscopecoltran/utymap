using System.Runtime.InteropServices;

namespace Assets.UtymapLib.Maps.Loader
{
    /// <summary> Provides the way to build tile encapsulating Utymap implementation. </summary>
    internal static class UtymapLib
    {
        /// <summary> Configure utymap. Should be called first. </summary>
        /// <param name="stringPath"> Path to string table. </param>
        /// <param name="dataPath"> Path to map data. </param>
        /// <param name="onError"> OnError callback. </param>
        public static void Configure(string stringPath, string dataPath, OnError onError)
        {
            configure(stringPath, dataPath, onError);
        }

        /// <summary>
        ///     Adds map data to in-memory storage.
        ///     Supported formats: shapefile, osm xml, osm pbf.
        /// </summary>
        /// <param name="stylePath"> Stylesheet path. </param>
        /// <param name="path"> Path to file. </param>
        /// <param name="startLod"> Start level of details for which data should be imported. </param>
        /// <param name="endLod"> End level of details for which data should be imported. </param>
        /// <param name="onError"> OnError callback. </param>
        public static void AddToInMemoryStore(string stylePath, string path, int startLod, int endLod, OnError onError)
        {
            addToInMemoryStore(stylePath, path, startLod, endLod, onError);
        }

        /// <summary> Registers element builder for processing. </summary>
        /// <param name="name"> Name of element builder in stylesheet. </param>
        public static void RegisterElementBuilder(string name)
        {
            registerElementBuilder(name);
        }

        /// <summary> Loads tile. </summary>
        /// <param name="stylePath"> Stylesheet path. </param>
        /// <param name="tileX"> Tile X. </param>
        /// <param name="tileY"> Tile Y. </param>
        /// <param name="levelOfDetails"> Level of details. </param>
        /// <param name="onMeshBuilt"></param>
        /// <param name="onElementLoaded"></param>
        /// <param name="onError"></param>
        public static void LoadTile(string stylePath, int tileX, int tileY, int levelOfDetails,
            OnMeshBuilt onMeshBuilt, OnElementLoaded onElementLoaded, OnError onError)
        {
            loadTile(stylePath, tileX, tileY, levelOfDetails, onMeshBuilt, onElementLoaded, onError);
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
        private static extern void configure(string stringPath, string dataPath, OnError errorHandler);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern void addToInMemoryStore(string stylePath, string path, int startLod, int endLod,
            OnError errorHandler);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern void registerElementBuilder(string name);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern int loadTile(string stylePath, int tileX, int tileY, int levelOfDetails,
            OnMeshBuilt meshBuiltHandler,
            OnElementLoaded elementLoadedHandler, OnError errorHandler);

        [DllImport("UtyMapLib", CallingConvention = CallingConvention.StdCall)]
        private static extern void cleanup();

        #endregion
    }
}