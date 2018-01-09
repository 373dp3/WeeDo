/*
* SakuraIoCommon.cpp
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
// 
// 
// 

#include "SakuraIoCommon.h"

extern unsigned long term_start_ms;
extern TwePacketParser twe;
extern SakuraIO_I2C sakuraio;

uint32_t preHms = 25UL * 3600UL;//25時という実在しない時間を設定
uint32_t getHmsSec() {
	uint32_t utime = (uint32_t)(sakuraio.getUnixtime() / 1000ULL);
	dln2("utime:", utime);
	uint32_t hms = (uint32_t)((utime + 9UL * 3600UL) % (24UL * 3600UL));
	dln2("hms:", hms);
	uint32_t hh = (uint32_t)(hms / 3600UL);
	uint32_t mm = (uint32_t)((hms - hh * 3600UL) / 60UL);
	uint32_t ss = hms - hh * 3600UL - mm * 60UL;
	dln2("hh:mm:ss ", (uint32_t)(hh * 10000 + mm * 100 + ss));
	return hms;
}

uint8_t getSakuraIoTxQueueSize() {
	uint8_t available = 0;
	uint8_t queued = 0;
	sakuraio.getTxQueueLength(&available, &queued);

	return queued;
}

uint8_t fetchSakuraIo(TwePacketHelper* helper, uint8_t twe_type) {
	uint8_t cnt = 0;
	dln("---- set queue into sakura.io ----");

	//Sakura.io状態チェック
	uint32_t hms = getHmsSec();

	bool isReset = false;
	if ((hms > SYSTEM_RESET_HMS) && (preHms < SYSTEM_RESET_HMS)) {
		//アップロード処理完了後にリセット
		isReset = true;
	}
	preHms = hms;

	//電波状況チェック

	// -- 初登場のモジュールをID、MAC通知 --
	unsigned long vector = helper->getChUpdateVector();
	dln2("vector:", vector);

	// -- 初めて現れたMACを通知する --
	for (uint8_t i = 0; i < TWE_QUEUE_SIZE; i++) {

		//シフト結果から該当するindex(Ch)に復帰
		if ((vector & (1UL << i)) == false) {
			continue;
		}

		//このchのデータは更新されている
		if (helper->isPacketAvailavle(i, true) == false) { continue; }

		uint8_t available = 0;
		uint8_t queued = 0;

		//有効時は0x00、それ以外は>0
		dln("Access to sakura.io");
		if (sakuraio.getTxQueueLength(&available, &queued) != SAKURA_OK)
		{
			dln("Sakura.io eror 1"); goto CLEAR_END;
		}
		dln("Sakura.io responce OK");

		//残数が0なら送信
		if (available == 0) {
			sakuraio.send();
			cnt++;
			for (int i = 0; i<3000; i++) {//0.1秒間隔でチェック、300秒、5分まで。
				delay(100);
				sakuraio.getTxQueueLength(&available, &queued);
				//登録可能ならばブレイク
				if (available > 0) { break; }
			}
		}

		//availableが0のままなら、タイムアウトしたと判断
		if (available == 0) { dln("ERROR: Sakura.io timeout"); goto CLEAR_END; }

		//MAC情報をキューに登録(64bit, 8octets uintがMAC情報として運用する)
		helper->setQueueSakuraIoMac(i, &sakuraio);

		dln2("Mac enquque:", i);
	}

	// -- 積算情報を送信する --
	for (uint8_t i = 0; i < TWE_QUEUE_SIZE; i++) {

		//指定したインデックスのパケットは有効か？
		if (helper->isPacketAvailavle(i) == false) { continue; }

		//キューのチェック 有効時は0x00、それ以外は>0
		uint8_t available = 0;
		uint8_t queued = 0;
		if (sakuraio.getTxQueueLength(&available, &queued) != SAKURA_OK)
		{
			dln("Sakura.io eror"); goto CLEAR_END;
		}

		//残数が0なら送信
		if (available == 0) {
			sakuraio.send();
			cnt++;
			for (int i = 0; i<3000; i++) {//0.1秒間隔でチェック、300秒、5分まで。
				delay(100);
				sakuraio.getTxQueueLength(&available, &queued);
				//登録可能ならばブレイク
				if (available > 0) { break; }
			}
		}

		//availableが0のままなら、タイムアウトしたと判断
		if (available == 0) { dln("ERROR: Sakura.io timeout"); goto CLEAR_END; }

		//送信用バイナリ構築
		dln2("dat enqueue :", i);
		helper->setQueueSakuraIo(i, twe_type, &sakuraio);
	}

	//キューに登録された物があれば送信
	{
		uint8_t available = 0;
		uint8_t queued = 0;
		sakuraio.getTxQueueLength(&available, &queued);
		dln2("Queue size:", queued);
		if (queued > 0) { sakuraio.send(); cnt++; }
	}

	//積算値をリセット
	helper->clearCntAndChUpdateInfo();

CLEAR_END:
	dln("---- clear ----");
	term_start_ms = millis();


	if (isReset) {
		dln(" **** RESET ****");
		uint8_t available = 0;
		uint8_t queued = 0;
		for (int i = 0; i<1200; i++) {
			sakuraio.getTxQueueLength(&available, &queued);
			if (queued == 0) { break; }
			delay(100);
		}
		//Sakura.ioリセット
		dln(" **** RESET Sakura.io ****");
		digitalWrite(PIN_SAKURA_IO_RESET_CTRL, LOW);
		pinMode(PIN_SAKURA_IO_RESET_CTRL, OUTPUT);
		digitalWrite(PIN_SAKURA_IO_RESET_CTRL, LOW);
		delay(1000);//1秒保持
		pinMode(PIN_SAKURA_IO_RESET_CTRL, INPUT);

		dln(" **** RESET software reset ****");
		delay(100);//0.1秒保持

				   //WDTリセットを誘発させる
		{
			asm volatile ("  jmp 0");//Arduino Pro mini向け
			wdt_disable();
			wdt_enable(WDTO_15MS);
			while (1) {}
		}
		dln(" **** RESET ERROR ****");
	}

	return cnt;
}

