/*
* parent.ino
*
* Copyright (c) 2018 WeeDo
* Author      : Toshiaki Minami (min@dp3.jp)
* Create Time: 2018/01/09
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
#include "twe_packet_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SakuraIO.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#define LED_PIN	(13)

//アップロード頻度リミッタ(SPOOL_UPLOAD_LIMITER_MS)は,
//spool_manager.hにて定義

#include "dmsg.h"
#include "SakuraIoCommon.h"
#include "RawClient.h"
#include "spool_manager.h"
#include "twe_packet_helper_ctAve.h"
#include "twe_packet_parseer.h"


//ADコンバータの値も有効にする
//#define TWE_ADC_ENABLE
#define MAX_STR_BUF (64)



//CT用配列サイズ
static const int TWE_QUEUE_SIZE = 10;
packet_ave twe_queue_buf[TWE_QUEUE_SIZE];

//RFID用スプールのサイズ
#define SPOOL_MGR_TABLE_SIZE	(TWE_QUEUE_SIZE)

unsigned long term_start_ms = 0;
unsigned long sakuraio_tx_cnt = 0;

TwePacketParser twe;
TwePacketHelperCtAve helper;
SakuraIO_I2C sakuraio;
SpoolMgr spoolMgr(&sakuraio);
RawClient rawclient;

//Serialの通信速度
#define HW_SERIAL_BPS	(38400)

//定期アップロード処理の時間間隔(ミリ秒)
#define UPLOAD_INTERVAL_MS	(300000UL)


void setup() {
	//ハードウェアシリアル初期化
	Serial.begin(HW_SERIAL_BPS);

	//TWE-Lite平均化処理インスタンス初期化
	memset(twe_queue_buf, 0, sizeof(twe_queue_buf));
	helper.init(twe_queue_buf);

	//Sakura.ioスリープピン
	pinMode(PIN_SAKURA_IO_SLEEP_CTRL, OUTPUT);
	digitalWrite(PIN_SAKURA_IO_SLEEP_CTRL, HIGH);

	//Sakura.ioリセットピン
	pinMode(PIN_SAKURA_IO_RESET_CTRL, INPUT);

	dln2("TWE-AVE BUFFER_SIZE: ", sizeof(twe_queue_buf));

	//LED出力
	pinMode(LED_PIN, OUTPUT);

}//setup


unsigned long keepOutMs = 0;


void loop() {
	int bufferIndex = 0;//AveバッファIndex取得用
	uint8_t signal = sakuraio.getSignalQuality();


	//電波が無い場合、LEDを点灯する。
	if (signal == 0) {
		digitalWrite(LED_PIN, HIGH);
	}
	else {
		digitalWrite(LED_PIN, LOW);
	}

	//バッファをSakura.ioで送信
	if (millis() - term_start_ms > UPLOAD_INTERVAL_MS) {
		//Sakura.ioの送信処理　返り値はSakura.ioの利用回数
		sakuraio_tx_cnt += fetchSakuraIo(&helper, TWE_SENSOR_TYPE_CT);

		uint8_t que;
		uint8_t imm;
		if (sakuraio.getTxStatus(&que, &imm) == SAKURA_OK) {

			//送信が行われず、かつ、スプールに残っている場合はスプール送信
			//(CT側で送信が行われているならスプールも解消される)
			if ((que != 0x01/* 送信中を示すコード */)
				&& (spoolMgr.isQueueAvailable())) {

				sakuraio.send();

			}
		}
	}

	//シリアル用バッファ蓄積
	unsigned long timeoutMs = millis() + 1000UL;
	char buf[MAX_STR_BUF];
	memset(buf, 0, MAX_STR_BUF);
	int isStart = 0;
	for (int i = 0; i<MAX_STR_BUF;) {
		int chk = Serial.read();
		if (millis() > timeoutMs) { return; }
		if (chk == -1) { continue; }

		if (chk == ':') {
			isStart = 1;
			timeoutMs = millis() + 2000UL;//タイムアウトの更新
		}
		if (isStart != 0) {
			buf[i] = chk;
			if (chk == 10 /* \n */) { break; }
			i++;
		}
	}

	while (Serial.read() >= 0) {}//バッファの破棄

	//  ----------------------------------------------------------------- [RAW NOW/RAW SPOOL]
	{
		spool_info spool;
		uint8_t dat[8];
		uint8_t tiweid = 0;
		int ans = twe.parseTweRaw(buf, sizeof(buf), &spool, dat, &tiweid);
		if (TwePacketParser::OK == ans) {
			//OKの時点灯開始
			digitalWrite(LED_PIN, HIGH);

			//即時送信タイプであればフラグを立てる
			bool isTxNow = (tiweid == TWE_SENSOR_TYPE_RAW_NOW);

			if (spoolMgr.put(&spool, dat, &sakuraio_tx_cnt, isTxNow) != SpoolMgr::SAME_SEQ) {
				//ACKの送信
				RawData rawdata;
				memset(&rawdata, 0, sizeof(RawData));
				for (int i = 0; i<sizeof(rawdata.data); i++) {
					rawdata.data[i] = dat[i + 1];
				}
				rawclient.sendData(&rawdata);
			}

			//直ぐに消灯
			if (signal > 0) { digitalWrite(LED_PIN, LOW); }
		}
		else {
			if (ans != TwePacketParser::ERROR_TYPE_MISSMATCH) {
				dln2("parse error: ", ans);
				buf[MAX_STR_BUF - 1] = 0x00;
				Serial.println(buf);

				//シリアルの再起動
				Serial.end();
				Serial.begin(HW_SERIAL_BPS);
			}
		}
	}

	//  ----------------------------------------------------------------- [POLL/CALL]
	{
		spool_info spool;
		uint8_t dat[8];
		uint8_t tiweid = 0;
		int ans = twe.parseTweCallOrPoll(buf, sizeof(buf), &spool, dat, &tiweid);
		if (TwePacketParser::OK == ans) {
			//OKの時点灯開始
			digitalWrite(LED_PIN, HIGH);

			//即時送信タイプであればフラグを立てる
			bool isTxNow = (tiweid == TWE_SENSOR_TYPE_CALL);

			if (spoolMgr.put(&spool, dat, &sakuraio_tx_cnt, isTxNow) != SpoolMgr::SAME_SEQ) {
				if (isTxNow) {
					dln2("CALL ", spool.mac);
				}
				else {
					dln2("POLL ", spool.mac);
				}
				if (sakuraio_tx_cnt > 0) {
					dln2("Tx ave:", (millis() / (sakuraio_tx_cnt)));
				}
			}

			//直ぐに消灯
			if (signal > 0) { digitalWrite(LED_PIN, LOW); }
		}
		else {
			dln2("parse error: ", ans);
			if (ans != TwePacketParser::ERROR_TYPE_MISSMATCH) {
				dln2("parse error: ", ans);
				buf[MAX_STR_BUF - 1] = 0x00;
				Serial.println(buf);

				//シリアルの再起動
				Serial.end();
				Serial.begin(HW_SERIAL_BPS);
			}
		}
	}

	//  ----------------------------------------------------------------- [ CT ]
	packet_ct ct;
	int ans = twe.parseTwe(buf, sizeof(buf), &ct, TWE_SENSOR_TYPE_CT);
	if (TwePacketParser::OK == ans) {
		//OKの時点灯開始
		digitalWrite(LED_PIN, HIGH);
		int code = helper.update(&ct, &bufferIndex);
		if (code != helper.NO_UPDATE) {
			twe.printPacketToSerial(&ct);
			helper.printIdxInfoToSerial(bufferIndex);
			dln2(" AVE result: ", helper.retCodeToString(code));
		}
		//直ぐに消灯
		if (signal > 0) { digitalWrite(LED_PIN, LOW); }
	}

}
