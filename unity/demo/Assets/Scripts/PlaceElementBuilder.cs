using System;
using System.Linq;
using UnityEngine;
using UtyMap.Unity.Core.Models;
using UtyMap.Unity.Core.Tiling;
using UtyMap.Unity.Explorer.Customization;

namespace Assets.Scripts
{
    //  Builds Place of Interest from Element
    public class PlaceElementBuilder
    {
        private readonly CustomizationService _customizationService;

        public PlaceElementBuilder(CustomizationService customizationService)
        {
            _customizationService = customizationService;
        }

        public GameObject Build(Tile tile, Element element)
        {
            GameObject gameObject = GameObject.CreatePrimitive(PrimitiveType.Cube);
            gameObject.name = GetName(element);
            
            var transform = gameObject.transform;
            transform.position = tile.Projection.Project(element.Geometry[0], 
                GetMinHeight(element) + element.Heights[0]);
            transform.localScale = new Vector3(2, 2, 2);

            gameObject.GetComponent<MeshFilter>().mesh.uv = GetUV(element);
            gameObject.GetComponent<MeshRenderer>().sharedMaterial = GetMaterial(element);

            return gameObject;
        }

        private Vector2[] GetUV(Element element)
        {
            Rect rect = GetUvRect(element);

            var p0 = new Vector2(rect.xMin, rect.yMin);
            var p1 = new Vector2(rect.xMax, rect.yMin);
            var p2 = new Vector2(rect.xMin, rect.yMax);
            var p3 = new Vector2(rect.xMax, rect.yMax);

            // Imagine looking at the front of the cube, the first 4 vertices are arranged like so
            //   2 --- 3
            //   |     |
            //   |     |
            //   0 --- 1
            // then the UV's are mapped as follows
            //    2    3    0    1   Front
            //    6    7   10   11   Back
            //   19   17   16   18   Left
            //   23   21   20   22   Right
            //    4    5    8    9   Top
            //   15   13   12   14   Bottom
            return new[]
            {
                p0, p1, p2, p3,
                p2, p3, p2, p3,
                p0, p1, p0, p1,
                p0, p3, p1, p2,
                p0, p3, p1, p2,
                p0, p3, p1, p2
            };
        }

        private string GetName(Element element)
        {
            return String.Format("place:{0}[{1}]", element.Id, 
                element.Tags.Aggregate("", (s, t) => s+=String.Format("{0}={1},", t.Key, t.Value)));
        }

        private float GetMinHeight(Element element)
        {
            return element.Styles.ContainsKey("min-height")
                ? float.Parse(element.Styles["min-height"])
                : 0;
        }

        private Material GetMaterial(Element element)
        {
            return _customizationService.GetSharedMaterial("Materials/" + element.Styles["material"]);
        }

        private Rect GetUvRect(Element element)
        {
            var values = element.Styles["rect"].Split('_');
            if (values.Length != 4)
                throw new InvalidOperationException("Cannot read uv mapping.");

            var textureHeight = float.Parse(element.Styles["height"]);
            var textureWidth = float.Parse(element.Styles["width"]);

            var width = (float)int.Parse(values[2]);
            var height = (float)int.Parse(values[3]);

            var offset = int.Parse(values[1]);
            var x = (float)int.Parse(values[0]);
            var y = Math.Abs((offset + height) - textureHeight);

            var leftBottom = new Vector2(x / textureWidth, y / textureHeight);
            var rightUpper = new Vector2((x + width) / textureWidth, (y + height) / textureHeight);

            return new Rect(leftBottom.x, leftBottom.y, rightUpper.x - leftBottom.x, rightUpper.y - leftBottom.y);
        }
    }
}
