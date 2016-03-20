using Assets.UtymapLib.Infrastructure.Config;

namespace UtymapLib.Tests.Infrastructure.Dependencies.Stubs
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
