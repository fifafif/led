using System;
using UnityEngine;

public class LEDPlane : MonoBehaviour
{
    [SerializeField] private SerialArrayReader serialArrayReader;
    [SerializeField] private Texture2D ledColors;

    private void Awake()
    {
        serialArrayReader.OnColorChanged += OnColorChanged;
    }

    private void OnColorChanged(Color color)
    {
    }

    private void Update()
    {
        for (int i = 0; i < serialArrayReader.Colors.Length; i++)
        {
            ledColors.SetPixel(i, 0, serialArrayReader.Colors[i]);
        }

        ledColors.Apply();
    }
}
