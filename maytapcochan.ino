// Chân 11: nút nhấn, Chân 12: buzzer, Chân 13: LED
const int buttonPin = 11;
const int buzzerPin = 12;
const int ledPin = 13;

// Timer: 30 phút, LED Program 1: 5s, LED Program 2: 25s
const unsigned long PROGRAM_TIMEOUT = 1800000UL;      // 30 phút = 1800000ms
const unsigned long LED_INTERVAL1 = 5000UL;           // Program 1: 5s
const unsigned long LED_INTERVAL2 = 25000UL;          // Program 2: 25s

// Trạng thái chương trình: 0 = dừng, 1 = chương trình 1, 2 = chương trình 2
int programState = 0;

// Biến cho phát hiện nút nhấn
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long buttonPressTime = 0;
const unsigned long debounceDelay = 50;

// Biến cho phát hiện nhấn nhiều lần
int clickCount = 0;
unsigned long lastClickTime = 0;
const unsigned long doubleClickThreshold = 500;  
const unsigned long holdThreshold = 2000;  

// Biến cho chương trình 1
unsigned long program1StartTime = 0;
unsigned long ledLastChangeTime1 = 0;
bool ledState1 = true;  
bool program1Started = false;

// Biến cho chương trình 2
unsigned long program2StartTime = 0;
unsigned long ledLastChangeTime2 = 0;
bool ledState2 = true;  
bool program2Started = false;

void setup() {
  Serial.begin(9600);
  
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledPin, LOW);
  
  Serial.println("=== Arduino Started ===");
  Serial.print("PROGRAM_TIMEOUT: ");
  Serial.print(PROGRAM_TIMEOUT);
  Serial.println(" ms");
  Serial.print("LED_INTERVAL1: ");
  Serial.print(LED_INTERVAL1);
  Serial.println(" ms");
  Serial.print("LED_INTERVAL2: ");
  Serial.print(LED_INTERVAL2);
  Serial.println(" ms");
}

void loop() {
  handleButton();
  
  if (programState == 1) {
    runProgram1();
  } else if (programState == 2) {
    runProgram2();
  } else {
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
  }
}

// Xử lý nút nhấn
void handleButton() {
  int reading = digitalRead(buttonPin);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      
      // Nút được ấn (từ HIGH -> LOW)
      if (buttonState == LOW) {
        buttonPressTime = millis();
        Serial.println("[BTN] PRESSED");
      }
      
      // Nút được thả (từ LOW -> HIGH)
      if (buttonState == HIGH) {
        unsigned long pressDuration = millis() - buttonPressTime;
        Serial.print("[BTN] RELEASED, ");
        Serial.print(pressDuration);
        Serial.println("ms");
        
        // Kiểm tra ấn giữ (> 2 giây)
        if (pressDuration > holdThreshold) {
          Serial.println("[HOLD] Stop all");
          buzzerBeep(3, 300, 200);
          programState = 0;
          program2Started = false;
          program1Started = false;
          clickCount = 0;
        } else {
          Serial.print("[CLICK] Count: ");
          clickCount++;
          Serial.println(clickCount);
          lastClickTime = millis();
        }
      }
    }
  }
  
  lastButtonState = reading;
  
  // Kiểm tra timeout cho double-click
  if (clickCount > 0 && (millis() - lastClickTime) > doubleClickThreshold) {
    if (clickCount == 1) {
      Serial.println("[PROGRAM] Starting Program 1 (5s LED, 30min)");
      buzzerBeep(1, 1000, 0);
      programState = 1;
      program2Started = false;
      program1Started = true;
      ledState1 = true;
      digitalWrite(ledPin, HIGH);
      ledLastChangeTime1 = millis();
      program1StartTime = millis();
    } else if (clickCount >= 2) {
      Serial.println("[PROGRAM] Starting Program 2 (25s LED, 30min)");
      buzzerBeep(2, 500, 300);
      programState = 2;
      program2StartTime = millis();
      program2Started = true;
      ledState2 = true;
      digitalWrite(ledPin, HIGH);
      ledLastChangeTime2 = millis();
    }
    clickCount = 0;
  }
}

// Hàm kêu buzzer: beepCount = số lần, duration = thời gian kêu, gap = khoảng cách
void buzzerBeep(int beepCount, unsigned long duration, unsigned long gap) {
  for (int i = 0; i < beepCount; i++) {
    digitalWrite(buzzerPin, HIGH);
    delay(duration);
    digitalWrite(buzzerPin, LOW);
    if (i < beepCount - 1) {
      delay(gap);
    }
  }
}

// Chương trình 1: LED sáng 5s → tắt 5s, 30 phút timeout
void runProgram1() {
  if (!program1Started) return;
  
  unsigned long currentTime = millis();
  unsigned long elapsed = currentTime - program1StartTime;
  
  // Timeout 30 phút
  if (elapsed >= PROGRAM_TIMEOUT) {
    Serial.print("[P1] TIMEOUT at ");
    Serial.print(elapsed / 1000);
    Serial.println("s - Beeping 6x");
    
    for (int i = 0; i < 6; i++) {
      digitalWrite(buzzerPin, HIGH);
      delay(500);
      digitalWrite(buzzerPin, LOW);
      delay(500);
    }
    Serial.println("[P1] STOPPED");
    programState = 0;
    program1Started = false;
    digitalWrite(ledPin, LOW);
    return;
  }
  
  // LED nhấp nháy 5s
  if (currentTime - ledLastChangeTime1 >= LED_INTERVAL1) {
    ledState1 = !ledState1;
    digitalWrite(ledPin, ledState1 ? HIGH : LOW);
    Serial.print("[P1] LED ");
    Serial.print(ledState1 ? "ON" : "OFF");
    Serial.print(" (");
    Serial.print(elapsed / 1000);
    Serial.println("s)");
    ledLastChangeTime1 = currentTime;
  }
}

// Chương trình 2: LED sáng 25s → tắt 25s, 30 phút timeout
void runProgram2() {
  if (!program2Started) return;
  
  unsigned long currentTime = millis();
  unsigned long elapsed = currentTime - program2StartTime;
  
  // Timeout 30 phút
  if (elapsed >= PROGRAM_TIMEOUT) {
    Serial.print("[P2] TIMEOUT at ");
    Serial.print(elapsed / 1000);
    Serial.println("s - Beeping 6x");
    
    for (int i = 0; i < 6; i++) {
      digitalWrite(buzzerPin, HIGH);
      delay(500);
      digitalWrite(buzzerPin, LOW);
      delay(500);
    }
    Serial.println("[P2] STOPPED");
    programState = 0;
    program2Started = false;
    digitalWrite(ledPin, LOW);
    return;
  }
  
  // LED nhấp nháy 25s
  if (currentTime - ledLastChangeTime2 >= LED_INTERVAL2) {
    ledState2 = !ledState2;
    digitalWrite(ledPin, ledState2 ? HIGH : LOW);
    Serial.print("[P2] LED ");
    Serial.print(ledState2 ? "ON" : "OFF");
    Serial.print(" (");
    Serial.print(elapsed / 1000);
    Serial.println("s)");
    ledLastChangeTime2 = currentTime;
  }
}
