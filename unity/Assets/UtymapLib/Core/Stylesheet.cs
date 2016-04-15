namespace Assets.UtymapLib.Core
{
    /// <summary> Represents stylesheet. </summary>
    public class Stylesheet
    {
        public string ImaginaryFile { get; set; }

        public string Path { get; private set; }

        public Stylesheet(string path)
        {
            Path = path;
        }
    }
}
