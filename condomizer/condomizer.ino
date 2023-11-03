#include <Stepper_28BYJ.h>
#include "FastLED.h"


// изменить количество шагов для вашего мотора
#define STEPS 4096

Stepper_28BYJ stepper1(STEPS, 0, 1, 2, 3);
Stepper_28BYJ stepper2(STEPS, 4, 5, 6, 7);

#define BUTTON       (8)
#define OUT_LED      (9)
#define BUTTON_RED   (10)
#define BUTTON_GREEN (11)

#define LED_COUNT 9

const int NUMBER = 25;
uint8_t OFFSET_COLLOR = 0;

static uint32_t timer = 0;
static bool out_led_status = 0;
static int counter = 0;
CRGB strip[LED_COUNT];
static CRGB color = CRGB::Green;

static bool FLAG_LGBT = false;

static uint32_t LAST_LGBT_TIME = 0;
static CRGB LGBT_COLOR[6] = {   CHSV(0, 255, 255),
                                CHSV(32, 255, 255),
                                CHSV(64, 255, 255),
                                CHSV(96, 255, 255),
                                CHSV(160, 255, 255),
                                CHSV(192, 255, 255) };


#define NUM_HISTORY 3
struct {
  uint8_t last_write;
  uint32_t array[NUM_HISTORY];
} HISTORY;

int8_t CalcIndex(uint8_t last_write_index, int8_t index) {  // "index" should be < 0
  index = last_write_index + index;
  if (index < 0) {
    index += NUM_HISTORY; // because overflow the value of index (case: index -1, last_write_index 0, so -1 will be max_uint8_t = 255)
  }
  return index;
}

uint32_t GetTimeInHistoryByIndex(int8_t index) {
  return HISTORY.array[CalcIndex(HISTORY.last_write, index)];
}

void WriteLastClickButtonAndCheck() {
  if (counter < 2) {
    return;
  }
  if (millis() - GetTimeInHistoryByIndex(-1) < 20000 &&
      millis() - GetTimeInHistoryByIndex(-2) < 20000) {
        FLAG_LGBT = true;
        LAST_LGBT_TIME = millis();
      }     
  HISTORY.last_write++;
  if (HISTORY.last_write == NUM_HISTORY) {
    HISTORY.last_write = 0;
  }
  HISTORY.array[HISTORY.last_write] = millis();
}

void setup()
{
  pinMode(BUTTON_RED, OUTPUT);
  pinMode(BUTTON_GREEN, OUTPUT);  
  pinMode(BUTTON, INPUT);

  stepper1.setSpeed(14);
  stepper2.setSpeed(14);
  FastLED.addLeds<WS2812B, OUT_LED, GRB>(strip, LED_COUNT);
}

CRGB GetColorNow() {
  uint8_t color_value = millis() / 30 + OFFSET_COLLOR;
  
  // Code below disable "blue"
  //         
  // if (color_value >= 105 && color_value < 145) {
  //   OFFSET_COLLOR += (145 - color_value);
  // }

  if (FLAG_LGBT) {
    if (millis() - LAST_LGBT_TIME > 15000) {
      FLAG_LGBT = false;
    }
    return LGBT_COLOR[map((millis() - LAST_LGBT_TIME) % 5000, 0, 5000, 0, 5)];
  }
  return CHSV(color_value, 255, 255);
}

bool GetStateButton() {
  if (digitalRead(BUTTON)) {
    WriteLastClickButtonAndCheck();
    return true;
  }
  return false;
}

void loop()
{
  if (counter < 2*NUMBER){
    digitalWrite(BUTTON_GREEN,HIGH);
    digitalWrite(BUTTON_RED,LOW);
    color = GetColorNow();
  }
  else{
    digitalWrite(BUTTON_RED,HIGH);
    digitalWrite(BUTTON_GREEN,LOW); 
    color = CRGB::Red;
  } 
  
  if (GetStateButton()){
    if (counter < 2*NUMBER){
      for (int i = 0; i< 8; i++){
        digitalWrite(BUTTON_RED,HIGH);
        digitalWrite(BUTTON_GREEN,HIGH);
        for (int i = 0; i < LED_COUNT; i++)
          strip[i] = GetColorNow();
        FastLED.show();
        
        if (counter % 2 == 1)
          stepper2.step(256);
        else
          stepper1.step(256);
        
        digitalWrite(BUTTON_RED,LOW);
        digitalWrite(BUTTON_GREEN,LOW);
        for (int i = 0; i < LED_COUNT; i++)
          strip[i] = CRGB::Black; // Черный цвет, т.е. выключено.
        FastLED.show();

        if (counter % 2 == 1)
          stepper2.step(256);
        else
          stepper1.step(256); 
      }
      counter += 1;
    }
    else{
      for (int i = 0; i< 8; i++){
        
        digitalWrite(BUTTON_RED,HIGH);
        for (int i = 0; i < LED_COUNT; i++)
          strip[i] = CRGB::Red;
        FastLED.show();
        
        delay(250);
        
        digitalWrite(BUTTON_RED,LOW);
        for (int i = 0; i < LED_COUNT; i++)
          strip[i] = CRGB::Black; // Черный цвет, т.е. выключено.
        FastLED.show();
        
        delay(250);
      }
    }
  }
  
  if (millis() > timer){
    if (out_led_status == false)
      for (int i = 0; i < LED_COUNT; i++)
        strip[i] = color; // Красный цвет.
    // else
      // for (int i = 0; i < LED_COUNT; i++)
      //   strip[i] = CRGB::Black; // Черный цвет, т.е. выключено.
    
    out_led_status = !out_led_status;
    timer += 20;
  }
  FastLED.show();

}