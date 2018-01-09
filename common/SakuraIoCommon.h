/*
* SakuraIoCommon.h
*
* Copyright (c) 2017 WeeDo
* Author      : Toshiaki Minami (min@dp3.jp)
* Create Time: 2017/12/1
*
* The MIT License (MIT)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/
// SakuraIoCommon.h

#ifndef _SAKURAIOCOMMON_h
#define _SAKURAIOCOMMON_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <SakuraIO.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "twe_packet_parseer.h"
#include "twe_packet_helper.h"
#include "dmsg.h"

#define PIN_SAKURA_IO_SLEEP_CTRL	(6)
//Sakura.ioのリセット制御(Z/L)
#define PIN_SAKURA_IO_RESET_CTRL	(3)
#define SAKURA_OK	(1)

//システムリセットを行う時刻 4:55 => (4*3600UL + 55*60)
#define SYSTEM_RESET_HMS	(4*3600UL + 55*60)


uint32_t getHmsSec();

uint8_t fetchSakuraIo(TwePacketHelper* helper, uint8_t twe_type);
uint8_t getSakuraIoTxQueueSize();

#endif

