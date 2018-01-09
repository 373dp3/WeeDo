/*
* seed70mm_rfid_client.ino
*
* Copyright (c) 2017 WeeDo
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
#include <SoftwareSerial.h>
#include "RawClient.h"


#define RAW_CLIENT_RX_PIN 10
#define RAW_CLIENT_TX_PIN 11
#define RAW_CLIENT_BPS	9600
RawClient raw(RAW_CLIENT_RX_PIN, RAW_CLIENT_TX_PIN, RAW_CLIENT_BPS);

#define LED_PIN	13

//チャタリング防止時間処理用変数(スイッチ切替瞬間の状態不安定化対策)
unsigned long privantChatteringTimeLimitMs = 0;
#define PRIVENT_CHATTERING_TIME_MS	(500)

#define PHOTO_TR_PIN	9
#define REED_SWITCH_PIN	7
int preStatePhotoTr = 1;
int preStateReeSwitch = 1;

void setup() {

	Serial.begin(115200);

	//LEDピンを出力に設定
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, LOW);

	//INPUT_PULLUP：端子開放時にH、短絡時にL
	pinMode(PHOTO_TR_PIN, INPUT_PULLUP);
	pinMode(REED_SWITCH_PIN, INPUT_PULLUP);

}

bool isPinChanged(int* preState, int pinNo) {
	int res = 0;
	res = digitalRead(pinNo);

	//前回のピン状態と異なるか？
	if (*preState != res) {
		privantChatteringTimeLimitMs = millis() + PRIVENT_CHATTERING_TIME_MS;
		while (privantChatteringTimeLimitMs > millis()) {
			res = digitalRead(pinNo);
			//前回と同じ状態になったら、チャタリング中途判断して処理終了
			if (*preState == res) { return false; }
		}

		//状態が変わった
		*preState = res;
		return true;
	}

	return false;
}

void loop() {
	bool isChanged = false;

	//フォトトランジスタの確認
	if (isPinChanged(&preStatePhotoTr, PHOTO_TR_PIN)) {
		isChanged = true;
	}

	//リードスイッチの確認
	if (isPinChanged(&preStateReeSwitch, REED_SWITCH_PIN)) {
		isChanged = true;
	}

	if (isChanged) {
		RawData rawdata;
		memset(&rawdata, 0, sizeof(RawData));

		rawdata.data[0] = preStatePhotoTr;
		rawdata.data[1] = preStateReeSwitch;
		raw.sendData(&rawdata);

		digitalWrite(LED_PIN, HIGH);
		delay(200);
		digitalWrite(LED_PIN, LOW);
	}

	delay(2000);
}
