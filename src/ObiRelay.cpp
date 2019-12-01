/*
 * ObiRelay.cpp
 *
 *  Created on: 20.05.2018
 *      Author: a4711
 */

#include "ObiRelay.h"

ObiRelay::ObiRelay ()
: state(false),
  timer_start_ms(0),
  timer_seconds(0),
  timer_running(false)
{
}

ObiRelay::~ObiRelay ()
{
}

