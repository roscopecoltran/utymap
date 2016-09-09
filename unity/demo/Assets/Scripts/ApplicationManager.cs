using System;
using System.Collections.Generic;
using Assets.Scripts.Console;
using Assets.Scripts.Environment;
using Assets.Scripts.Reactive;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Explorer.Customization;
using UtyMap.Unity.Infrastructure;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyRx;
using IContainer = UtyDepend.IContainer;
using Container = UtyDepend.Container;
using Component = UtyDepend.Component;

namespace Assets.Scripts
{
    /// <summary> Provides unified way to work with application state from different scenes. </summary>
    /// <remarks> This class should be only one singleton in demo app. </remarks>
    class ApplicationManager
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
                var config = ConfigBuilder.GetDefault()
                    .SetStringIndex("Index/")
                    .SetSpatialIndex("Index/")
                    .Build();

                // create entry point for utymap functionallity
                _compositionRoot = new CompositionRoot(_container, config)
                    .RegisterAction((c, _) => c.RegisterInstance<ITrace>(_trace))
                    .RegisterAction((c, _) => c.Register(Component.For<IPathResolver>().Use<DemoPathResolver>()))
                    .RegisterAction((c, _) => c.Register(Component.For<IModelBuilder>().Use<DemoModelBuilder>()))
                    .RegisterAction((c, _) => c.Register(Component.For<INetworkService>().Use<DemoNetworkService>()))
                    .RegisterAction((c, _) => c.Register(Component.For<CustomizationService>().Use<CustomizationService>()))
                    .RegisterAction((c, _) => c.Register(Component.For<Stylesheet>().Use<Stylesheet>(@"MapCss/default/default.mapcss")));

                // this is the way to insert custom extensions from outside. You may need to do it for
                // some scenes.
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
        public void CreateDebugConsole(bool isOpen = true)
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

        #region Service locator: use these methods carefully.

        /// <summary> Gets service of T from container. </summary>
        public T GetService<T>()
        {
            return _container.Resolve<T>();
        }

        /// <summary> Gets services of T from container. sS</summary>
        public IEnumerable<T> GetServices<T>()
        {
            return _container.ResolveAll<T>();
        }

        #endregion
    }
}
