namespace Assets.UtymapLib
{
    internal static class Strings
    {
        // errors
        public static string CannotRegisteActionIfSetupIsComplete = "Cannot register action if setup is complete.";
        public static string SetupIsCalledMoreThanOnce = "Cannot perform setup more than once.";
        public static string CannotFindSrtmData = "SRTM data cell not found: {0}";

        // information
        public static string LoadElevationFrom = "Load elevation from {0}..";
        public static string NoPresistentElementSourceFound = "No offline map data found for {0}, will query default server: {1}";
        public static string CannotAddDataToInMemoryStore = "Cannot add data to in memory store: {0}";
    }
}
