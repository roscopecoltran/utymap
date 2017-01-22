using System;
using System.Collections.Generic;
using System.Linq;
using Assets.Scripts;
using UnityEngine;
using UtyMap.Unity.Core;
using UtyMap.Unity.Core.Utils;

namespace Assets.Scenes.Surface.Scripts
{
    class SurfaceCameraController : MonoBehaviour
    {
        private float _originDistance = 1000;

        private int _minLod = 15;
        private int _maxLod = 16;
        private int _currentLod;
        private float _lodStep;

        private float _closestDistance = 5;
        private Vector3 _lastPosition = Vector3.zero;

        private readonly Vector3 _origin = Vector3.zero;
        private GeoCoordinate _geoOrigin = new GeoCoordinate(52.53193, 13.38710);

        public GameObject Planet;

        private readonly HashSet<QuadKey> _loadedQuadKeys = new HashSet<QuadKey>();

        void Start()
        {
            _lodStep = (transform.position.y - _closestDistance) / (_maxLod - _minLod);

            UpdateLod();
            BuildInitial();
        }

        void Update()
        {
            // no movements
            if (_lastPosition == transform.position)
                return;

            _lastPosition = transform.position;

            UpdateLod();

            BuildIfNecessary();

            KeepOrigin();
        }

        void OnGUI()
        {
            GUI.Label(new Rect(0, 0, Screen.width, Screen.height),
                String.Format("Position: {0} \n LOD: {1}", transform.position, _currentLod));
        }

        private void KeepOrigin()
        {
            if (Vector3.Distance(new Vector2(transform.position.x, transform.position.z), _origin) < _originDistance)
                return;

            Vector3 direction = transform.position - _origin;

            transform.position = _origin;
            Planet.transform.position += direction * -1;

            _geoOrigin = GeoUtils.ToGeoCoordinate(_geoOrigin, new Vector2(direction.x, direction.z));
        }

        /// <summary> Updates current lod level based on current position. </summary>
        private void UpdateLod()
        {
            var distance = transform.position.y - _closestDistance;
            _currentLod = Mathf.Clamp(_maxLod - (int)Math.Round(distance / _lodStep), _minLod, _maxLod);
        }

        /// <summary> Builds planet on initial lod. </summary>
        private void BuildInitial()
        {
            var quadKey = GeoUtils.CreateQuadKey(_geoOrigin, _minLod);
            var quadKeys = new List<QuadKey>(9);
            foreach (var q in GetNeighbours(quadKey))
                quadKeys.Add(q);

            BuildQuadKeys(Planet, quadKeys);
        }

        /// <summary> Builds quadkeys if necessary. Decision is based on visible quadkey and lod level. </summary>
        private void BuildIfNecessary()
        {
            var quadKey = GetQuadKey(transform.position);
            var quadKeys = new List<QuadKey>();

            // zoom in or pan
            if (quadKey.LevelOfDetail <= _currentLod)
            {
                quadKeys.AddRange(GetNeighbours(quadKey));
            }
            // zoom out
            else if (quadKey.LevelOfDetail > _currentLod)
            {
                var qName = quadKey.ToString();
                var q = QuadKey.FromString(qName.Substring(0, qName.Length - 1));
                quadKeys.AddRange(GetNeighbours(q));
            }

            if (quadKeys.Any())
            {
                BuildQuadKeys(Planet, quadKeys);
                DestroyExcept(quadKeys);
            }
        }

        /// <summary> Builds quadkeys </summary>
        private void BuildQuadKeys(GameObject parent, IEnumerable<QuadKey> quadKeys)
        {
            var projection = new CartesianProjection(_geoOrigin);
            // TODO
            foreach (var quadKey in quadKeys)
                if (!_loadedQuadKeys.Contains(quadKey))
                {
                    QuadTreeSystem.BuildQuadTree(parent, quadKey, projection);
                    _loadedQuadKeys.Add(quadKey);
                }
        }

        private void DestroyExcept(List<QuadKey> quadKeys)
        {
            // TODO can be improved
            foreach (var trans in Planet.GetComponentsInChildren<Transform>())
            {
                if (trans.gameObject == Planet)
                    continue;

                var quadKey = QuadKey.FromString(trans.name);
                if (!quadKeys.Contains(quadKey))
                {
                    GameObject.Destroy(trans.gameObject);
                    _loadedQuadKeys.Remove(quadKey);
                }
            }
        }

        private IEnumerable<QuadKey> GetNeighbours(QuadKey quadKey)
        {
            yield return new QuadKey(quadKey.TileX, quadKey.TileY, quadKey.LevelOfDetail);

            yield return new QuadKey(quadKey.TileX - 1, quadKey.TileY, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX - 1, quadKey.TileY + 1, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX, quadKey.TileY + 1, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX + 1, quadKey.TileY + 1, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX + 1, quadKey.TileY, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX + 1, quadKey.TileY - 1, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX, quadKey.TileY - 1, quadKey.LevelOfDetail);
            yield return new QuadKey(quadKey.TileX - 1, quadKey.TileY - 1, quadKey.LevelOfDetail);
        }

        private QuadKey GetQuadKey(Vector3 point)
        {
            var coordinate = GeoUtils.ToGeoCoordinate(_geoOrigin, new Vector2(point.x, point.z));
            return GeoUtils.CreateQuadKey(coordinate, _currentLod);
        }
    }
}
