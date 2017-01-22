using System;
using TouchScript.Gestures.TransformGestures;
using UnityEngine;

namespace Assets.Scenes.Surface.Scripts
{
    class SurfaceTouchController : MonoBehaviour
    {
        public ScreenTransformGesture TwoFingerMoveGesture;
        public ScreenTransformGesture ManipulationGesture;

        public float PanSpeed = 200f;
        public float RotationSpeed = 200f;
        public float ZoomSpeed = 10f;

        private Transform _pivot;
        private Transform _cam;

        private void Awake()
        {
            _pivot = transform.Find("Pivot");
            _cam = transform.Find("Pivot/Camera");
        }

        private void OnEnable()
        {
            TwoFingerMoveGesture.Transformed += twoFingerTransformHandler;
            ManipulationGesture.Transformed += manipulationTransformedHandler;
        }

        private void OnDisable()
        {
            TwoFingerMoveGesture.Transformed -= twoFingerTransformHandler;
            ManipulationGesture.Transformed -= manipulationTransformedHandler;
        }

        private void manipulationTransformedHandler(object sender, EventArgs e)
        {
            var rotation = Quaternion.Euler(
                ManipulationGesture.DeltaPosition.y / Screen.height * RotationSpeed,
                -ManipulationGesture.DeltaPosition.x / Screen.width * RotationSpeed,
                ManipulationGesture.DeltaRotation);
            _pivot.localRotation *= rotation;
            _cam.transform.localPosition += Vector3.up * (ManipulationGesture.DeltaScale - 1f) * ZoomSpeed;
        }

        private void twoFingerTransformHandler(object sender, EventArgs e)
        {
            _pivot.localPosition += new Vector3(TwoFingerMoveGesture.DeltaPosition.x, 0, TwoFingerMoveGesture.DeltaPosition.y) * PanSpeed;
        }
    }
}
