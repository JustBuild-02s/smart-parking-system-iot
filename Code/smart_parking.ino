#include <Wire.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

// ---------- WIFI ----------
const char* ssid = "AndroidAP";
const char* password = "@mrjana78";

// ---------- THINGSPEAK ----------
unsigned long channelID = 3246265;
const char* writeAPIKey = "F8B1879LXDYDIU3C";
WiFiClient client;

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- ULTRASONIC ----------
#define S1_TRIG 14
#define S1_ECHO 27

#define S2_TRIG 26
#define S2_ECHO 25

#define S3_TRIG 33
#define S3_ECHO 32

#define ENTRY_TRIG 13
#define ENTRY_ECHO 12

// ---------- OTHERS ----------
#define SERVO_PIN 4
#define GREEN_LED 16
#define RED_LED   17

#define SERVO_OPEN  90
#define SERVO_CLOSE 30

Servo gateServo;
bool gateOpen = false;

// ---------- DISTANCE FUNCTION ----------
long readDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration == 0) return 100;
  return duration * 0.034 / 2;
}

void setup() {
  Serial.begin(9600);
  Serial.println("SMART PARKING SYSTEM STARTED");

  // Ultrasonic pins
  pinMode(S1_TRIG, OUTPUT); pinMode(S1_ECHO, INPUT);
  pinMode(S2_TRIG, OUTPUT); pinMode(S2_ECHO, INPUT);
  pinMode(S3_TRIG, OUTPUT); pinMode(S3_ECHO, INPUT);
  pinMode(ENTRY_TRIG, OUTPUT); pinMode(ENTRY_ECHO, INPUT);

  // LEDs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // Servo
  gateServo.attach(SERVO_PIN);
  gateServo.write(SERVO_CLOSE);

  // OLED
  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED NOT FOUND");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("SMART PARKING");
  display.display();

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  ThingSpeak.begin(client);
}

void loop() {

  long d1 = readDistance(S1_TRIG, S1_ECHO);
  long d2 = readDistance(S2_TRIG, S2_ECHO);
  long d3 = readDistance(S3_TRIG, S3_ECHO);
  long entryDist = readDistance(ENTRY_TRIG, ENTRY_ECHO);

  int slot1 = (d1 < 10);
  int slot2 = (d2 < 10);
  int slot3 = (d3 < 10);

  int freeSlots = 3 - (slot1 + slot2 + slot3);

  // ---------- SERIAL ----------
  Serial.println("---------------------");
  Serial.print("Slot1: "); Serial.println(slot1 ? "FULL" : "EMPTY");
  Serial.print("Slot2: "); Serial.println(slot2 ? "FULL" : "EMPTY");
  Serial.print("Slot3: "); Serial.println(slot3 ? "FULL" : "EMPTY");
  Serial.print("Free Slots: "); Serial.println(freeSlots);

  // ---------- LED ----------
  if (freeSlots == 0) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
  } else {
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
  }

  // ---------- SERVO ----------
  if (entryDist < 15 && freeSlots > 0 && !gateOpen) {
    Serial.println("Gate OPEN");
    gateServo.write(SERVO_OPEN);
    gateOpen = true;
    delay(3000);
  }

  if (gateOpen && entryDist > 20) {
    Serial.println("Gate CLOSE");
    gateServo.write(SERVO_CLOSE);
    gateOpen = false;
  }

  // ---------- OLED ----------
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("SMART PARKING");

  display.setCursor(0, 15);
  display.print("S1: "); display.println(slot1 ? "FULL" : "EMPTY");

  display.setCursor(64, 15);
  display.print("S2: "); display.println(slot2 ? "FULL" : "EMPTY");

  display.setCursor(0, 30);
  display.print("S3: "); display.println(slot3 ? "FULL" : "EMPTY");

  display.setCursor(0, 50);
  display.print("Free: "); display.println(freeSlots);

  display.display();

  // ---------- THINGSPEAK ----------
  ThingSpeak.setF
