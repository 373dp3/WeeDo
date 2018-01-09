/*
* spool_manager.h
*
* Copyright (c) 2017 WeeDo
* Author      : Toshiaki Minami (min@dp3.jp)
* Create Time: 2017/12/14
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

#ifndef _SPOOL_MANAGER_h
#define _SPOOL_MANAGER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <SakuraIO.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "twe_packet_parseer.h"
#include "dmsg.h"

#ifndef SPOOL_MGR_TABLE_SIZE
#define SPOOL_MGR_TABLE_SIZE	(5)
#endif

#define TWE_REPEAT_INTERVAL_MS	(200UL)
#define SPOOL_SAKURAIO_INDEX_OFFSET	(64)

//アップロード頻度リミッタ 送信頻度がこれ以下の場合は送信しない
#define SPOOL_UPLOAD_LIMITER_MS	(90UL * 1000UL)


class SpoolMgr {
public:
	SpoolMgr(SakuraIO_I2C* sakuraio_i2c);
	int put(spool_info* spool, uint8_t* oct8, unsigned long *txCnt, bool isTxNow);
	bool isQueueAvailable();
	unsigned long getLastUpdateMs();

	enum SpoolMgrResult {
		NOOP, SAME_SEQ, WEAK_LQI, SAKURA_NOT_READY, OK
	};

private:
	int getSpoolIndex(spool_info* spool);

	SakuraIO_I2C* sakuraio;

	//新規子機やテーブル更新の場合、一度に2枠使用する場合があるため、
	//最大値である16から2つ余裕をもたせる。
	const int MAX_QUEUE = 14;
	const int SAKURA_RES_OK = 1;

	spool_info spoolTable[SPOOL_MGR_TABLE_SIZE];

	unsigned long lastUpdateTimeMs = 0;

};

#endif

