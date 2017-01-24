using System;
using System.Collections.Generic;
using UtyDepend.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Infrastructure.IO;
using UtyMap.Unity.Infrastructure.Primitives;
using UtyRx;

namespace UtyMap.Unity.Data
{
    /// <summary> Loads data from core library. </summary>
    internal class MapDataLoader : ISubject<Tile, Union<Element, Mesh>>, IConfigurable
    {
        private const string TraceCategory = "mapdata.loader";

        private readonly List<IObserver<Union<Element, Mesh>>> _observers = new List<IObserver<Union<Element, Mesh>>>();
        private readonly IPathResolver _pathResolver;
        private readonly ITrace _trace;

        private ElevationDataType _eleDataType = ElevationDataType.Flat;

        public MapDataLoader(IPathResolver pathResolver, ITrace trace)
        {
            _pathResolver = pathResolver;
            _trace = trace;
        }

        /// <inheritdoc />
        public void OnCompleted()
        {
            _observers.ForEach(o => o.OnCompleted());
        }

        /// <inheritdoc />
        public void OnError(Exception error)
        {
            _observers.ForEach(o => o.OnError(error));
        }

        /// <inheritdoc />
        public void OnNext(Tile tile)
        {
            var stylesheetPathResolved = _pathResolver.Resolve(tile.Stylesheet.Path);

            _trace.Info(TraceCategory, "loading tile: {0} using style: {1}", tile.ToString(), stylesheetPathResolved);
            var adapter = new MapDataAdapter(tile, _observers, _trace);

            CoreLibrary.LoadQuadKey(
                stylesheetPathResolved,
                tile.QuadKey,
                _eleDataType,
                adapter.AdaptMesh,
                adapter.AdaptElement,
                adapter.AdaptError);

            _trace.Info(TraceCategory, "tile loaded: {0}", tile.ToString());
        }

        /// <inheritdoc />
        public IDisposable Subscribe(IObserver<Union<Element, Mesh>> observer)
        {
            _observers.Add(observer);
            return Disposable.Empty;
        }

        /// <inheritdoc />
        public void Configure(IConfigSection configSection)
        {
            _eleDataType = (ElevationDataType) configSection.GetInt("data/elevation/type", 0);
        }
    }
}