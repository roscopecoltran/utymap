// defined from .NET Framework 4.5 and NETFX_CORE

#if !NETFX_CORE

namespace Utymap.UnityLib.Infrastructure.Reactive
{
    public interface IProgress<T>
    {
        void Report(T value);
    }
}

#endif