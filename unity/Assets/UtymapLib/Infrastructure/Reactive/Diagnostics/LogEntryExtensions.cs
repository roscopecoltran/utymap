using System;
using System.Collections.Generic;
using System.Text;

namespace Assets.UtymapLib.Infrastructure.Reactive.Diagnostics
{
    public static partial class LogEntryExtensions
    {
        public static IDisposable LogToUnityDebug(this IObservable<LogEntry> source)
        {
            return source.Subscribe(new UnityDebugSink());
        }
    }
}