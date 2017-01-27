using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UtyMap.Unity;

namespace Assets.Scripts.UI
{
    [ExecuteInEditMode]
    public class SphereText : Text
    {
        public float Radius = 100.5f;
        public GeoCoordinate Coordinate;

        protected override void OnPopulateMesh(VertexHelper vh)
        {
            base.OnPopulateMesh(vh);

            var vertices = new List<UIVertex>();
            vh.GetUIVertexStream(vertices);

            var circumference = 2.0f / 360.0f * Mathf.PI * Radius;
            for (var i = 0; i < vertices.Count; i++)
            {
                var v = vertices[i];

                var xPercentCircumference = v.position.x / circumference;
                var yPercentCircumference = v.position.y / circumference;
                var zOffset = Quaternion.Euler(0, -xPercentCircumference, 0) * Vector3.forward;
                var yOffset = Quaternion.Euler(-yPercentCircumference, 0.0f, 0.0f) * Vector3.up;
                v.position = yOffset * v.position.y + zOffset * Radius;

                vertices[i] = v;
            }

            rectTransform.Rotate(new Vector3( (float)-Coordinate.Latitude, (float) -Coordinate.Longitude, 0));

            vh.AddUIVertexTriangleStream(vertices);
        }
    }
}
