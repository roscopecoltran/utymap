using System;
using System.Collections.Generic;
using System.Linq;
using Assets.Scripts.UI;
using UnityEngine;
using UtyMap.Unity;
using UtyMap.Unity.Utils;

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
            // TODO should be in sync with sphere size and offsetted polygons
            sphereText.Radius = 6371 + 15;

            var font = new FontWrapper(element.Styles);
            sphereText.font = font.Font;
            sphereText.fontSize = font.Size;
            sphereText.color = font.Color;
            sphereText.text = GetText(element);
            sphereText.alignment = TextAnchor.MiddleCenter;
           
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

        private struct FontWrapper
        {
            public readonly Font Font;
            public readonly int Size;
            public readonly Color Color;

            public FontWrapper(Dictionary<string, string> styles)
            {
                Size = int.Parse(styles["font-size"]);
                Font = UnityEngine.Font.CreateDynamicFontFromOSFont(styles["font-name"], Size);
                Color = ColorUtils.FromUnknown(styles["font-color"]);
            }
        }
    }
}
