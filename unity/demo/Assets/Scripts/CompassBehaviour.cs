using UnityEngine;
using System.Collections;

public class CompassBehaviour : MonoBehaviour
{
    public Texture BackgroundTexture;
    public Texture PointerTexture;

    private Transform _target;

    void Start()
    {
        _target = gameObject.transform;
    }

    void OnGUI()
    {
        GUI.DrawTexture(new Rect(0, 0, 120, 120), BackgroundTexture);
        GUI.DrawTexture(CreateBlip(), PointerTexture);
    }

    private Rect CreateBlip()
    {
        float angDeg = _target.eulerAngles.y - 90;
        float angRed = angDeg * Mathf.Deg2Rad;

        float blipX = 25 * Mathf.Cos(angRed);
        float blipY = 25 * Mathf.Sin(angRed);

        blipX += 55;
        blipY += 55;

        return new Rect(blipX, blipY, 10, 10);
    }
}
