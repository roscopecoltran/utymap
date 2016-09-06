using UnityEngine;
using UtyMap.Unity.Infrastructure.Config;

namespace Assets.Scripts
{
    internal class MainSceneBehaviour : MonoBehaviour
    {
        private void Awake()
        {
            ApplicationManager.Instance.InitializeFramework(ConfigBuilder.GetDefault(), _ => { });
        }
    }
}