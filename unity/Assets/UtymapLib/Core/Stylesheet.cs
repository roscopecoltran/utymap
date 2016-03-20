using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Assets.UtymapLib.Core
{
    /// <summary> Represents stylesheet. </summary>
    public class Stylesheet
    {
        public string Path { get; private set; }

        public Stylesheet(string path)
        {
            Path = path;
        }
    }
}
