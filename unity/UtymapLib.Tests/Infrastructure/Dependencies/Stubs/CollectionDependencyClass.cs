using System.Collections.Generic;
using Assets.UtymapLib.Infrastructure.Dependencies;

namespace UtymapLib.Tests.Infrastructure.Dependencies.Stubs
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
