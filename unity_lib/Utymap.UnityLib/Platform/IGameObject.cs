using System;
using Utymap.UnityLib.Infrastructure.Reactive;
using UnityEngine;

namespace Utymap.UnityLib.Platform
{
    /// <summary>
    ///     Should hold Unity specific GameObject class. Actually, this is workaround which allows to
    ///     use Unity's GameObject outside Unity process.
    /// </summary>
    public interface IGameObject
    {
        /// <summary> True, if gameobject has no components. </summary>
        bool IsEmpty { get; }

        /// <summary> Gets or sets name. </summary>
        string Name { get; }

        /// <summary> Sets parent game object. </summary>
        IGameObject Parent { set; }

        /// <summary> Adds component of given type to game object. </summary>
        /// ///
        /// <typeparam name="T"> Type of component. </typeparam>
        /// <param name="type"> Component type. </param>
        /// <returns>Component.</returns>
        T AddComponent<T>(Type type);

        /// <summary> Adds component to game object. </summary>
        /// <typeparam name="T"> Type of component. </typeparam>
        /// <param name="component"> Component. </param>
        /// <returns> Component. </returns>
        T AddComponent<T>(T component);

        /// <summary> Gets component of given type. </summary>
        /// <typeparam name="T">Type.</typeparam>
        /// <returns>Component.</returns>
        T GetComponent<T>();
    }

    #region Default implementation

    /// <summary> Wrapper of real Unity's GameObject. </summary>
    internal class UnityGameObject : IGameObject
    {
        private readonly string _name;
        private object _gameObject;

        /// <summary> Creates UnityGameObject. Internally creates Unity's GameObject with given name. </summary>
        /// <param name="name">Name.</param>
        public UnityGameObject(string name)
        {
            _name = name;
        }

        /// <inheritdoc />
        public bool IsBehaviourAttached { get; set; }

        /// <inheritdoc />
        public T AddComponent<T>(Type type)
        {
            return (T) ((object) (_gameObject as GameObject).AddComponent(type));
        }

        /// <inheritdoc />
        public T AddComponent<T>(T component)
        {
            // work-around to run Unity-specific
            if (typeof (T).IsAssignableFrom(typeof (GameObject)))
            {
                if (_gameObject != null)
                    throw new InvalidOperationException("GameObject is already added!");

                _gameObject = component;
                if (!string.IsNullOrEmpty(_name))
                    (component as GameObject).name = _name;
            }
            return component;
        }

        /// <inheritdoc />
        public T GetComponent<T>()
        {
            if (_gameObject != null)
                return (T) _gameObject;

            throw new InvalidOperationException("GameObject isn't yet added!");
        }

        /// <inheritdoc />
        public bool IsEmpty
        {
            get { return _gameObject == null; }
        }

        /// <inheritdoc />
        public string Name
        {
            get { return _name; }
        }

        /// <inheritdoc />
        public IGameObject Parent
        {
            set
            {
                Observable.Start(() =>
                {
                    if (IsEmpty)
                        throw new InvalidOperationException("Propery setter: parent cannot be set for empty object: " + Name);
                    (_gameObject as GameObject).transform.parent = value.GetComponent<GameObject>().transform;
                }, Scheduler.MainThread);
            }
        }

        /// <summary>  Sets parent on current thread. </summary>
        internal void SetParent(IGameObject parent)
        {
            if (IsEmpty)
                throw new InvalidOperationException("SetParent: parent cannot be set for empty object: " + Name);

            (_gameObject as GameObject).transform.parent = parent.GetComponent<GameObject>().transform;
        }
    }

    #endregion
}