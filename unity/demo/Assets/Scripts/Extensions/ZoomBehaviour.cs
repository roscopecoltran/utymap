using UnityEngine;
using UtyMap.Unity.Infrastructure;
using UtyRx;

using UnityStandardAssets.Cameras;
using UnityStandardAssets.Characters.ThirdPerson;

namespace Assets.Scripts.Extensions
{
    public class ZoomBehaviour : MonoBehaviour
    {
        private const int CameraAnimationDuration = 2;
        private const float SpeedMultiplier = 10;

        private CameraAnimation _camAnimation;
        private ThirdPersonCharacter _character;

        private float _originalSpeed;
        private int _levelOfDetails = 16;

        void Start()
        {
            _camAnimation = FindObjectOfType<CameraAnimation>();
            _character = FindObjectOfType<ThirdPersonCharacter>();
            _originalSpeed = _character.m_MoveSpeedMultiplier;

            var messageBus = ApplicationManager.Instance.GetService<IMessageBus>();

            messageBus.AsObservable<OnZoomChanged>()
                .Subscribe(msg =>
                {
                    if (msg.LevelOfDetails == _levelOfDetails)
                        return;

                    bool isZoomOut = msg.LevelOfDetails < _levelOfDetails;

                    _camAnimation.Play(CameraAnimationDuration, isZoomOut);
                    _levelOfDetails = msg.LevelOfDetails;

                    _character.m_MoveSpeedMultiplier = isZoomOut 
                        ? _originalSpeed * SpeedMultiplier 
                        : _originalSpeed;
                });
        }
    }
}
