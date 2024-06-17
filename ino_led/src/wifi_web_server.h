#ifndef WIFI_WEB_SERVER_H
#define WIFI_WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "strip_handler.h"
#include "playback.h"

const char* ssid = "ESP32-LED";
const char* password = "123456789";

WebServer server(80);

StripHandler *wifiStrip;
Playback *wifiPlayback;

void handleGetColor();
void handleSetColor();
void handleSetSpeed();
void handleSetBrightness();

void setupWifi(StripHandler *strip, Playback *playback)
{
    wifiStrip = strip;
    wifiPlayback = playback;

    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    server.on("/set-color", handleSetColor);
    server.on("/get-color", handleGetColor);
    server.on("/set-speed", handleSetSpeed);
    server.on("/set-brightness", handleSetBrightness);

    server.begin();
}

void loopWifi()
{
    server.handleClient();
}

void handleSetColor()
{
    if (!server.hasArg("hue")) 
    {
        server.send(400, "text/plain", "Bad Request: Missing hue parameter");
    }
    
    byte hueValue = (byte)server.arg("hue").toInt();

    wifiStrip->setRGBFromWheel(hueValue);
    String message = "Color set to: ";
    message += "Red: " + String(hueValue);
    server.send(200, "text/plain", message);

    Serial.println(message);
}

void handleGetColor()
{
    String hueValue = String(wifiStrip->colorWheelPosition);
    String message = "Get color: " + String(hueValue);
    server.send(200, "text/plain", message);

    Serial.println(message);
}

void handleSetSpeed()
{
    if (!server.hasArg("speed")) 
    {
        server.send(400, "text/plain", "Bad Request: Missing speed parameter");
    }
    
    byte speed = (byte)server.arg("speed").toInt();

    wifiPlayback->speed = speed;
    String message = "Speed set to: " + String(speed);
    server.send(200, "text/plain", message);

    Serial.println(message);
}

void handleSetBrightness()
{
    if (!server.hasArg("brightness")) 
    {
        server.send(400, "text/plain", "Bad Request: Missing brightness parameter");
    }
    
    byte brightness = (byte)server.arg("brightness").toInt();

    wifiStrip->brightness = brightness;
    String message = "Brightness set to: " + String(brightness);
    server.send(200, "text/plain", message);

    Serial.println(message);
}
#endif