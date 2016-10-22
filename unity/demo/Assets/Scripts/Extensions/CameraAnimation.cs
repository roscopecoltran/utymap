using System;
using UnityEngine;

namespace Assets.Scripts.Extensions
{
    ///<summary> Camera animation script. </summary>
    /// <remarks> 
    /// This script is based on approach found here: 
    /// https://alastaira.wordpress.com/2013/11/08/smooth-unity-camera-transitions-with-animation-curves/>  
    /// </remarks>
    public class CameraAnimation : MonoBehaviour
    {
        public GameObject Pivot;
        public event EventHandler Finished;

        // How camera pitch (i.e. rotation about x axis) should vary with zoom
        private AnimationCurve _pitchCurve;
        // How far the camera should be placed back along the chosen pitch based on zoom
        private AnimationCurve _distanceCurve;
        private Transform _targetTransform;

        private float _distance = 400;
        private float _angle = 45;
        private float _animationTime = int.MaxValue;
        private float _animationDuration;
        private bool _isInversed;

        public void Play(Transform targetTransform, float duration, bool awayFromTarget)
        {
            _targetTransform = targetTransform;
            _animationTime = 0;
            _animationDuration = duration;
            _isInversed = !awayFromTarget;
        }

        void Start()
        {
            _pitchCurve = AnimationCurve.EaseInOut(0, Pivot.transform.rotation.eulerAngles.x, 1, _angle);
            Keyframe[] ks = new Keyframe[2];
            ks[0] = new Keyframe(0, Pivot.transform.position.y);
            ks[0].outTangent = 0;
            ks[1] = new Keyframe(1, _distance);
            ks[1].inTangent = 90;

            _distanceCurve = new AnimationCurve(ks);
        }

        void Update()
        {
            if (_animationTime > _animationDuration)
                return;

            _animationTime += Time.deltaTime;

            var zoom = Math.Min(_animationDuration, _animationTime) / _animationDuration;
            var isLastAnimationFrame = Math.Abs(zoom - 1) < float.Epsilon;

            if (_isInversed)
                zoom = 1 - zoom;

            // Calculate the appropriate pitch (x rotation) for the camera based on current zoom
            float targetRotX = _pitchCurve.Evaluate(zoom);

            // The desired yaw (y rotation) is to match that of the target object
            float targetRotY = _targetTransform.rotation.eulerAngles.y;

            // Create target rotation as quaternion
            // Set z to 0 as we don't want to roll the camera
            Quaternion targetRot = Quaternion.Euler(targetRotX, targetRotY, 0.0f);

            // Calculate in world-aligned axis, how far back we want the camera to be based on current zoom
            Vector3 offset = Vector3.forward * _distanceCurve.Evaluate(zoom);

            // Then subtract this offset based on the current camera rotation
            Vector3 targetPos = _targetTransform.position - targetRot * offset;

            Pivot.transform.position = targetPos;
            Pivot.transform.rotation = targetRot;

            if (isLastAnimationFrame)
                OnFinished();
        }

        private void OnFinished()
        {
            EventHandler handler = Finished;
            if (handler != null)
                handler(this, new EventArgs());
        }
    }
}
