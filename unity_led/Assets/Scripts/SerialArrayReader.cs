using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO.Ports;
using System;
using System.Linq;

public class SerialArrayReader : MonoBehaviour
{
    public string portName = "COM3"; // Set this to your Arduino's port
    public int baudRate = 9600;
    public int arraySize = 300;

    private SerialPort serialPort;
    private int[] receivedArray;
    public Color[] colors;
    private int bytesPerColor = 3;
    private int totalBytes;
    private List<byte> buffer;
    private double lastUpdateTime;

    private void Awake()
    {
        totalBytes = arraySize * bytesPerColor;
        buffer = new List<byte>(totalBytes);
    }

    private void Start()
    {
        OpenConnection();
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

                        // Clear the buffer for the next message
                        buffer.Clear();

                        yield return null;
                    }
                }
            }

            yield return null;
        }
    }

    void OpenConnection()
    {
        try
        {
            serialPort = new SerialPort(portName, baudRate);
            serialPort.Open();
            Debug.Log("Connected to " + portName);

            StartCoroutine(ReadBytesUntilNewline());
        }
        catch (System.Exception e)
        {
            Debug.LogError("Failed to open serial port: " + e.Message);
        }
    }

    void OnDestroy()
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
        //Debug.Log(byteArray.Count);
        if (byteArray.Count < totalBytes) return;

        Debug.Log($"Colors received! update duration={Time.unscaledTimeAsDouble - lastUpdateTime:N3}");
        lastUpdateTime = Time.unscaledTimeAsDouble;

        try
        {
            colors = new Color[arraySize];
            receivedArray = new int[arraySize];
            for (int i = 0; i < arraySize; i++)
            {
                int red = (int)byteArray[i * bytesPerColor];
                int green = (int)byteArray[i * bytesPerColor + 1];
                int blue = (int)byteArray[i * bytesPerColor + 2];

                //Debug.Log($"{i}={red},{green},{blue}");

                colors[i] = new Color(red / 255f, green / 255f, blue / 255f);
            }

            //Debug.Log("Received array: " + string.Join(", ", colors));
        }
        catch (Exception e)
        {
            Debug.LogError("Failed to parse array: " + e.Message);
        }
    }
}
