using UnityEngine;
using UnityEngine.SceneManagement;

namespace Assets.Scripts.Menu
{
    class BackButton : MonoBehaviour
    {
        void OnGUI()
        {
            if (GUI.Button(new Rect(Screen.width - 120, 20, 100, 30), "Back to menu"))
                SceneManager.LoadScene("MainScene");
        }
    }
}
