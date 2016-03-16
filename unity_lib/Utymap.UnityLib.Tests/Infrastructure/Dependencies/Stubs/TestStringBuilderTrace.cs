using System;
using System.Text;
using Utymap.UnityLib.Infrastructure.Diagnostic;

namespace Utymap.UnityLib.Tests.Infrastructure.Dependencies.Stubs
{
    public class TestStringBuilderTrace: DefaultTrace
    {
        private readonly StringBuilder _sb;

        public TestStringBuilderTrace(StringBuilder sb)
        {
            _sb = sb;
        }

        protected override void WriteRecord(RecordType type, string category, string message, Exception exception)
        {
            _sb.AppendLine(String.Format("{0} {1}", category, message));
        }
    }
}
