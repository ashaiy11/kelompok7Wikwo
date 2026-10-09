
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1
);

const int buttons[4] = {13, 12, 14, 27};
const int leds[4] = {18, 19, 23, 25};
const int buzzer = 26;

const char* colors[4] = {
  "RED", "BLUE", "GREEN", "YELLOW"
};

const int tones[4] = {262, 330, 392, 523};

int sequence[100];
int sequenceLength = 0;
int score = 0;
int level = 0;
int inputIndex = 0;

bool started = false;
bool gameOver = false;

void sound(int frequency, int duration) {
  tone(buzzer, frequency, duration);
  delay(duration);
  noTone(buzzer);
}

void centerText(String text, int y, int size) {
  display.setTextSize(size);
  display.setTextColor(SSD1306_WHITE);

  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(
    text, 0, y, &x1, &y1, &w, &h
  );

  display.setCursor((128 - w) / 2, y);
  display.print(text);
}

void showMessage(
  String title, String line1, String line2
) {
  display.clearDisplay();

  centerText(title, 3, 2);
  display.drawLine(0, 24, 127, 24, SSD1306_WHITE);
  centerText(line1, 32, 1);
  centerText(line2, 46, 1);

  display.display();
}

void showMenu() {
  display.clearDisplay();

  centerText("BRAIN BLITZ", 3, 2);
  display.drawRoundRect(
    4, 27, 120, 32, 5, SSD1306_WHITE
  );
  centerText("MEMORY GAME", 31, 1);
  centerText("PRESS ANY BUTTON", 45, 1);

  display.display();
}

int readButton() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttons[i]) == LOW) {
      delay(25);

      if (digitalRead(buttons[i]) == LOW) {
        while (digitalRead(buttons[i]) == LOW) {
          delay(5);
        }

        return i;
      }
    }
  }

  return -1;
}

void playColor(int color, int duration) {
  digitalWrite(leds[color], HIGH);

  display.clearDisplay();
  centerText("REMEMBER!", 4, 1);

  display.drawRoundRect(
    8, 20, 112, 35, 6, SSD1306_WHITE
  );

  centerText(colors[color], 30, 2);
  display.display();

  sound(tones[color], duration);

  digitalWrite(leds[color], LOW);
  delay(150);
}

void showStats() {
  display.clearDisplay();

  centerText("YOUR TURN", 2, 2);
  display.drawLine(0, 23, 127, 23, SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(5, 31);
  display.print("LEVEL: ");
  display.print(level);

  display.setCursor(5, 42);
  display.print("SCORE: ");
  display.print(score);

  display.setCursor(5, 54);
  display.print("REPEAT THE COLORS");

  display.display();
}

void nextRound() {
  if (sequenceLength >= 100) {
    gameOver = true;
    showMessage("YOU WIN!", "MAX LEVEL!", "PRESS TO REPLAY");
    return;
  }

  sequence[sequenceLength] = random(0, 4);
  sequenceLength++;
  level = sequenceLength;
  inputIndex = 0;

  showMessage(
    "LEVEL " + String(level),
    "WATCH THE COLORS",
    "GET READY!"
  );

  sound(880, 100);
  delay(400);

  int duration = max(180, 550 - level * 20);

  for (int i = 0; i < sequenceLength; i++) {
    playColor(sequence[i], duration);
    delay(150);
  }

  showStats();
}

void loseGame() {
  gameOver = true;

  sound(180, 250);
  delay(100);
  sound(140, 350);

  display.clearDisplay();

  centerText("GAME OVER", 3, 2);
  display.drawLine(0, 25, 127, 25, SSD1306_WHITE);
  centerText("SCORE: " + String(score), 33, 1);
  centerText("PRESS TO RETRY", 47, 1);

  display.display();
}

void startGame() {
  score = 0;
  level = 0;
  sequenceLength = 0;
  inputIndex = 0;
  gameOver = false;
  started = true;

  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], LOW);
  }

  randomSeed(micros());

  sound(523, 100);
  sound(659, 100);
  sound(784, 150);

  nextRound();
}

void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(buttons[i], INPUT_PULLUP);
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }

  pinMode(buzzer, OUTPUT);

  Wire.begin(21, 22);

  if (!display.begin(
    SSD1306_SWITCHCAPVCC, 0x3C
  )) {
    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.display();

  randomSeed(micros());
  showMenu();
}

void loop() {
  int pressed = readButton();

  if (pressed == -1) {
    return;
  }

  if (!started || gameOver) {
    startGame();
    return;
  }

  digitalWrite(leds[pressed], HIGH);
  sound(tones[pressed], 100);
  digitalWrite(leds[pressed], LOW);

  if (pressed == sequence[inputIndex]) {
    inputIndex++;
    score += 10;

    if (inputIndex == sequenceLength) {
      inputIndex = 0;

      sound(784, 100);
      sound(988, 150);

      delay(300);
      nextRound();
    } else {
      showStats();
    }
  } else {
    loseGame();
  }
}