using System.Collections;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;
using UtyMap.Unity.Infrastructure.Config;

namespace Assets.Scripts.Menu
{
    public class PanelManager : MonoBehaviour
    {
        private const string OpenTransitionName = "Open";
        private const string ClosedStateName = "Closed";

        public Animator initiallyOpen;

        private int _openParameterId;
        private Animator _open;
        private GameObject _previouslySelected;

        public void OnEnable()
        {
            _openParameterId = Animator.StringToHash(OpenTransitionName);

            if (initiallyOpen == null)
                return;

            OpenPanel(initiallyOpen);
        }

        public void OpenPanel (Animator anim)
        {
            if (_open == anim)
                return;

            anim.gameObject.SetActive(true);
            var newPreviouslySelected = EventSystem.current.currentSelectedGameObject;

            anim.transform.SetAsLastSibling();

            CloseCurrent();

            _previouslySelected = newPreviouslySelected;

            _open = anim;
            _open.SetBool(_openParameterId, true);

            //GameObject go = FindFirstEnabledSelectable(anim.gameObject);
            //SetSelected(go);
        }

        //static GameObject FindFirstEnabledSelectable (GameObject gameObject)
        //{
        //    GameObject go = null;
        //    var selectables = gameObject.GetComponentsInChildren<Selectable> (true);
        //    foreach (var selectable in selectables) {
        //        if (selectable.IsActive () && selectable.IsInteractable ()) {
        //            go = selectable.gameObject;
        //            break;
        //        }
        //    }
        //    return go;
        //}

        public void CloseCurrent()
        {
            if (_open == null)
                return;

            _open.SetBool(_openParameterId, false);
            SetSelected(_previouslySelected);
            StartCoroutine(DisablePanelDeleyed(_open));
            _open = null;
        }

        IEnumerator DisablePanelDeleyed(Animator anim)
        {
            bool closedStateReached = false;
            bool wantToClose = true;
            while (!closedStateReached && wantToClose)
            {
                if (!anim.IsInTransition(0))
                    closedStateReached = anim.GetCurrentAnimatorStateInfo(0).IsName(ClosedStateName);

                wantToClose = !anim.GetBool(_openParameterId);

                yield return new WaitForEndOfFrame();
            }

            if (wantToClose)
                anim.gameObject.SetActive(false);
        }

        private void SetSelected(GameObject go)
        {
            EventSystem.current.SetSelectedGameObject(go);

            var standaloneInputModule = EventSystem.current.currentInputModule as StandaloneInputModule;
            if (standaloneInputModule != null)
                return;

            EventSystem.current.SetSelectedGameObject(null);
        }
    }
}
