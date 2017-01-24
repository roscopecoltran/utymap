using System;
using Assets.Scripts.Debug;
using Assets.Scripts.Environment;
using Assets.Scripts.Environment.Reactive;
using Assets.Scripts.Scene;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using IContainer = UtyDepend.IContainer;
using Container = UtyDepend.Container;
using Component = UtyDepend.Component;

namespace Assets.Scripts
{
    /// <summary> Provides unified way to work with application state from different scenes. </summary>
    /// <remarks> This class should be only one singleton in demo app. </remarks>
    internal class ApplicationManager
    {
        private const string FatalCategoryName = "Fatal";

        private IContainer _container;
        private DebugConsoleTrace _trace;
        private CompositionRoot _compositionRoot;

        public bool IsInitialized { get; private set; }

        #region Singleton implementation

        private ApplicationManager()
        {
        }

        public static ApplicationManager Instance { get { return Nested.__instance; } }

        private class Nested
        {
            // Explicit static constructor to tell C# compiler
            // not to mark type as beforefieldinit
            static Nested() { }

            internal static readonly ApplicationManager __instance = new ApplicationManager();
        }

        #endregion

        #region Initialization logic

        public void InitializeFramework(ConfigBuilder configBuilder, Action<CompositionRoot> initAction)
        {
            IsInitialized = false;

            // Need to dispose all previously used components as we're going to create new ones.
            if (_container != null)
                _container.Dispose();

            // create default container which should not be exposed outside to avoid Service Locator pattern.
            _container = new Container();

            // create trace to log important messages
            _trace = new DebugConsoleTrace();

            // UtyMap requires some files/directories to be precreated.
            InstallationApi.EnsureFileHierarchy(_trace);

            // Setup RX configuration.
            UnityScheduler.SetDefaultForUnity();

            // subscribe to unhandled exceptions in RX
            MainThreadDispatcher.RegisterUnhandledExceptionCallback(ex =>
                _trace.Error(FatalCategoryName, ex, "Unhandled exception"));

            try
            {
                var config = configBuilder
                    .SetStringIndex("Index/")
                    .SetSpatialIndex("Index/")
                    .Build();

                // create entry point for utymap functionallity
                _compositionRoot = new CompositionRoot(_container, config)
                    // override default services with unity specific implementation
                    .RegisterAction((c, _) => c.RegisterInstance<ITrace>(_trace))
                    .RegisterAction((c, _) => c.Register(Component.For<IPathResolver>().Use<UnityPathResolver>()))
                    .RegisterAction((c, _) => c.Register(Component.For<INetworkService>().Use<UnityNetworkService>()))
                    // register scene specific services
                    .RegisterAction((c, _) => c.Register(Component.For<UnityModelBuilder>().Use<UnityModelBuilder>()))
                    .RegisterAction((c, _) => c.Register(Component.For<MaterialProvider>().Use<MaterialProvider>()))
                    // register default mapcss
                    .RegisterAction((c, _) => c.Register(Component.For<Stylesheet>().Use<Stylesheet>(@"MapCss/default/default.mapcss")));

                // this is the way to insert custom extensions from outside. You may need to do it for some scenes.
                initAction(_compositionRoot);

                // setup object graph
                _compositionRoot.Setup();

                IsInitialized = true;
            }
            catch (Exception ex)
            {
                _trace.Error(FatalCategoryName, ex, "Cannot setup object graph.");
                throw;
            }
        }

        /// <summary> Creates debug console in scene. </summary>
        /// <remarks> 
        ///     Console is way to debug/investigate app behavior on real devices when 
        ///     regular debugger is not applicable.
        /// </remarks>
        public void CreateDebugConsole(bool isOpen = false)
        {
            // NOTE DebugConsole is based on some adapted solution found in Internet
            var consoleGameObject = new GameObject("_DebugConsole_");
            var console = consoleGameObject.AddComponent<DebugConsole>();
            _trace.SetConsole(console);
            // that is not nice, but we need to use commands registered in DI with their dependencies
            console.SetContainer(_container);
            console.IsOpen = isOpen;
        }

        #endregion

        /// <summary> Gets service of T from container. </summary>
        /// <remarks>
        ///    Extensive usage of this methid leads to service locator antipatter. 
        ///    So use it carefully.
        /// </remarks>
        public T GetService<T>()
        {
            return _container.Resolve<T>();
        }
    }
}
