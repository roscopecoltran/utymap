using System;
using System.Collections.Generic;
using UnityEngine;

namespace UtyMap.Unity.Explorer.Customization
{
    /// <summary> Maintains list of customization properties. </summary>
    public sealed class CustomizationService
    {
        private readonly Dictionary<string, Type> _modelBehaviours;
        private readonly Dictionary<string, Material> _sharedMaterials;

        /// <summary> Creates instance of <see cref="CustomizationService"/>. </summary>
        public CustomizationService()
        {
            _sharedMaterials = new Dictionary<string, Material>();
        }

        #region Materials

        /// <summary> Gets shared material by key. </summary>
        /// <remarks> Should be called from UI thread only. </remarks>
        public Material GetSharedMaterial(string key)
        {
            if (!_sharedMaterials.ContainsKey(key))
                _sharedMaterials[key] = Resources.Load<Material>(key);

            return _sharedMaterials[key];
        }

        #endregion
    }
}
