using Utymap.UnityLib.Infrastructure.Config;

namespace Utymap.UnityLib.Tests.Infrastructure.Dependencies.Stubs
{
    class ConfigurableClass: IConfigurable
    {
        public IConfigSection ConfigSection { get; set; }
        public void Configure(IConfigSection config)
        {
            ConfigSection = config;
        }
    }
}
