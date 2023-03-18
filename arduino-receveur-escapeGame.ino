#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "Font_Data.h"
#include <LiquidCrystal_I2C.h>

// définition des paramètres de la matrice 32x8
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CLK_PIN 13
#define DATA_PIN 11
#define CS_PIN 10

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

#define SPEED_TIME 75
#define PAUSE_TIME 0
#define MAX_MESG 20
#define DEBUG 0

char szTime[4];
int servo = A1;
int buzzer = A0;
unsigned long startTime;

bool countdown = false;


LiquidCrystal_I2C lcd(0x27, 16, 2);

void getTime(char *psz, bool f = true) {
  int total_seconds = 60 * 60;
  int elapsed_seconds = (millis() / 1000) % total_seconds;  // On calcule le nombre de secondes écoulées depuis le début en prenant le reste de la division de millis() par 1000 par le nombre total de secondes

  int remaining_seconds = total_seconds - elapsed_seconds;  // On calcule le nombre de secondes restantes en soustrayant les secondes écoulées du nombre total de secondes
  int minutes = remaining_seconds / 60;                     // On calcule le nombre de minutes restantes en divisant le nombre de secondes restantes par 60
  int seconds = remaining_seconds % 60;                     // On calcule le nombre de secondes restantes en prenant le reste de la division du nombre de secondes restantes par 60

  sprintf(psz, "%02d%c%02d", minutes, (f ? ':' : ' '), seconds);  // On utilise sprintf pour formatter les minutes et les secondes
}

// fonction buzzer si le code entré est mauvais
void wrongcode() {
  tone(buzzer, 500);  // allume le buzzer actif arduino
  delay(400);
  tone(buzzer, 800);  // allume le buzzer actif arduino
  delay(400);
  tone(buzzer, 600);  // allume le buzzer actif arduino
  delay(400);
  tone(buzzer, 900);  // allume le buzzer actif arduino
  delay(400);
  noTone(buzzer);  // désactiver le buzzer actif arduino
  delay(500);
}

// fonction buzzer si le code entré est bon
void goodcode() {
  tone(buzzer, 440);  // allume le buzzer actif arduino
  delay(300);
  tone(buzzer, 466);  // allume le buzzer actif arduino
  delay(300);
  tone(buzzer, 494);  // allume le buzzer actif arduino
  delay(300);
  tone(buzzer, 523);  // allume le buzzer actif arduino
  delay(600);
  noTone(buzzer);  // désactiver le buzzer actif arduino
  delay(500);
}

// fonction buzzer si la bombe est désamorcée
void wingame() {
  tone(buzzer, 784);
  delay(200);
  tone(buzzer, 740);
  delay(200);
  tone(buzzer, 622);
  delay(200);
  tone(buzzer, 440);
  delay(200);
  tone(buzzer, 415);
  delay(200);
  tone(buzzer, 659);
  delay(200);
  tone(buzzer, 831);
  delay(200);
  tone(buzzer, 2093);
  delay(200);
  noTone(buzzer);
}

// fonction buzzer si le compteur arrive à zéro
void losegame() {
  tone(buzzer, 1100);
  delay(600);
  noTone(buzzer);
  delay(500);
  tone(buzzer, 1100);
  delay(500);
  noTone(buzzer);
  delay(500);
  tone(buzzer, 1000);
  delay(400);
  noTone(buzzer);
  delay(500);
  tone(buzzer, 1000);
  delay(300);
  noTone(buzzer);
  delay(500);
  tone(buzzer, 900);
  delay(200);
  noTone(buzzer);
  delay(500);
  tone(buzzer, 900);
  delay(300);
  noTone(buzzer);
  delay(500);
}

// fonction pour le servo moteur
void penDash(int x) {
  int kyori = (x * 10.25) + 450;
  digitalWrite(servo, HIGH);
  delayMicroseconds(kyori);
  digitalWrite(servo, LOW);
  delay(5);
}

void setup() {
  Serial.begin(9600);

  startTime = millis();
  // initialise le servo moteur, le bouton de désactivation et le buzzer
  pinMode(servo, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Initialise l'écran LCD avec le premier message à afficher
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Entrer code :");

  // initialise la matrice et les paramètres d'affichage
  P.begin(2);
  P.setZone(0, 0, MAX_DEVICES - 5);
  P.setZone(1, MAX_DEVICES - 4, MAX_DEVICES - 1);
  P.setFont(1, numeric7Seg);
  P.setInvert(false);
  P.displayZoneText(1, szTime, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  getTime(szTime);  // Récupère le temps actuel et le stocke dans szTime
}

void loop() {
  static uint32_t lastTime = 0;
  static bool flasher = false;

  String receivedData = Serial.readString();

  if (receivedData == "goodCode") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Code correct");
    goodcode();
  }

  if (receivedData == "wrongCode") {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Code incorrect");
    wrongcode();
  }

  if (receivedData == "winGame") {
    wingame();
    countdown = true;
  }

  if (receivedData == "servoZero") {
    penDash(0);
  }

  if (receivedData == "servoNine") {
    penDash(90);
  }

  if (countdown) {
    P.getPause();
  } else {
    getTime(szTime);
    P.displayZoneText(1, szTime, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
    if (millis() - startTime >= 3600000) {
      losegame();
      while (1) {}
    }
  }

  P.displayAnimate();
  P.displayReset(0);
}
