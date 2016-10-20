using UnityEngine;
using UnityEngine.UI;

namespace Assets.Scripts.Extensions
{
    public class CompassBehaviour : MonoBehaviour
    {
        public Image Pointer;
        public int Scale = 25;

        private Transform _target;
        private RectTransform _rectTransform;

        void Start()
        {
            _target = gameObject.transform;
            _rectTransform = Pointer.rectTransform;
        }

        void FixedUpdate()
        {
            float angDeg = _target.eulerAngles.y + 90;
            float angRed = angDeg * Mathf.Deg2Rad;

            _rectTransform.anchoredPosition = new Vector2(Mathf.Cos(angRed) * Scale, Mathf.Sin(angRed) * Scale);
        }
    }
}
