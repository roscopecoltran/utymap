using System;
using System.Collections.Generic;
using System.Text;

namespace Utymap.UnityLib.Infrastructure.Reactive
{
    public interface ICancelable : IDisposable
    {
        bool IsDisposed { get; }
    }
}
