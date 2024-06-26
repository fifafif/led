using System;
using TMPro;
using UnityEngine;

[RequireComponent(typeof(TextMeshProUGUI))]
public class DebugPrint : MonoBehaviour
{
    private static DebugPrint instance;
    
    private TextMeshProUGUI text;

    private void Awake()
    {
        instance = this;
        text = GetComponent<TextMeshProUGUI>();

        Application.logMessageReceived += LogMessageReceived;
    }

    private void OnDestroy()
    {
        Application.logMessageReceived -= LogMessageReceived;
    }

    private void LogMessageReceived(string condition, string stackTrace, LogType type)
    {
        if (type == LogType.Error)
        {
            LogError(condition);
        }
        else
        {
            Log(condition);
        }
    }

    public static void Log(string msg)
    {
        if (instance == null) return;

        instance.text.text = msg;
    }

    public static void LogError(string msg)
    {
        if (instance == null) return;

        instance.text.text = $"<color=red>{msg}</color>";
    }

}
