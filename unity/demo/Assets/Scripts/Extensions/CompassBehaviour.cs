using UnityEngine;
using UnityEngine.UI;

namespace Assets.Scripts.Extensions
{
    public class CompassBehaviour : MonoBehaviour
    {
        public Image Pointer;
        private Transform _target;
        private RectTransform _rectTransform;

        void Start()
        {
            _target = gameObject.transform;
            _rectTransform = Pointer.rectTransform;
        }

        void FixedUpdate()
        {
            var direction = _target.rotation.eulerAngles;
            _rectTransform.transform.eulerAngles = new Vector3(0, 0, -direction.y);
        }
    }
}
