using System;
using System.Collections.Generic;
using Utymap.UnityLib.Core.Models;
using Utymap.UnityLib.Core.Tiling;
using Utymap.UnityLib.Infrastructure;
using Utymap.UnityLib.Infrastructure.Config;
using Utymap.UnityLib.Infrastructure.Dependencies;
using Utymap.UnityLib.Infrastructure.Diagnostic;
using Utymap.UnityLib.Infrastructure.IO;
using Utymap.UnityLib.Maps.Elevation;
using Utymap.UnityLib.Maps.Loader;

namespace Utymap.UnityLib
{
    /// <summary> Represents application's compistion root. </summary>
    public class CompositionRoot : IDisposable
    {
        private bool _isStarted;
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
            if (_isStarted)
                throw new InvalidOperationException(Strings.CannotRegisteActionIfSetupIsComplete);

            _bootstrapperActions.Add(action);
            return this;
        }

        /// <summary> Performs setup </summary>
        public CompositionRoot Setup()
        {
            if (_isStarted)
                throw new InvalidOperationException(Strings.SetupIsCalledMoreThanOnce);

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

            _bootstrapperActions.ForEach(a => a(_container, _configSection));
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
