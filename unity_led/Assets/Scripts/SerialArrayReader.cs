using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO.Ports;
using System;
using System.Text;

public class SerialArrayReader : MonoBehaviour
{
    public Color[] Colors { get; private set; }
    public bool IsInit { get; private set; }
    public Action<Color> OnColorChanged { get; set; }
    public Action<string> OnSerialMessage { get; set; }

    public string portName = "COM3";
    public int baudRate = 115200;
    public int arraySize = 300;
    public bool hasOnlyBrightness;

    private SerialPort serialPort;
    private int bytesPerColor = 3;
    private int totalBytes;
    private List<byte> buffer;
    private double lastUpdateTime;
    private Color color = new Color(0f, 1f, 0f);

    private void Awake()
    {
        bytesPerColor = hasOnlyBrightness ? 1 : 3;
        totalBytes = arraySize * bytesPerColor;
        buffer = new List<byte>(totalBytes);
        Colors = new Color[arraySize];
    }

    private void Start()
    {
        foreach (string str in SerialPort.GetPortNames())
        {
            Debug.Log(string.Format("Existing COM port: {0}", str));
        }

        OpenConnection();
    }

    private void OnDestroy()
    {
        Disconnect();
    }

    private void Update()
    {
        if (Input.GetKeyDown(KeyCode.R))
        {
            OpenConnection();
        }

        if (Input.GetKeyDown(KeyCode.D))
        {
            Disconnect();
        }
    }

    private IEnumerator ReadBytesUntilNewline()
    {
        while (true)
        {
            if (serialPort != null && serialPort.IsOpen)
            {
                while (serialPort.BytesToRead > 0)
                {
                    byte b = (byte)serialPort.ReadByte();
                    buffer.Add(b);

                    if (buffer.Count >= 2 && buffer[buffer.Count - 2] == '\r' && buffer[buffer.Count - 1] == '\n')
                    {
                        // Remove the \r\n from the buffer
                        buffer.RemoveAt(buffer.Count - 1);
                        buffer.RemoveAt(buffer.Count - 1);

                        // Process the complete message
                        //string message = System.Text.Encoding.ASCII.GetString(buffer.ToArray());
                        //Debug.Log("Received message: " + message);

                        ParseArray(buffer);

                        buffer.Clear();
                        //yield return null;
                    }
                }
            }

            yield return null;
        }
    }

    public void OpenConnection()
    {
        StopAllCoroutines();
        
        try
        {
            Disconnect();

            Debug.Log($"Connecting to {portName} - {baudRate}");
            serialPort = new SerialPort(portName, baudRate);
            serialPort.Open();
            Debug.Log("Connected to " + portName);

            StartCoroutine(ReadBytesUntilNewline());
        }
        catch (Exception e)
        {
            Debug.LogError("Failed to open serial port: " + e.Message);
        }
    }

    public void Disconnect()
    {
        if (serialPort != null && serialPort.IsOpen)
        {
            serialPort.Dispose();
            serialPort.Close();
            Debug.Log("Serial port closed");
        }
    }

    void ParseArray(List<byte> byteArray)
    {
        if (byteArray.Count == 4
            && byteArray[0] == '#')
        {
            ParseColorChange(byteArray);
            return;
        }

        // Let's assume 30 is max length of a message.
        if (byteArray.Count <= 30)
        {
            var message = Encoding.ASCII.GetString(buffer.ToArray());
            OnSerialMessage?.Invoke(message);
            return;
        }

        if (byteArray.Count < totalBytes) return;

        Debug.Log($"Update duration={Time.unscaledTimeAsDouble - lastUpdateTime:N3}");
        lastUpdateTime = Time.unscaledTimeAsDouble;

        try
        {
            if (hasOnlyBrightness)
            {
                for (int i = 0; i < arraySize; i++)
                {
                    float value = byteArray[i] / 255f;

                    Colors[i] = color * value;
                }
            }
            else
            {
                for (int i = 0; i < arraySize; i++)
                {
                    byte red = byteArray[i * bytesPerColor];
                    byte green = byteArray[i * bytesPerColor + 1];
                    byte blue = byteArray[i * bytesPerColor + 2];

                    //Debug.Log($"{i}={red},{green},{blue}");

                    Colors[i] = new Color(red / 255f, green / 255f, blue / 255f);
                }
            }

            //Debug.Log("Received array: " + string.Join(", ", colors));
        }
        catch (Exception e)
        {
            Debug.LogError("Failed to parse array: " + e.Message);
        }
    }

    private void ParseColorChange(List<byte> byteArray)
    {
        byte red = byteArray[1];
        byte green = byteArray[2];
        byte blue = byteArray[3];

        //Debug.Log($"{i}={red},{green},{blue}");

        color = new Color(red / 255f, green / 255f, blue / 255f);
        OnColorChanged?.Invoke(color);
    }
}
