#include <FastLED.h>

#define DIODE_1 13        // Пін для білого діода
#define DIODE_2 11        // Пін для червоного діода
#define LED_PIN 12        // Пін, на який підключена світлодіодна стрічка
#define NUM_LEDS 246      // Кількість світлодіодів у стрічці
#define BUTTON_PIN 2     // Пін, на який підключена сенсорна кнопка
#define BRIGHTNESS  50     // Яскравість: 5-255
#define CYCLE_DELAY 20   // Затримка між миганням в 1-шому режимі
#define MAX_SIMULTANEOUS_LEDS 45 // Максимальна к-сть світодіодів включкних одночасно в 1-шому режимі 
#define LED_TYPE WS2812B 


boolean buttonPressed = false; // Змінна для відстеження стану кнопки
boolean systemOn = true; // Стан системи при включенні 
unsigned long buttonPressStartTime = 0;
int mode = 0;  // Початковий ефект
int MODE_AMOUNT = 3; // Кількість ефектів

// Індикація в салоні
const long LONG_PRESS_DURATION = 1000; // Час який потрібно тримати кнопку для виключення системи 
const unsigned long interval = 150; // Інтервал між миганнями в мілісекундах
unsigned long previousMillis = 0;  // Зберігає останній час мигання світлодіоду
bool ledState = false;  // Поточний стан світлодіоду (ввімкнено/вимкнено)

CRGB leds[NUM_LEDS];


void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(DIODE_1, OUTPUT);
  pinMode(DIODE_2, OUTPUT);

  FastLED.addLeds<LED_TYPE, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  randomSeed(analogRead(A0)); // Seed the random number generator with an analog reading
}

void loop() {
  boolean touchState = digitalRead(BUTTON_PIN);

  if (touchState == HIGH && !buttonPressed) {
    buttonPressed = true;
    buttonPressStartTime = millis(); // Запам'ятовуємо час початку натискання кнопки
  } else if (touchState == LOW && buttonPressed) {
    buttonPressed = false;
    unsigned long buttonPressDuration = millis() - buttonPressStartTime;

    if (buttonPressDuration >= LONG_PRESS_DURATION) {
      // Довгий натиск
      systemOn = !systemOn; // Зміна стану системи (увімкнення/вимкнення)
      if (systemOn) {
        Serial.println("Систему увімкнено");
      } else {
        Serial.println("Систему вимкнено");
        FastLED.clear(); // Очистити всі світлодіоди на ленті
        FastLED.show();  // Оновити ленту
      }
    } else {
      // Короткий натиск
      Serial.println("Тач-кнопку натиснуто!");
      if (++mode >= MODE_AMOUNT){
      mode = 0;
    }
    }
  }

  if (systemOn) {
    switchEffect(mode);
  }
}

void switchEffect(int mode) {
  // Застосування обраного ефекту
  switch (mode) {
    case 0:
      digitalWrite(DIODE_2, LOW);
      diode();
      randomLEDs();
      break;
    case 1:
      digitalWrite(DIODE_1, LOW);
      digitalWrite(DIODE_2, HIGH);
      policeFlashing();
      break;
    case 2:
      digitalWrite(DIODE_2, LOW);
      digitalWrite(DIODE_1, HIGH);
      progressFill();
      break;
  }
}

void diode(){
  digitalWrite(DIODE_2, LOW);
  unsigned long currentMillis = millis(); // Поточний час

  // Перевіряємо, чи пройшов інтервал для мигання
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Запам'ятовуємо час останнього мигання

    // Змінюємо стан світлодіоду
    if (ledState) {
      ledState = false;
    } else {
      ledState = true;
    }

    digitalWrite(DIODE_1, ledState); // Встановлюємо стан світлодіоду
  }
}


void randomLEDs(){
  int indices[NUM_LEDS];
  int numSimultaneous = random(1, MAX_SIMULTANEOUS_LEDS + 1);
  for (int i = 0; i < NUM_LEDS; i++) {
    indices[i] = i;
  }
  for (int i = NUM_LEDS - 1; i > 0; i--) {
    int j = random(i + 1);
    int temp = indices[i];
    indices[i] = indices[j];
    indices[j] = temp;
  }
   for (int i = 0; i < NUM_LEDS; i++) {
    if (i < numSimultaneous) {
      leds[indices[i]] = CRGB::White; // Turn on LEDs
    } else {
      leds[indices[i]] = CRGB::Black; // Turn off LEDs
    }
  }
  delay(CYCLE_DELAY);
  FastLED.show();
}


void policeFlashing() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < NUM_LEDS / 2) {
      leds[i] = CRGB::Blue;  // First half is blue
    } else {
      leds[i] = CRGB::Black; // Second half is dark
    }
  }
  FastLED.show();
  delay(100); // Adjust flashing speed
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < NUM_LEDS / 2) {
      leds[i] = CRGB::Black; // First half becomes dark
    } else {
      leds[i] = CRGB::Red;   // Second half becomes red
    }
  }
  FastLED.show();
  delay(100); // Adjust flashing speed
  
}

void meteorRain(int ledPerMillis, int meteorSize, CRGB color, int wait) {
  static byte meteorTrail[NUM_LEDS];
  static long lastTime = 0;

  if (millis() - lastTime > ledPerMillis) {
    lastTime = millis();

    // Зміщення всіх пікселів вгору
    for (int i = 0; i < NUM_LEDS - 1; i++) {
      leds[i] = leds[i + 1];
      if (meteorTrail[i] > 0) meteorTrail[i]--; // Зменшення "світлового хвоста" метеора
    }

    // Генерація нового метеора на нижньому ряді
    if (random(255) < 255 - meteorSize) {
      leds[NUM_LEDS - 1] = color;
      meteorTrail[NUM_LEDS - 1] = meteorSize;
    }

    delay(wait);
    FastLED.show();
  }
}

void progressFill() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
  }
  delay(50);
  FastLED.show();
}

void staticEffect() {
  // Налаштуйте кольори, які ви хочете використовувати.
  CRGB color = CRGB(255, 255, 255); // Наприклад, червоний колір (RGB)

  // Пройдемося по всіх світлодіодах і встановимо їх бажаним кольором.
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }

  // Оновити стрічку, щоб показати встановлені кольори.
  FastLED.show();
}
