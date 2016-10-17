using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

namespace Assets.Scripts.Menu
{
    public class ScenePanelManager : MonoBehaviour
    {
        public Button BackButton;
        public Button ZoomInButton;
        public Button ZoomOutButton;

        void Start()
        {
        }

        public void OnBackClick()
        {
            SceneManager.LoadScene("MainScene");
        }

        public void OnZoomIn()
        {
        }

        public void OnZoomOut()
        {
        }
    }
}
