using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using Assets.UtymapLib.Core.Models;
using Assets.UtymapLib.Core.Tiling;
using Assets.UtymapLib.Infrastructure;
using Assets.UtymapLib.Infrastructure.Config;
using Assets.UtymapLib.Infrastructure.Dependencies;
using Assets.UtymapLib.Infrastructure.Diagnostic;
using Assets.UtymapLib.Infrastructure.IO;
using Assets.UtymapLib.Maps.Elevation;
using Assets.UtymapLib.Maps.Geocoding;
using Assets.UtymapLib.Maps.Imaginary;
using Assets.UtymapLib.Maps.Loader;

// has to be there due to Unity3d's generated project 
[assembly: InternalsVisibleTo("UtymapLib.Tests")]

namespace Assets.UtymapLib
{
    /// <summary> Represents application's composition root. </summary>
    /// <remarks> 
    ///     At this level, application setups object graph by specifying 
    ///     concrete implementations. 
    ///  </remarks>
    public class CompositionRoot : IDisposable
    {
        private bool _isInitialized;
        private readonly IContainer _container;
        private readonly IConfigSection _configSection;
        private readonly List<Action<IContainer, IConfigSection>> _bootstrapperActions;

        /// <summary> Creates instance of <see cref="CompositionRoot"/>. </summary>
        /// <param name="container"> Dependency injection container. </param>
        /// <param name="configSection"> Application configuration. </param>
        public CompositionRoot(IContainer container, IConfigSection configSection)
        {
            _container = container;
            _configSection = configSection;
            _bootstrapperActions = new List<Action<IContainer, IConfigSection>>();
        }

        /// <summary> Registers action for setup which can extend/replace default service implementations. </summary>
        public CompositionRoot RegisterAction(Action<IContainer, IConfigSection> action)
        {
            if (_isInitialized)
                throw new InvalidOperationException(Strings.CannotRegisteActionIfSetupIsComplete);

            _bootstrapperActions.Add(action);
            return this;
        }

        /// <summary> Performs object graph setup. </summary>
        public CompositionRoot Setup()
        {
            if (_isInitialized)
                throw new InvalidOperationException(Strings.SetupIsCalledMoreThanOnce);

            // register all services which has to be registered, but concrete implementation 
            // can be overriden in custom action by RegisterAction

            // infrastructure services
            _container.Register(Component.For<ITrace>().Use<DefaultTrace>());
            _container.Register(Component.For<IMessageBus>().Use<MessageBus>());
            _container.Register(Component.For<IPathResolver>().Use<PathResolver>());
            _container.Register(Component.For<IFileSystemService>().Use<FileSystemService>());

            // core services 
            _container.Register(Component.For<IModelBuilder>().Use<ModelBuilder>());
            _container.Register(Component.For<IElevationProvider>().Use<SrtmElevationProvider>().SetConfig(_configSection));
            _container.Register(Component.For<ITileController>().Use<TileController>().SetConfig(_configSection));
            _container.Register(Component.For<IMapDataLoader>().Use<MapDataLoader>().SetConfig(_configSection));
            _container.Register(Component.For<IGeocoder>().Use<NominatimGeocoder>().SetConfig(_configSection));
            _container.Register(Component.For<ImaginaryProvider>().Use<BingImaginaryProvider>().SetConfig(_configSection));

            // go through all actions to add/override services.
            _bootstrapperActions.ForEach(action => action(_container, _configSection));
            
            _isInitialized = true;

            return this;
        }

        /// <summary> Returns instance registered in container by its type. </summary>
        /// <remarks> This is classical Service Locator antipattern. Avoid its usage if possible. </remarks>
        public T GetService<T>()
        {
            return _container.Resolve<T>();
        }

        /// <inheritdoc />
        public void Dispose()
        {
            _container.Dispose();
        }
    }
}
