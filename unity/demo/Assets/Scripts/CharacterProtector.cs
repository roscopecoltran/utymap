using UnityEngine;

namespace Assets.Scripts
{
    /// <summary> This class is workaround for slow tile loading. </summary>
    /// <remarks> 
    ///     Actual reason can be slow internet connection, remote server
    ///     performance, amount of map data to be processed. 
    /// </remarks>
    class CharacterProtector : MonoBehaviour
    {
        private GameObject _plane;

        void Start()
        {
            _plane = GameObject.CreatePrimitive(PrimitiveType.Plane);
            _plane.name = "ProtectionPlane";
            _plane.GetComponent<MeshRenderer>().enabled = false;
            _plane.transform.localScale = new Vector3(0.5f, 0.1f, 0.5f);
        }

        void Update()
        {
            var position = gameObject.transform.position;
            _plane.transform.position = new Vector3(position.x, -1.2f, position.z);
        }
    }
}
