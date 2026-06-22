#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

#include "KameRobot.h"
#include "calibration_page.h"
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
    json += (int)robot.speed();
    json += ",\"gait\":\"";
    json += robot.gaitName();
    json += "\"";
    json += ",\"ip\":\"";
    json += WiFi.softAPIP().toString();
    json += "\"}";
    return json;
}

String calibrationJson(const String& extra = "") {
    String json = "{";
    if (extra.length() > 0) {
        json += extra;
        json += ",";
    }

    json += "\"loaded\":";
    json += robot.calibrationLoaded() ? "true" : "false";
    json += ",\"mode\":\"";
    json += robot.modeName();
    json += "\",\"servos\":[";

    for (uint8_t i = 0; i < KameRobot::ServoCount; i++) {
        if (i > 0) {
            json += ",";
        }
        json += "{\"id\":";
        json += (int)i;
        json += ",\"trim\":";
        json += (int)robot.trim(i);
        json += ",\"reversed\":";
        json += robot.reversed(i) ? "true" : "false";
        json += "}";
    }

    json += "]}";
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
    if (name == "bow") return ActionMode::Bow;
    if (name == "wiggle") return ActionMode::Wiggle;
    if (name == "stretch") return ActionMode::Stretch;
    if (name == "patrol") return ActionMode::Patrol;
    if (name == "showtime") return ActionMode::ShowOff;
    return ActionMode::None;
}

GaitStyle parseGaitStyle(const String& name) {
    if (name == "sneak") return GaitStyle::Sneak;
    if (name == "bounce") return GaitStyle::Bounce;
    return GaitStyle::Normal;
}

void sendJson(uint16_t code, const String& json) {
    server.sendHeader("Cache-Control", "no-store");
    server.send(code, "application/json", json);
}

void handleRoot() {
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleCalibrationPage() {
    robot.enterCalibration();
    server.send_P(200, "text/html; charset=utf-8", CALIBRATION_HTML);
}

void handleState() {
    sendJson(200, jsonState());
}

void handleCalibrationState() {
    robot.enterCalibration();
    sendJson(200, calibrationJson());
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

void handleGait() {
    robot.setGaitStyle(parseGaitStyle(server.arg("style")));
    sendJson(200, jsonState());
}

void handleCalibrationSet() {
    const uint8_t id = constrain(server.arg("id").toInt(), 0, KameRobot::ServoCount - 1);
    const int8_t trim = constrain(server.arg("trim").toInt(), -30, 30);
    const String reverseArg = server.arg("reverse");
    const bool reversed = reverseArg == "1" || reverseArg == "true";

    robot.enterCalibration();
    robot.setTrim(id, trim);
    robot.setReversed(id, reversed);
    sendJson(200, calibrationJson());
}

void handleCalibrationTest() {
    const uint8_t id = constrain(server.arg("id").toInt(), 0, KameRobot::ServoCount - 1);
    const float angle = constrain(server.arg("angle").toFloat(), 45.0f, 135.0f);

    robot.enterCalibration();
    robot.setCalibrationServo(id, angle);
    sendJson(200, calibrationJson());
}

void handleCalibrationNeutral() {
    robot.calibrationNeutral();
    sendJson(200, calibrationJson());
}

void handleCalibrationSave() {
    robot.enterCalibration();
    const bool saved = robot.saveCalibration();
    sendJson(200, calibrationJson(saved ? "\"saved\":true" : "\"saved\":false"));
}

void handleCalibrationReset() {
    robot.resetCalibration();
    sendJson(200, calibrationJson("\"reset\":true"));
}

void handleCalibrationExit() {
    robot.exitCalibration();
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
    server.on("/calibration", HTTP_GET, handleCalibrationPage);
    server.on("/api/state", HTTP_GET, handleState);
    server.on("/api/drive", HTTP_POST, handleDrive);
    server.on("/api/action", HTTP_POST, handleAction);
    server.on("/api/gait", HTTP_POST, handleGait);
    server.on("/api/calibration/state", HTTP_GET, handleCalibrationState);
    server.on("/api/calibration/set", HTTP_POST, handleCalibrationSet);
    server.on("/api/calibration/test", HTTP_POST, handleCalibrationTest);
    server.on("/api/calibration/neutral", HTTP_POST, handleCalibrationNeutral);
    server.on("/api/calibration/save", HTTP_POST, handleCalibrationSave);
    server.on("/api/calibration/reset", HTTP_POST, handleCalibrationReset);
    server.on("/api/calibration/exit", HTTP_POST, handleCalibrationExit);
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
