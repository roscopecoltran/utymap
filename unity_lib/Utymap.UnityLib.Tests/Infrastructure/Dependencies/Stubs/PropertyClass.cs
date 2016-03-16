using Utymap.UnityLib.Infrastructure.Dependencies;

namespace Utymap.UnityLib.Tests.Infrastructure.Dependencies.Stubs
{
    public interface ITestInterface
    {

    }

    public class TestInterface : ITestInterface
    {
        
    }

    public interface IPropertyClass
    {
    }

    public class PropertyClass : IPropertyClass
    {
        [Dependency]
        public ITestInterface Test { get; set; }
    }
}
