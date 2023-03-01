// implémentation des librairies

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "Font_Data.h"
#include <Keypad.h>
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

// définition des variables globales
char szTime[4];
const int ROW_NUM = 4;
const int COLUMN_NUM = 4;
bool foundCode = false;
int servo = A1;
unsigned long startTime;

char keys[ROW_NUM][COLUMN_NUM] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte pin_rows[ROW_NUM] = { 9, 8, 7, 6 };
byte pin_column[COLUMN_NUM] = { 5, 4, 3, 2 };

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// fonction pour compte à rebours
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
  tone(A0, 500);  // allume le buzzer actif arduino
  delay(400);
  tone(A0, 800);  // allume le buzzer actif arduino
  delay(400);
  tone(A0, 600);  // allume le buzzer actif arduino
  delay(400);
  tone(A0, 900);  // allume le buzzer actif arduino
  delay(400);
  noTone(A0);  // désactiver le buzzer actif arduino
  delay(500);
}

// fonction buzzer si le code entré est bon
void goodcode() {
  tone(A0, 440);  // allume le buzzer actif arduino
  delay(300);
  tone(A0, 466);  // allume le buzzer actif arduino
  delay(300);
  tone(A0, 494);  // allume le buzzer actif arduino
  delay(300);
  tone(A0, 523);  // allume le buzzer actif arduino
  delay(600);
  noTone(A0);  // désactiver le buzzer actif arduino
  delay(500);
}

// fonction buzzer si la bombe est désamorcée
void wingame() {
  tone(A0, 784);
  delay(200);
  tone(A0, 740);
  delay(200);
  tone(A0, 622);
  delay(200);
  tone(A0, 440);
  delay(200);
  tone(A0, 415);
  delay(200);
  tone(A0, 659);
  delay(200);
  tone(A0, 831);
  delay(200);
  tone(A0, 2093);
  delay(200);
  noTone(A0);
}

// fonction buzzer si le compteur arrive à zéro
void losegame() {
  tone(A0, 1100);
  delay(600);
  noTone(A0);
  delay(500);
  tone(A0, 1100);
  delay(500);
  noTone(A0);
  delay(500);
  tone(A0, 1000);
  delay(400);
  noTone(A0);
  delay(500);
  tone(A0, 1000);
  delay(300);
  noTone(A0);
  delay(500);
  tone(A0, 900);
  delay(200);
  noTone(A0); 
  delay(500);
  tone(A0, 900); 
  delay(300);
  noTone(A0);
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


void setup(void) {
  startTime = millis();
  // initialise le servo moteur, le bouton de désactivation et le buzzer
  pinMode(servo, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A2, INPUT_PULLUP);
  // Initialise l'écran LCD avec le premier message à afficher
  lcd.begin();
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

void loop(void) {
  static int count = 0;
  char code[5];
  char key = keypad.getKey();
  static uint32_t lastTime = 0;
  static bool flasher = false;
  P.displayAnimate();
  P.displayReset(0);
  // première condition si le code est trouvé
  if (foundCode) {
    // active le servo à 0
    penDash(0);
    // condition si bouton est pressé
    if (analogRead(A2) < 50) {
      wingame();
      while (1) {
        // fige l'ensemble des composants (matrice et lcd)
      }
    }
  } else {
    if (key) {
      penDash(90);
      lcd.setCursor(count, 1);
      lcd.print(key);
      code[count] = key;
      count++;
      if (count == 4) {  //vérifie que 4 chiffres sont entrés pour passer à l'étape suivante
        code[4] = '\0';
        if (strcmp(code, "1507") == 0) {  //vérifie si le code est similaire à 1507
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Code correct");
          goodcode();
          // change la variable à true
          foundCode = true;
        } else { //si le code n'est pas trouvé
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Code incorrect");
          wrongcode();
          count = 0;
        }
      }
    }
  }
  getTime(szTime);
  P.displayZoneText(1, szTime, PA_CENTER, SPEED_TIME, PAUSE_TIME, PA_PRINT, PA_NO_EFFECT);
  if (millis() - startTime >= 3600000) {
    losegame();
    while(1) {}
  }
}