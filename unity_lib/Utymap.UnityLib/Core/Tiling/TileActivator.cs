using System;
using Utymap.UnityLib.Infrastructure.Reactive;
using UnityEngine;
using Utymap.UnityLib.Infrastructure.Dependencies;
using Utymap.UnityLib.Infrastructure.Diagnostic;

namespace Utymap.UnityLib.Core.Tiling
{
    /// <summary> Defines logic for tile activation, deactivation and destroy. </summary>
    public interface ITileActivator
    {
        /// <summary> Activate given tile. It means show tile if it is hidden (deactivated) before. </summary>
        /// <param name="tile">Tile.</param>
        void Activate(Tile tile);

        /// <summary> Deactivate given tile. It means hide tile if it is shown (activated). </summary>
        /// <param name="tile">Tile</param>
        void Deactivate(Tile tile);

        /// <summary> Destroy tile and its childred. </summary>
        /// <param name="tile">Tile.</param>
        void Destroy(Tile tile);
    }

    #region Default implementation

    /// <summary> Represents default tile activator. </summary>
    internal class TileActivator: ITileActivator
    {
        private const string LogTag = "tile";

        /// <summary> Trace. </summary>
        [global::System.Reflection.Obfuscation(Exclude = true, Feature = "renaming")]
        [Dependency]
        public ITrace Trace { get; set; }

        /// <inheritdoc />
        public void Activate(Tile tile)
        {
            Trace.Debug(LogTag, "activate tile: {0}", tile.Rectangle.center.ToString());
            Observable.Start(() => ProcessWithChildren(tile, true), Scheduler.MainThread);
        }

        /// <inheritdoc />
        public void Deactivate(Tile tile)
        {
            Trace.Debug(LogTag, "deactivate tile: {0}", tile.Rectangle.center.ToString());
            Observable.Start(() => ProcessWithChildren(tile, false), Scheduler.MainThread);
        }

        /// <inheritdoc />
        public void Destroy(Tile tile)
        {
            Trace.Debug(LogTag, "destroy tile: {0}", tile.Rectangle.center.ToString());
            Observable.Start(() => DestroyWithChildren(tile), Scheduler.MainThread);
        }

        /// <summary> Destroys tile using UnityEngine.Object.Destroy. </summary>
        /// <param name="tile">Tile.</param>
        protected virtual void DestroyWithChildren(Tile tile)
        {
            var parent = tile.GameObject.GetComponent<GameObject>();
            // NOTE should it be recursive as child may have children?
            foreach (Transform child in parent.transform)
                UnityEngine.Object.Destroy(child.gameObject);

            UnityEngine.Object.Destroy(parent);
            // NOTE this is required to release meshes as they are not garbage collected
            Resources.UnloadUnusedAssets();
            GC.Collect();
        }

        /// <summary> Calls UnityEngine.GameObject.SetActive(active) for given tile. </summary>
        /// <param name="tile">Tile.</param>
        /// <param name="active">Active flag.</param>
        protected virtual void ProcessWithChildren(Tile tile, bool active)
        {
            tile.GameObject.GetComponent<GameObject>().SetActive(active);
        }
    }

    #endregion
}
