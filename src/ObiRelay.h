/*
 * ObiRelay.h
 *
 *  Created on: 20.05.2018
 *      Author: a4711
 */

#ifndef SRC_OBIRELAY_H_
#define SRC_OBIRELAY_H_
#include <Arduino.h>

class ObiRelay
{
  enum
  {
    RELAY_OFF = 5,  // <- RED LED shows status
    RELAY_ON = 12,
    PULSE_WIDTH = 1 // width of LOW pulse for controlling relay
  };

public:
  ObiRelay ();
  virtual ~ObiRelay ();

  void setup()
  {
    pinMode(RELAY_OFF, OUTPUT);
    pinMode(RELAY_ON, OUTPUT);
    enable(true);
    delay(PULSE_WIDTH);
    enable(false);
  }

  void enable(bool status)
  {
    state = status;
    if (status)
    {
      digitalWrite(RELAY_ON, LOW);
      delay(PULSE_WIDTH);
      digitalWrite(RELAY_ON, HIGH);
    }
    else
    {
      digitalWrite(RELAY_OFF, LOW);
      delay(PULSE_WIDTH);
      digitalWrite(RELAY_OFF, HIGH);
    }
  };

  void toggle()
  {
    enable(!state);
  }

  void timed_on(int seconds)
  {
    activate_timer(seconds);
  }

  void expire()
  {
    check_timer();
  }

private:

  void activate_timer(int seconds)
  {
    timer_seconds = seconds;
    timer_start_ms = millis();
    timer_running = true;
    enable(true);
  }
  void expire_timer()
  {
    timer_seconds = 0;
    timer_start_ms = 0;
    timer_running = false;
    enable(false);
  }

  void check_timer()
  {
    if (!timer_running) return;
    unsigned long dt = millis()-timer_start_ms;
    if (dt >= timer_seconds*1e3)
    {
      expire_timer();
    }
  }


  bool state = false;
  unsigned long timer_start_ms;
  unsigned long timer_seconds;
  bool timer_running;
};

#endif /* SRC_OBIRELAY_H_ */
