using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;
using UtyMap.Unity.Infrastructure;

namespace Assets.Scripts.Menu
{
    public class ScenePanelManager : MonoBehaviour
    {
        public Button BackButton;
        public Button ZoomInButton;
        public Button ZoomOutButton;

        private IMessageBus _messageBus;

        void Start()
        {
            _messageBus = ApplicationManager.Instance.GetService<IMessageBus>();
        }

        public void OnBackClick()
        {
            SceneManager.LoadScene("MainScene");
        }

        public void OnZoomIn()
        {
            _messageBus.Send(new OnZoomRequested(false));
        }

        public void OnZoomOut()
        {
            _messageBus.Send(new OnZoomRequested(true));
        }
    }
}
