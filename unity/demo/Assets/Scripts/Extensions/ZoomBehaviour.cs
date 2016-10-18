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
        private int _levelOfDetails = 16;

        void Start()
        {
            _autoCam = FindObjectOfType<AutoCam>();

            _camAnimation = FindObjectOfType<CameraAnimation>();
            _camAnimation.Finished += (sender, args) => _autoCam.enabled = true;

            var messageBus = ApplicationManager.Instance.GetService<IMessageBus>();

            messageBus.AsObservable<OnZoomChanged>()
                .Subscribe(msg =>
                {
                    if (msg.LevelOfDetails == _levelOfDetails)
                        return;

                    _autoCam.enabled = false;
                    _camAnimation.Play(CameraAnimationDuration, msg.LevelOfDetails < _levelOfDetails);
                    _levelOfDetails = msg.LevelOfDetails;
                });
        }
    }
}
