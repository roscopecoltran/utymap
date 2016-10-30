using UnityEngine;
using UtyMap.Unity.Core.Utils;
using UtyMap.Unity.Maps.Data;

namespace Assets.Scripts.Extensions
{
    /// <summary> This class is workaround for slow tile loading. </summary>
    /// <remarks> 
    ///     Actual reason can be slow internet connection, remote server
    ///     performance, amount of map data to be processed. 
    /// </remarks>
    class CharacterProtector : MonoBehaviour
    {
        private const string PlaneName = "ProtectionPlane";
        private const int LevelOfDetails = 16;

        private GameObject _plane;

        private MapElevationLoader _eleLoader;

        void Start()
        {
            _plane = GameObject.CreatePrimitive(PrimitiveType.Plane);
            _plane.name = PlaneName;
            _plane.GetComponent<MeshRenderer>().enabled = false;
            _plane.transform.localScale = new Vector3(0.5f, 0.1f, 0.5f);

            _eleLoader = ApplicationManager.Instance.GetService<MapElevationLoader>();
        }

        void OnCollisionEnter(Collision collision)
        {
            if (collision.gameObject.name == PlaneName)
            {
                var position = transform.position;
                var coordinate = GeoUtils.ToGeoCoordinate(PositionConfiguration.StartPosition, position.x, position.z);
                var quadKey = GeoUtils.CreateQuadKey(coordinate, LevelOfDetails);

                var height = (float) _eleLoader.Load(quadKey, coordinate);
                transform.position = new Vector3(position.x, height, position.z);
            }
        }

        void Update()
        {
            var position = gameObject.transform.position;
            // NOTE this won't work if character below y.
            _plane.transform.position = new Vector3(position.x, -1.2f, position.z);
        }
    }
}
