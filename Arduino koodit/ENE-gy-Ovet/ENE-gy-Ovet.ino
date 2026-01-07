#include <Servo.h>
#include "DFRobot_RGBLCD1602.h"
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <avr/wdt.h>
SoftwareSerial link(10,11);

// --- Password / UI state ---
bool passwordRequired = false;
String password = "2025"; 
String inputCode = "";

// --- LEDs ---
const int green = 12;
const int red = 13;

// --- LCD setup ---
const int colorR = 255;
const int colorG = 255;
const int colorB = 255;
DFRobot_RGBLCD1602 lcd(/*RGBAddr*/0x60 ,/*lcdCols*/16,/*lcdRows*/2);

// --- Servo & Sensor ---
Servo myServo;
Servo doorServo;
const int CLOSED_ANGLE = -2;
const int OPEN_ANGLE   = 60;

#define SERVO_PIN 9
#define SERVOD_PIN 8
#define SENSOR_PIN A0       // Analog sensor pin
#define MAX_RANG 520        // cm
#define ADC_SOLUTION 1023.0 // 10-bit ADC

// --- Buzzer/Speaker ---
#define BUZZER_PIN 5
int melody[] = {262, 294, 330, 349, 392, 440, 494, 523}; // Do-Re-Mi-Fa-So-La-Si-Do
int noteDuration = 400; // milliseconds

// --- Watchdog ---
#define WATCHDOG_TIMEOUT WDTO_8S


// --- LOCK-KEYPAD ---
int wrongAttempts = 0;
bool keypadLocked = false;
unsigned long lockoutUntil = 0;
const unsigned long LOCKOUT_MS = 60000;  // 1 minute


// --- Servo sweep via Timer2 ISR ---
volatile int angle = 0;
volatile int step = 2;     // degrees per tick
volatile byte tickCount = 0;
unsigned long passwordStartTime = 0;


// --- Helper functions ---
float getDistance() {
  float sensorValue = analogRead(SENSOR_PIN);
  float distance = sensorValue * MAX_RANG / ADC_SOLUTION;
  if (distance > MAX_RANG) distance = MAX_RANG;
  return distance;
}

void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    wdt_reset();
    delay(10);
  }
}


// --- Melody helper ---
// void beep(int duration) {
//   digitalWrite(BUZZER_PIN, HIGH);
//   delay(duration);
//   digitalWrite(BUZZER_PIN, LOW);
// }


void playTone(int freq, int duration) {
  long period = 1000000L / freq;
  long cycles = (long)duration * 1000L / period;
  for (long i = 0; i < cycles; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(period / 2);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(period / 2);
  }
}

void playMelody() {
  for (int i = 0; i < 8; i++) {
    wdt_reset();
    playTone(melody[i], noteDuration);
    delay(50);
  }
}

void playSuccessMelody() {
  int notes[] = {1046, 1318, 1567, 2093, 1567};  
  int count = sizeof(notes) / sizeof(notes[0]);

  for (int i = 0; i < count; i++) {
    playTone(notes[i], 180);
    delay(20);
  }
}

void playlockWarningMelody() {
  // Rising alarm sequence
  playTone(500, 150);
  delay(180);
  playTone(600, 150);
  delay(180);
  playTone(700, 150);
  delay(180);
  playTone(900, 200);
  delay(250);

  // Falling melodic tones
  playTone(750, 180);
  delay(200);
  playTone(620, 180);
  delay(200);
  playTone(480, 250);
  delay(300);

  // Deep final lockout tone
  playTone(300, 600);
  delay(650);
}

void playTimeoutMelody() {
  // Descending tones
  playTone(800, 150);  // First tone
  delay(100);
  playTone(600, 150);  // Second tone
  delay(100);
  playTone(400, 250);  // Final tone
  delay(200);
}


// --- EEPROM helpers ---
void loadPassword() {
  char stored[10] = {0};
  for (int i = 0; i < 10; i++) {
    char c = EEPROM.read(i);
    if (c == '\0' || c == 0xFF) break; // Stop at null or empty
    stored[i] = c;
  }
  password = String(stored);
  if (password.length() == 0) password = "2025"; // Default
}

void savePassword(String newPass) {
  for (int i = 0; i < newPass.length(); i++) {
    EEPROM.write(i, newPass[i]);
  }
  EEPROM.write(newPass.length(), '\0');
}


void openDoorSequenceBlocking() {
  // Open door
  doorServo.write(OPEN_ANGLE);
  unsigned long start = millis();
  safeDelay(500); 
  lcd.clear();
  lcd.print("Door Ready");
  // Wait 10 seconds
  safeDelay(10000);
  lcd.clear();
  lcd.print("Door Close");
  // Reattach (if detached) and close
  if (!doorServo.attached()) {
    doorServo.attach(SERVOD_PIN);
  }
  doorServo.write(CLOSED_ANGLE);
  delay(500);
} 

// --- Process one key character from Nano #2 ---
void processKeyChar(char key) {
  if(keypadLocked) return;
  if (key == '#') {
    // Confirm code
    inputCode.trim();
    if (inputCode == password) {    
      keypadLocked = false;
      wrongAttempts = 0; 
      digitalWrite(red, !digitalRead(red)); 
      digitalWrite(green, HIGH);
      lcd.clear();
      lcd.print("Access Granted");
      lcd.setRGB(0, 255, 0);
      playMelody();
      openDoorSequenceBlocking();
      safeDelay(3000); // UI pause
      // Ask if user wants to change password
      lcd.clear();
      lcd.print("Change Pass?");
      lcd.setCursor(0, 1);
      lcd.print("Type ** or #");

      String choice = "";
        unsigned long startTime = millis();
        while (millis() - startTime < 10000) { // 10s window
          if (link.available()) {
            char key = (char)link.read();
            if (key == '\n' || key == '\r') continue;
            choice += key;
            if (choice.endsWith("**")) break; // Change password
            if (choice.endsWith("#")) break;  // Skip
          }
        }

        if (choice.endsWith("**")) {
        // Change password mode
        lcd.clear();
        lcd.print("New Code:");
        String newPass = "";
        while (true) {
          if (link.available()) {
            char k = (char)link.read();
            if (k == '#') break; // Confirm new password
            newPass += k;
            lcd.clear();
            lcd.print("New: ");
            lcd.print(newPass);
          }
        }

        if (newPass.length() > 0) {
          savePassword(newPass);
          password = newPass; // Update in RAM
          lcd.clear();
          lcd.print("Saved!");
          playSuccessMelody();
          safeDelay(2000);
        }
      } else {
        wrongAttempts = 0;
        keypadLocked = false;
        lcd.clear();
        lcd.print("No Change");
        safeDelay(1000);
      }
      passwordRequired = false;
      myServo.attach(SERVO_PIN); // resume servo motion
      digitalWrite(green, LOW);
      lcd.setRGB(255, 255, 255);
    } else {
       // Check if we reached 3 wrong attempts
      if (wrongAttempts >= 2) {
        keypadLocked = true;
        lockoutUntil = millis() + LOCKOUT_MS;
        wrongAttempts = 0;

        digitalWrite(red, HIGH);
        lcd.setRGB(255, 0, 0);
        lcd.clear();
        lcd.print("Too many wrong attempts");
        lcd.setCursor(0, 1);
        lcd.print("Doorpad is closed for 1 minute");

        // Optional alert pattern before reset
        playlockWarningMelody();
      }
      wrongAttempts++;
      lcd.clear();
      lcd.print("Wrong Code");
      lcd.setCursor(0, 1);
      lcd.print("Try Again!");
      // Triple beep for wrong code
      playTone(200, 200);
      delay(100);
      playTone(200, 200);
      delay(100);
      playTone(200, 200);
    }
    inputCode = "";
  } else if (key == '*') {
    inputCode = "";
    lcd.clear();
    lcd.print("Enter Code:");
  } else {
    // Append any other key
    inputCode += key;
    lcd.clear();
    lcd.print("Code: ");
    lcd.print(inputCode);
  }
}

// --- Timer2 Compare Match ISR (non-blocking sweep) ---
// CTC mode, OCR2A chosen for ~5ms tick, accumulate to ~50ms per angle step
ISR(TIMER2_COMPA_vect) {
  // Only sweep when not in password mode
  if (!passwordRequired) {
    tickCount++;
    if (tickCount >= 10) {     // 10 * ~5ms = ~50ms step
      tickCount = 0;
      angle += step;
      if (angle >= 180 || angle <= 0) step = -step;
    }
  } 
}


void setup() {
  // WATCHDOG safety
  MCUSR = 0;          
  wdt_disable();      
  wdt_enable(WDTO_8S);


  // LEDs
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(green, LOW);
  digitalWrite(red, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // UART (Nano-to-Nano)
  Serial.begin(9600); // For debugging
  link.begin(9600); // SoftwareSerial for Nano-to-Nano

  // LCD
  lcd.init();
  lcd.setRGB(colorR, colorG, colorB);
  lcd.clear();
  lcd.print("ENE-gy Ovet");

  // Servo
  myServo.attach(SERVO_PIN);
  doorServo.attach(SERVOD_PIN);
  doorServo.write(CLOSED_ANGLE);



  loadPassword(); // Load password from EEPROM
  // savePassword("2025");
  // --- Timer2 in CTC mode ---
  // 16MHz / 1024 prescaler => 15625 Hz timer tick
  // OCR2A=78 => (78+1)/15625 ≈ 0.005s = ~5ms
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2A |= (1 << WGM21);                           // CTC mode
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);// Prescaler 1024
  OCR2A = 78;                                       // ~5ms interval
  TIMSK2 |= (1 << OCIE2A);                          // Enable Compare A interrupt
  sei();
  
}



void loop() {
  wdt_reset();              // Normal kick → resets counter to 0
  float distance = getDistance();

  // If someone is close and we aren’t already in password mode
  if (distance < 10 && !passwordRequired) {
    passwordRequired = true;
    passwordStartTime = millis(); // Start timer
    myServo.detach();             // stop servo motion to keep door locked
    lcd.setRGB(255, 0, 0);
    lcd.clear();
    lcd.print("Enter Code:");
    digitalWrite(red, HIGH);
    playTone(400, 200); // Short beep for object detection
  }

  // Handle keypad input coming from Nano #2 over UART
  if (passwordRequired) {
    if (keypadLocked) {
      // Stay locked until time is up
      if (millis() >= lockoutUntil) {
        keypadLocked = false;
        wrongAttempts = 0;          
        digitalWrite(red, LOW);
        lcd.setRGB(255, 255, 255);
        lcd.clear();
        lcd.print("Lockout over"); 
        safeDelay(1000);
        lcd.clear();
        lcd.print("Enter Code:");
        passwordStartTime = millis(); 
      } else {
        // Show countdown
        while (millis() < lockoutUntil) {
          wdt_reset();
          unsigned long remaining = (lockoutUntil - millis() + 999) / 1000;
          lcd.clear();
          lcd.print("Keypad locked");    
          lcd.setCursor(0, 1);
          lcd.print(remaining);
          lcd.print(" s");
          delay(200);
        }
      }
      return; 
    }

    while (link.available() > 0) {
      char key = (char)link.read();
      if (key == '\n' || key == '\r') continue;
      processKeyChar(key);
      passwordStartTime = millis(); // Reset timer on input
    }

    if (millis() - passwordStartTime > 7000) { // 7 seconds
      passwordRequired = false;
      myServo.attach(SERVO_PIN); // Resume servo motion
      digitalWrite(red, LOW);
      lcd.setRGB(255, 255, 255);
      lcd.clear();
      lcd.print("Timeout!");
      playTimeoutMelody();
      safeDelay(2000);
      keypadLocked = false;
      wrongAttempts = 0; 
    }

    while (link.available() > 0) {
      char key = (char)link.read();
      // Optional: skip delimiters like '\n' or '\r'
      if (key == '\n' || key == '\r') continue;
      processKeyChar(key);
      passwordStartTime = millis(); // Reset timer on input
    }
  } else {
    // Normal sweep display & servo update
    myServo.write(angle);

    // Status to Serial Monitor for debugging
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    Serial.print("Angle: ");
    Serial.print(angle);
    Serial.println(" astetta");

    // LCD screen
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Angle:");
    lcd.print(angle);
    lcd.setCursor(0, 1);
    lcd.print("Dist:");
    lcd.print(distance, 0);
    lcd.print(" cm");
  }

}
