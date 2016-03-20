using System;
using System.Collections.Generic;
using System.Text;

namespace Assets.UtymapLib.Infrastructure.Reactive
{
    public interface ICancelable : IDisposable
    {
        bool IsDisposed { get; }
    }
}
