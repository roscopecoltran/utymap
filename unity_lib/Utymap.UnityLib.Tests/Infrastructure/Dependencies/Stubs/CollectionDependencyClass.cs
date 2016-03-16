using System.Collections.Generic;
using Utymap.UnityLib.Infrastructure.Dependencies;

namespace Utymap.UnityLib.Tests.Infrastructure.Dependencies.Stubs
{
    public class CollectionDependencyClass
    {
        public IEnumerable<IClassA> Classes { get; private set; }

        [Dependency]
        public CollectionDependencyClass(IEnumerable<IClassA> classes)
        {
            Classes = classes;
        }
    }
}
