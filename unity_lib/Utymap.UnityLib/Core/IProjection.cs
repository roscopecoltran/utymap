using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;

namespace Utymap.UnityLib.Core
{
    public interface IProjection
    {
        Vector3 Project(GeoCoordinate coordinate, double height);
    }
}
