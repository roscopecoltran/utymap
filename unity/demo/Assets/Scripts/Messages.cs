namespace Assets.Scripts
{
    internal class OnZoomRequested
    {
        public readonly bool IsZoomOut;

        public OnZoomRequested(bool isZoomOut)
        {
            IsZoomOut = isZoomOut;
        }
    }

    internal class OnZoomChanged
    {
        public readonly int LevelOfDetails;

        public OnZoomChanged(int levelOfDetails)
        {
            LevelOfDetails = levelOfDetails;
        }
    }
}