#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include "KameRobot.h"
#include "web_page.h"

namespace {
constexpr char AP_SSID[] = "Kame-Remix";
constexpr char AP_PASSWORD[] = "a1b2c3d4";
constexpr uint16_t HTTP_PORT = 80;
constexpr uint16_t COMMAND_TIMEOUT_MS = 700;

IPAddress localIp(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(HTTP_PORT);
KameRobot robot;

uint32_t lastDriveCommandAt = 0;

String jsonState() {
    String json = "{";
    json += "\"mode\":\"";
    json += robot.modeName();
    json += "\",\"speed\":";
    json += robot.speed();
    json += ",\"ip\":\"";
    json += WiFi.softAPIP().toString();
    json += "\"}";
    return json;
}

DriveMode parseDriveMode(const String& command) {
    if (command == "forward") return DriveMode::Forward;
    if (command == "backward") return DriveMode::Backward;
    if (command == "left") return DriveMode::Left;
    if (command == "right") return DriveMode::Right;
    return DriveMode::Stop;
}

ActionMode parseActionMode(const String& name) {
    if (name == "hello") return ActionMode::Hello;
    if (name == "dance") return ActionMode::Dance;
    if (name == "pushup") return ActionMode::PushUp;
    if (name == "moonwalk") return ActionMode::Moonwalk;
    return ActionMode::None;
}

void sendJson(uint16_t code, const String& json) {
    server.sendHeader("Cache-Control", "no-store");
    server.send(code, "application/json", json);
}

void handleRoot() {
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleState() {
    sendJson(200, jsonState());
}

void handleDrive() {
    const String command = server.arg("cmd");
    const uint8_t speed = constrain(server.arg("speed").toInt(), 20, 100);
    const DriveMode mode = parseDriveMode(command);

    if (mode == DriveMode::Stop) {
        robot.stop();
    } else {
        robot.drive(mode, speed);
        lastDriveCommandAt = millis();
    }

    sendJson(200, jsonState());
}

void handleAction() {
    const String name = server.arg("name");

    if (name == "home") {
        robot.home();
    } else {
        const ActionMode action = parseActionMode(name);
        if (action == ActionMode::None) {
            sendJson(400, "{\"error\":\"unknown action\"}");
            return;
        }
        robot.action(action);
    }

    sendJson(200, jsonState());
}

void handleNotFound() {
    sendJson(404, "{\"error\":\"not found\"}");
}

void startAccessPoint() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(localIp, gateway, subnet);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
}

void bindRoutes() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/api/state", HTTP_GET, handleState);
    server.on("/api/drive", HTTP_POST, handleDrive);
    server.on("/api/action", HTTP_POST, handleAction);
    server.onNotFound(handleNotFound);
}
}

void setup() {
    Serial.begin(115200);
    delay(100);

    robot.begin();
    startAccessPoint();
    bindRoutes();
    server.begin();

    Serial.println();
    Serial.println("Kame Remix AP started");
    Serial.print("SSID: ");
    Serial.println(AP_SSID);
    Serial.print("URL: http://");
    Serial.println(WiFi.softAPIP());
}

void loop() {
    server.handleClient();

    if (robot.driveMode() != DriveMode::Stop &&
        millis() - lastDriveCommandAt > COMMAND_TIMEOUT_MS) {
        robot.stop();
    }

    robot.tick();
    yield();
}
