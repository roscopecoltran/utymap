using System;
using UnityEngine;
using UnityEngine.UI;
using UtyMap.Unity.Infrastructure.Primitives;

namespace Assets.Scripts.Menu
{
    public class RangeBehaviour : MonoBehaviour
    {
        public Slider MinSlider;
        public Slider MaxSlider;

        public Text MinText;
        public Text MaxText;

        private Range<int> _range;

        void Start () 
        {
            _range = new Range<int>((int)MinSlider.minValue, (int) MinSlider.maxValue);

            MinSlider.onValueChanged.AddListener(OnValueChanged);
            MaxSlider.onValueChanged.AddListener(OnValueChanged);

            OnValueChanged(0);
        }

        void OnValueChanged(float value)
        {
            var minValueOrig = MinSlider.value;

            if (MinSlider.value > MaxSlider.value)
                MinSlider.value = Mathf.Max(_range.Minimum, MinSlider.value - 1);

            if (MaxSlider.value < minValueOrig)
                MaxSlider.value = Mathf.Min(_range.Maximum, MaxSlider.value + 1);

            MinText.text = String.Format("Min: {0}", MinSlider.value);
            MaxText.text = String.Format("Max: {0}", MaxSlider.value);
        }

    }
}
