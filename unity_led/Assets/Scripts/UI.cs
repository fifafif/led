using System.Collections.Generic;
using System.Text.RegularExpressions;
using TMPro;
using UnityEngine;

public class UI : MonoBehaviour
{
    [SerializeField] private TextMeshProUGUI messageLogText;

    private SerialArrayReader serial;
    private List<string> messages;
    private int maxMessageCount = 5;

    private void Awake()
    {
        serial = FindObjectOfType<SerialArrayReader>();
        serial.OnSerialMessage += OnSerialMessage;
        messages = new List<string>(maxMessageCount);
    }

    private void OnSerialMessage(string message)
    {
        if (!ContainsLetterOrNumber(message)) return;

        messages.Add($"{Time.realtimeSinceStartup:N3}: {message}\n");
        if (messages.Count > maxMessageCount)
        {
            messages.RemoveAt(0);
        }

        messageLogText.text = "";
        for (int i = 0; i < messages.Count; i++)
        {
            messageLogText.text += messages[i];
        } 
    }

    public static bool ContainsLetterOrNumber(string input)
    {
        // Define the regex pattern to match any letter (a-z, A-Z) or number (0-9)
        string pattern = @"[a-zA-Z0-9]";
        // Create a regex object with the pattern
        Regex regex = new Regex(pattern);
        // Check if the input string matches the pattern
        return regex.IsMatch(input);
    }
}
