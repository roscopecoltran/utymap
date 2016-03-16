using Utymap.UnityLib.Infrastructure.Reactive;
using UnityEngine;

namespace Utymap.UnityLib.Platform
{
    /// <summary> Creates GameObjects. </summary>
    public interface IGameObjectFactory
    {
        /// <summary> Creates new game object with given name. </summary>
        /// <param name="name">Name.</param>
        /// <returns>Game object wrapper.</returns>
        IGameObject CreateNew(string name);

        /// <summary> Creates new game object with given name and parent. </summary>
        /// <param name="name">Name.</param>
        /// <param name="parent">Parent.</param>
        /// <returns>Game object wrapper.</returns>
        IGameObject CreateNew(string name, IGameObject parent);
    }

    #region Default implementation

    /// <summary> Represents default GameObject factory. </summary>
    internal class GameObjectFactory : IGameObjectFactory
    {
        /// <inheritdoc />
        public virtual IGameObject CreateNew(string name)
        {
            return new UnityGameObject(name);
        }

        /// <inheritdoc />
        public IGameObject CreateNew(string name, IGameObject parent)
        {
            var go = CreateNew(name);
            if (go.IsEmpty)
            {
                Observable.Start(() =>
                {
                    go.AddComponent(new GameObject());
                    if (go is UnityGameObject)
                        (go as UnityGameObject).SetParent(parent);
                }, Scheduler.MainThread);
            }
            else
                go.Parent = parent;

            return go;
        }
    }

    #endregion
}