#include <Adafruit_NeoPixel.h>
#include <Easing.h>
#include <elapsedMillis.h>
#include <SerialCommand.h>

#define NEO_PIN 6
#define NEO_NUMPIX 8

#define INTERVAL 1000

elapsedMillis timeElapsed;
SerialCommand sCmd;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NEO_NUMPIX, NEO_PIN, NEO_GRB + NEO_KHZ800);

struct Colour {
  byte r;
  byte g;
  byte b;
};

struct RgbTween {
  Colour   from;
  Colour   to;
  uint32_t duration = 0;
  uint32_t pos = 0;
  boolean  complete = true;
};

Colour current_colour;

#define MAX_TWEEN_QUEUE_SIZE 2
struct TweenQueue {
  RgbTween tweens[MAX_TWEEN_QUEUE_SIZE];
  uint8_t length = 0;
} tween_queue;

void cmd_unrecognised(const char *command) {
  Serial.println("Unrecognised command. Ensure you have newline enabled");
}

void cmd_colour() {
  char *arg;
  arg = sCmd.next();

  String arg1 = String(arg);

  if (arg1 != NULL) {
    if (arg1.equalsIgnoreCase("GET")) {
      // return the current colour values
      Serial.print("RGB ");
      Serial.print(current_colour.r);
      Serial.print(",");
      Serial.print(current_colour.g);
      Serial.print(",");
      Serial.println(current_colour.b);
    }
    else if (arg1.equalsIgnoreCase("RGB")) {
      // set the RGB values directly
      arg = sCmd.next();
      arg1 = String(arg);

      uint8_t comma1 = arg1.indexOf(",");
      uint8_t comma2 = arg1.indexOf(",", comma2);

      Colour newcolour;

      newcolour.r = arg1.substring(0, comma1).toInt();
      newcolour.g = arg1.substring(comma1, comma2).toInt();
      newcolour.b = arg1.substring(comma2).toInt();

      current_colour = newcolour;
    }
  }

  sCmd.clearBuffer();
}

void setup() {
  sCmd.addCommand("COLOUR", cmd_colour);
  sCmd.setDefaultHandler(cmd_unrecognised);

  pixels.begin();
  pixels.show();
}

boolean tweening = false;
uint8_t r, g, b;

void loop() {
  // put your main code here, to run repeatedly:

  if (!tweening && tween_queue.length > 0) {
    tweening = true;

    Colour *from = &tween_queue.tweens[0].from;
    Colour *to   = &tween_queue.tweens[0].to;

    // Make the destination the distance
    to->r = current_colour.r - from->r;
    to->g = current_colour.g - from->g;
    to->b = current_colour.b - from->b;

    from->r = current_colour.r;
    from->g = current_colour.g;
    from->b = current_colour.b;
  }

  if (tweening) {
    
  }
}
