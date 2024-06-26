using System.Collections.Generic;
using UnityEngine;

public class LEDs : MonoBehaviour
{
    [SerializeField] private SerialArrayReader serialArrayReader;
    [SerializeField] private LED ledPrefab;
    [SerializeField] private float ledWidth = 1f;

    private List<LED> leds;

    private void Awake()
    {
        CreateLEDs();
    }

    private void CreateLEDs()
    {
        var count = serialArrayReader.arraySize;
        leds = new List<LED>(count);
        for (int i = 0; i < count; i++)
        {
            var led = Instantiate(ledPrefab, transform);
            led.transform.localPosition = new Vector3(i * ledWidth - count * ledWidth * 0.5f, 0f, 0f);
            leds.Add(led);
        }
        ledPrefab.gameObject.SetActive(false);
    }

    private void Update()
    {
        for (int i = 0; i < leds.Count && i < serialArrayReader.Colors.Length; i++)
        {
            leds[i].SetColor(serialArrayReader.Colors[i]);
        }
    }
}
