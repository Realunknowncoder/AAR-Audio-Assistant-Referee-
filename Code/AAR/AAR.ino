#include <SD.h>
#include <TMRpcm.h>
#include <pcmConfig.h>
#include <pcmRF.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


#define SD_ChipSelectPin 10

TMRpcm audio;
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Buttons
const int recordBtn = 3;
const int playBtn = 5;
const int deleteBtn = 2;
const int forwardBtn = 6;
const int backwardBtn = 4;

const int micPin = A0;

// State (RENAMED HERE)
bool isRecording = false;
bool isRecorded = false;
bool isPaused = false;

char file_name[] = "REC.wav";

unsigned long playStart = 0;
unsigned long pauseStart = 0;

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  if (!SD.begin(SD_ChipSelectPin)) {
    lcd.print("SD Fail");
    while (1);
  }

  audio.CSPin = SD_ChipSelectPin;
  audio.setVolume(5);

  pinMode(recordBtn, INPUT_PULLUP);
  pinMode(playBtn, INPUT_PULLUP);
  pinMode(deleteBtn, INPUT_PULLUP);
  pinMode(forwardBtn, INPUT_PULLUP);
  pinMode(backwardBtn, INPUT_PULLUP);

  lcd.print("Ready");
}

void loop() {

  // ---------- RECORD ----------
  if (!digitalRead(recordBtn)) {
    delay(200);

    if (!isRecorded) {
      if (!isRecording) {
        lcd.clear();
        lcd.print("Recording...");
        audio.startRecording(file_name, 8000, micPin);
        isRecording = true;
      } else {
        audio.stopRecording(file_name);
        isRecording = false;
        isRecorded = true;

        lcd.clear();
        lcd.print("Saved");
      }
    }
  }

  // ---------- PLAY / PAUSE ----------
  if (!digitalRead(playBtn)) {
    delay(200);

    if (isRecorded) {
      if (!audio.isPlaying()) {
        audio.play(file_name);
        playStart = millis();
        isPaused = false;

        lcd.clear();
        lcd.print("Playing");
      } else {
        audio.pause();

        if (!isPaused) {
          pauseStart = millis();
          isPaused = true;
          lcd.setCursor(0,2);
          lcd.print("Paused       ");
        } else {
          playStart += millis() - pauseStart;
          isPaused = false;
          lcd.setCursor(0,2);
          lcd.print("Playing      ");
        }
      }
    }
  }

  // ---------- DELETE ----------
  if (!digitalRead(deleteBtn)) {
    delay(200);

    if (isRecorded) {
      SD.remove(file_name);
      isRecorded = false;

      lcd.clear();
      lcd.print("Deleted");
    }
  }

  // ---------- FORWARD ----------
  if (!digitalRead(forwardBtn)) {
    delay(200);
    skipAudio(5);
  }

  // ---------- BACKWARD ----------
  if (!digitalRead(backwardBtn)) {
    delay(200);
    skipAudio(-5);
  }

  // ---------- TIME DISPLAY ----------
  if (audio.isPlaying() && !isPaused) {
    unsigned long sec = (millis() - playStart) / 1000;
    int m = sec / 60;
    int s = sec % 60;

    lcd.setCursor(0,3);
    lcd.print("Time: ");

    if (m < 10) lcd.print("0");
    lcd.print(m);
    lcd.print(":");

    if (s < 10) lcd.print("0");
    lcd.print(s);
    lcd.print("   ");
  }
}

// ---------- SKIP FUNCTION ----------
void skipAudio(int seconds) {
  if (!audio.isPlaying()) return;

  audio.stopPlayback();

  long current = (millis() - playStart) / 1000;
  current += seconds;
  if (current < 0) current = 0;

  audio.play(file_name);
  playStart = millis() - (current * 1000);

  lcd.setCursor(0,2);
  if (seconds > 0) {
    lcd.print(">> Forward   ");
  } else {
    lcd.print("<< Back      ");
  }
}