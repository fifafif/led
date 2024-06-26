using UnityEngine;

public class TexturePrinter : MonoBehaviour
{
    [SerializeField] private Texture2D ledColors;

    private void Start()
    {
        CreateColors();
    }

    public void CreateColors()
    {
        for (int i = 0; i < 256; i++)
        {
            ledColors.SetPixel(i, 0, rgbFromWheel(i));
        }

        ledColors.Apply();

#if UNITY_EDITOR
        UnityEditor.EditorUtility.SetDirty(ledColors);
#endif
    }

    Color rgbFromWheel(int WheelPos)
    {
        int redValue, greenValue, blueValue;

        WheelPos = 255 - WheelPos;
        if (WheelPos < 85)
        {
            redValue = 255 - WheelPos * 3;
            greenValue = 0;
            blueValue = WheelPos * 3;
        }
        else if (WheelPos < 170)
        {
            WheelPos -= 85;
            redValue = 0;
            greenValue = WheelPos * 3;
            blueValue = 255 - WheelPos * 3;
        }
        else
        {
            WheelPos -= 170;
            redValue = WheelPos * 3;
            greenValue = 255 - WheelPos;
            blueValue = 0;
        }

        return new Color(redValue / 255f, greenValue / 255f, blueValue / 255f, 1f);
    }

}
