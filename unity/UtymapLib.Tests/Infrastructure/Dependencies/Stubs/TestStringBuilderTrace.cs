using System;
using System.Text;
using Assets.UtymapLib.Infrastructure.Diagnostic;

namespace UtymapLib.Tests.Infrastructure.Dependencies.Stubs
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
