using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;
using UtyMap.Unity.Infrastructure;
using UtyRx;

using UnityStandardAssets.Cameras;
using UnityStandardAssets.Characters.ThirdPerson;

namespace Assets.Scripts.Extensions
{
    public class ZoomBehaviour : MonoBehaviour
    {
        private const int CameraAnimationDuration = 3;
        private const float SpeedMultiplier = 10;

        private AutoCam _autoCam;
        private CameraAnimation _camAnimation;
        private ThirdPersonCharacter _character;

        private float _originalSpeed;
        private int _levelOfDetails = 16;

        void Start()
        {
            _autoCam = FindObjectOfType<AutoCam>();
            
            _character = FindObjectOfType<ThirdPersonCharacter>();
            _originalSpeed = _character.m_MoveSpeedMultiplier;

            _camAnimation = FindObjectOfType<CameraAnimation>();
            _camAnimation.Finished += (sender, args) => _autoCam.enabled = true;

            var messageBus = ApplicationManager.Instance.GetService<IMessageBus>();

            messageBus.AsObservable<OnZoomChanged>()
                .Subscribe(msg =>
                {
                    if (msg.LevelOfDetails == _levelOfDetails)
                        return;

                    bool isZoomOut = msg.LevelOfDetails < _levelOfDetails;

                    _autoCam.enabled = false;
                    _camAnimation.Play(CameraAnimationDuration, isZoomOut);
                    _levelOfDetails = msg.LevelOfDetails;

                    _character.m_MoveSpeedMultiplier = isZoomOut ? _originalSpeed * SpeedMultiplier : _originalSpeed;
                });
        }
    }
}
