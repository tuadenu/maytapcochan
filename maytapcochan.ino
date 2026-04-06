// Chân 11: nút nhấn, Chân 12: buzzer, Chân 13: LED
const int buttonPin = 11;
const int buzzerPin = 12;
const int ledPin = 13;

// Trạng thái chương trình: 0 = dừng, 1 = chương trình 1 (30 phút kêu), 2 = chương trình 2 (LED nhấp nháy)
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
const unsigned long doubleClickThreshold = 500;  // 500ms cho double-click
const unsigned long holdThreshold = 2000;  // 2 giây để phát hiện ấn giữ

// Biến cho chương trình 1 (30 phút)
unsigned long lastBuzzerTime1 = 0;
const unsigned long interval1 = 30 * 60 * 1000;  // 30 phút

// Biến cho chương trình 2 (LED nhấp nháy + 30 phút dừng)
unsigned long ledLastChangeTime2 = 0;
const unsigned long ledInterval2 = 20 * 1000;  // 20 giây
bool ledState2 = true;  // true = sáng, false = tắt
unsigned long program2StartTime = 0;
const unsigned long program2Duration = 30 * 60 * 1000;  // 30 phút

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  digitalWrite(buzzerPin, LOW);
  digitalWrite(ledPin, LOW);
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
      }
      
      // Nút được thả (từ LOW -> HIGH)
      if (buttonState == HIGH) {
        unsigned long pressDuration = millis() - buttonPressTime;
        
        // Kiểm tra ấn giữ (> 2 giây)
        if (pressDuration > holdThreshold) {
          // Ấn giữ → kêu 3 lần tút tút → dừng tất cả
          buzzerBeep(3, 300, 200);
          programState = 0;
          clickCount = 0;
        } else {
          // Ấn ngắn → tính số lần ấn
          clickCount++;
          lastClickTime = millis();
          
          // Nếu là lần ấn thứ 1
          if (clickCount == 1) {
            // Chờ xem có ấn lần 2 không (trong 500ms)
          }
        }
      }
    }
  }
  
  lastButtonState = reading;
  
  // Kiểm tra timeout cho double-click
  if (clickCount > 0 && (millis() - lastClickTime) > doubleClickThreshold) {
    if (clickCount == 1) {
      // Single click → kêu 1 tiếng dài 2s → chạy program 1
      buzzerBeep(1, 2000, 0);
      programState = 1;
      lastBuzzerTime1 = millis();
    } else if (clickCount == 2) {
      // Double click → kêu 2 tiếng → chạy program 2
      buzzerBeep(2, 500, 300);
      programState = 2;
      program2StartTime = millis();
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

// Chương trình 1: 30 phút, sau đó buzzer kêu 6 tiếng
void runProgram1() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastBuzzerTime1 >= interval1) {
    // Kêu buzzer 6 tiếng
    for (int i = 0; i < 6; i++) {
      digitalWrite(buzzerPin, HIGH);
      delay(500);
      digitalWrite(buzzerPin, LOW);
      delay(500);
    }
    programState = 0;  // Dừng lại
    lastBuzzerTime1 = currentTime;
  }
}

// Chương trình 2: LED nhấp nháy 20s sáng/20s tắt, 30 phút sau dừng và kêu 6 tiếng
void runProgram2() {
  unsigned long currentTime = millis();
  
  // Kiểm tra nếu đã qua 30 phút
  if (currentTime - program2StartTime >= program2Duration) {
    // Kêu buzzer 6 tiếng
    for (int i = 0; i < 6; i++) {
      digitalWrite(buzzerPin, HIGH);
      delay(500);
      digitalWrite(buzzerPin, LOW);
      delay(500);
    }
    programState = 0;  // Dừng tất cả
    digitalWrite(ledPin, LOW);
    return;
  }
  
  // LED nhấp nháy 20s sáng/20s tắt
  if (currentTime - ledLastChangeTime2 >= ledInterval2) {
    ledState2 = !ledState2;
    digitalWrite(ledPin, ledState2 ? HIGH : LOW);
    ledLastChangeTime2 = currentTime;
  }
}
