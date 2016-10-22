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

        private CameraAnimation _camAnimation;
        private ThirdPersonCharacter _character;
        private AutoCam _autoCam;

        private float _originalSpeed;
        private float _originalMove;
        private float _originalTurn;
        private float _originalRoll;

        private int _levelOfDetails = 16;

        void Start()
        {
            _camAnimation = FindObjectOfType<CameraAnimation>();
            _character = FindObjectOfType<ThirdPersonCharacter>();
            _autoCam = FindObjectOfType<AutoCam>();

            _originalSpeed = _character.m_MoveSpeedMultiplier;
            _originalMove = _autoCam.m_MoveSpeed;
            _originalTurn = _autoCam.m_TurnSpeed;
            _originalRoll = _autoCam.m_RollSpeed;

            var messageBus = ApplicationManager.Instance.GetService<IMessageBus>();

            messageBus.AsObservable<OnZoomChanged>()
                .Subscribe(msg =>
                {
                    if (msg.LevelOfDetails == _levelOfDetails)
                        return;

                    bool isZoomOut = msg.LevelOfDetails < _levelOfDetails;

                    _camAnimation.Play(_character.transform, CameraAnimationDuration, isZoomOut);
                    _levelOfDetails = msg.LevelOfDetails;
                    TweakMovementValues(isZoomOut);
                });
        }

        void TweakMovementValues(bool isZoomOut)
        {
            const float speedMultiplier = 10;
            const float autoCamMulitplier = 100;

            _character.m_MoveSpeedMultiplier = isZoomOut ? _originalSpeed * speedMultiplier : _originalSpeed;

            _autoCam.m_MoveSpeed = isZoomOut ? _originalMove * autoCamMulitplier : _originalMove;
            _autoCam.m_TurnSpeed = isZoomOut ? _originalTurn * autoCamMulitplier : _originalTurn;
            _autoCam.m_RollSpeed = isZoomOut ? _originalRoll * autoCamMulitplier : _originalRoll;
        }
    }
}
