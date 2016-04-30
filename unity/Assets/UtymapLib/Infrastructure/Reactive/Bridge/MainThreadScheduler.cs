namespace Assets.UtymapLib.Infrastructure.Reactive
{
    public static partial class Scheduler
    {
#if CONSOLE
        public static IScheduler MainThread = new CurrentThreadScheduler();
#else
        public static IScheduler MainThread = new UnityMainThreadScheduler();
#endif
    }
}
