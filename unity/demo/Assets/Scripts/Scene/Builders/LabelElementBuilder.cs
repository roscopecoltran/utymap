using System;
using System.Linq;
using Assets.Scripts.UI;
using UnityEngine;
using UtyMap.Unity;

namespace Assets.Scripts.Scene.Builders
{
    /// <summary> Builds a label. </summary>
    internal sealed class LabelElementBuilder : IElementBuilder
    {
        private GameObject _canvas;

        public LabelElementBuilder()
        {
            _canvas = GameObject.Find("Text Visualizer");
        }

        /// <inheritdoc />
        public GameObject Build(Tile tile, Element element)
        {
            var gameObject = new GameObject(GetName(element));
            var sphereText = gameObject.AddComponent<SphereText>();
            
            sphereText.Coordinate = element.Geometry[0];
            sphereText.Radius = 1001f; // TODO should be in sync with sphere size

            var font = GetFont(element);
            sphereText.font = font;
            sphereText.fontSize = int.Parse(element.Styles["font-size"]);
            sphereText.text = GetText(element);
            sphereText.alignment = TextAnchor.MiddleCenter;
            sphereText.color = Color.red;

            // NOTE should be attached to properly oriented canvas.
            gameObject.transform.SetParent(_canvas.transform);

            return gameObject;
        }

        private static Font GetFont(Element element)
        {
            var name = element.Styles["font-name"];
            var size = int.Parse(element.Styles["font-size"]);
            // TODO cache created font
            return Font.CreateDynamicFontFromOSFont(name, size);
        }

        private static string GetText(Element element)
        {
            return element.Tags["name"];
        }

        private static string GetName(Element element)
        {
            return String.Format("place:{0}[{1}]", element.Id,
                element.Tags.Aggregate("", (s, t) => s += String.Format("{0}={1},", t.Key, t.Value)));
        }
    }
}
