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

/*
 * sort this out to be a linked list or ring buffer
 */

struct RgbTween {
  Colour   from;
  Colour   to;
  uint32_t duration = 0;
  uint32_t pos = 0;
  boolean  complete = true;
};

struct Node {
  RgbTween tween;
  Node next;
};

Colour current_colour;

#define MAX_TWEEN_QUEUE_SIZE 2
struct TweenQueue {
  RgbTween tweens[MAX_TWEEN_QUEUE_SIZE];
  uint8_t length = 0;
} tween_queue;

/*
 * Modified version of the function from the Easing library
 * Shortcuts if there is no distance to ease toward
 */
float easeInOutCubic (float t, float b, float c, float d) {
  if (c == 0) return b;
  else if ((t/=d/2) < 1) return c/2*t*t*t + b;
  return c/2*((t-=2)*t*t + 2) + b;
}

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
    Colour *from = &tween_queue.tweens[0].from;
    Colour *to   = &tween_queue.tweens[0].to;

    uint32_t elapsed  = timeElapsed;
    uint32_t duration = tween_queue.tweens[0].duration;

    if (elapsed >= duration) {
      elapsed = duration;
      teen_queue.tweens[0].complete = true;
    }

    uint8_t r = easeInOutCubic(elapsed, from->r, to->r, duration);
    uint8_t g = easeInOutCubic(elapsed, from->g, to->g, duration);
    uint8_t b = easeInOutCubic(elapsed, from->b, to->b, duration);

    uint32_t pixel_colour = pixels.Color(r, g, b);
    for (uint8_t i=0; i<NEO_NUMPIX; i++) {
      pixels.setPixelColor(i, pixel_colour);
    }

    if (teen_queue.tweens[0].complete) {
      // manage the queue
      tweening = false;
    }
  }
}
