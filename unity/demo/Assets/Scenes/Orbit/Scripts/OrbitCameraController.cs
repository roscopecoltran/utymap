using System;
using System.Collections.Generic;
using Assets.Scripts;
using Assets.Scripts.Scene;
using UnityEngine;
using UnityEngine.SceneManagement;
using UtyMap.Unity;
using UtyMap.Unity.Data;
using UtyMap.Unity.Infrastructure.Config;
using UtyMap.Unity.Infrastructure.Diagnostic;
using UtyMap.Unity.Utils;
using UtyRx;

namespace Assets.Scenes.Orbit.Scripts
{
    internal sealed class OrbitCameraController : MonoBehaviour
    {
        private const string TraceCategory = "scene.orbit";

        private int _currentLod;
        private int _minLod = 1;
        private int _maxLod = 7;
        private float _radius = 6371;
        private float _lodStep;
        private float _closestDistance;

        private Vector3 _lastPosition = new Vector3(float.MinValue, float.MinValue, float.MinValue);
        private Vector3 _origin = Vector3.zero;

        private IMapDataStore _dataStore;
        private IProjection _projection;
        private Stylesheet _stylesheet;

        public GameObject Planet;

        public bool ShowState = true;
        public bool FreezeLod = false;

        #region Unity's callbacks

        /// <summary> Performs framework initialization once, before any Start() is called. </summary>
        void Awake()
        {
            var appManager = ApplicationManager.Instance;
            appManager.InitializeFramework(ConfigBuilder.GetDefault(), init => { });

            var trace = appManager.GetService<ITrace>();
            var modelBuilder = appManager.GetService<UnityModelBuilder>();
            appManager.GetService<IMapDataStore>()
               .SubscribeOn(Scheduler.ThreadPool)
               .ObserveOn(Scheduler.MainThread)
               .Subscribe(r => r.Item2.Match(
                               e => modelBuilder.BuildElement(r.Item1, e),
                               m => modelBuilder.BuildMesh(r.Item1, m)),
                          ex => trace.Error(TraceCategory, ex, "cannot process mapdata."),
                          () => trace.Warn(TraceCategory, "stop listening mapdata."));

            _dataStore = appManager.GetService<IMapDataStore>();
            _stylesheet = appManager.GetService<Stylesheet>();
            _projection = new SphericalProjection(_radius);
        }

        void Start()
        {
            _closestDistance = _radius * 1.1f;
            _lodStep = (Vector3.Distance(transform.position, _origin) - _closestDistance) / (_maxLod - _minLod);

            BuildInitial();
        }

        void Update()
        {
            // no movements
            if (_lastPosition == transform.position)
                return;

            _lastPosition = transform.position;

            // close to surface
            if (Vector3.Distance(_lastPosition, _origin) < _closestDistance)
            {
                //SceneManager.LoadScene("Surface");
                return;
            }

            UpdateLod();
            BuildIfNecessary();
        }

        void OnGUI()
        {
            if (ShowState)
            {
                GUI.Label(new Rect(0, 0, Screen.width, Screen.height),
                    String.Format("Position: {0} \n LOD: {1} \n Distance: {2}", transform.position, GetLod(),
                        Vector3.Distance(transform.position, _origin)));
            }
        }

        #endregion

        /// <summary> Updates current lod level based on current position. </summary>
        private void UpdateLod()
        {
            if (FreezeLod)
            {
                _currentLod = Math.Max(1, _currentLod);
                return;
            }

            _currentLod = GetLod();
        }

        private int GetLod()
        {
            var distance = Vector3.Distance(transform.position, _origin) - _closestDistance;
            return Math.Max(_maxLod - (int)Math.Round(distance / _lodStep), _minLod);
        }

        /// <summary> Builds planet on initial lod. </summary>
        private void BuildInitial()
        {
            var quadKeys = new List<QuadKey>();
            var maxQuad = GeoUtils.CreateQuadKey(new GeoCoordinate(-89.99, 179.99), _minLod);
            for (int y = 0; y <= maxQuad.TileY; ++y)
                for (int x = 0; x <= maxQuad.TileX; ++x)
                    quadKeys.Add(new QuadKey(x, y, _minLod));

            BuildQuadKeys(Planet, quadKeys);
        }

        /// <summary> Builds quadkeys if necessary. Decision is based on visible quadkey and lod level. </summary>
        private void BuildIfNecessary()
        {
            RaycastHit hit;
            if (!Physics.Raycast(transform.position, (_origin - transform.position).normalized, out hit))
                return;

            // get parent which should have name the same as quadkey string representation
            var hitGameObject = hit.transform.parent.transform.gameObject;
            // skip "cap"
            if (hitGameObject == Planet) return;
            var hitName = hitGameObject.name;
            var hitQuadKey = QuadKey.FromString(hitName);

            var parent = Planet;
            var quadKeys = new List<QuadKey>();

            // zoom in
            if (hitQuadKey.LevelOfDetail < _currentLod)
            {
                quadKeys.AddRange(GetChildren(hitQuadKey));
                // create empty parent and destroy old quadkey.
                parent = new GameObject(hitGameObject.name);
                parent.transform.parent = hitGameObject.transform.parent;
                GameObject.Destroy(hitGameObject.gameObject);
            }
            // zoom out
            else if (hitQuadKey.LevelOfDetail > _currentLod)
            {
                string name = hitName.Substring(0, hitName.Length - 1);
                var quadKey = QuadKey.FromString(name);
                // destroy all siblings
                foreach (var child in GetChildren(quadKey))
                    SafeDestroy(child.ToString());
                // destroy current as it might be just placeholder.
                SafeDestroy(name);
                parent = GetParent(quadKey);
                quadKeys.Add(quadKey);
            }

            BuildQuadKeys(parent, quadKeys);
        }

        /// <summary> Builds quadkeys </summary>
        private void BuildQuadKeys(GameObject parent, IEnumerable<QuadKey> quadKeys)
        {
            foreach (var quadKey in quadKeys)
            {
                var tileGameObject = new GameObject(quadKey.ToString());
                tileGameObject.transform.parent = parent.transform;
                _dataStore.OnNext(new Tile(quadKey, _stylesheet, _projection, tileGameObject));
            }
        }

        /// <summary> Destroys gameobject by its name if it exists. </summary>
        private void SafeDestroy(string name)
        {
            var gameObject = GameObject.Find(name);
            if (gameObject != null)
                GameObject.Destroy(gameObject);
        }

        /// <summary> Gets childrent for quadkey. </summary>
        private IEnumerable<QuadKey> GetChildren(QuadKey quadKey)
        {
            // TODO can be optimized to avoid string allocations.
            var quadKeyName = quadKey.ToString();
            yield return QuadKey.FromString(quadKeyName + "0");
            yield return QuadKey.FromString(quadKeyName + "1");
            yield return QuadKey.FromString(quadKeyName + "2");
            yield return QuadKey.FromString(quadKeyName + "3");
        }

        /// <summary> Gets parent game object for given quadkey. Creates hierarchy if necessary. </summary>
        private GameObject GetParent(QuadKey quadKey)
        {
            // recursion end
            if (quadKey.LevelOfDetail == _minLod)
                return Planet;

            string quadKeyName = quadKey.ToString();
            string parentName = quadKeyName.Substring(0, quadKeyName.Length - 1);
            var parent = GameObject.Find(parentName);
            return parent != null
                ? parent
                : GetParent(QuadKey.FromString(parentName));
        }
    }
}
