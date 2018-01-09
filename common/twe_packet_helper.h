/*
* twe_packet_helper.h
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
#ifndef _TWE_PACKET_AVE_HELPER_H
#define _TWE_PACKET_AVE_HELPER_H

#include <Arduino.h>
#include <stdio.h>
#include "twe_packet_parseer.h"
#include <SakuraIO.h>

extern const int TWE_QUEUE_SIZE;

class TwePacketHelper {
public:
	TwePacketHelper();

	//積算と情報の更新
	virtual int update(packet_ct* packet, int* queueIndex = NULL, uint8_t twe_type = TWE_SENSOR_TYPE_CT) = 0;

	//リターンコードからテキストに変換
	char* retCodeToString(int code);

	//indexで指定した位置にあるバッファ内容をSerialに出力
	virtual void printIdxInfoToSerial(int index) = 0;

	//積算情報と更新Ch判定ベクタのクリア
	virtual void clearCntAndChUpdateInfo() = 0;

	//前回から更新されたChのビットが1になっているベクタ
	virtual unsigned long getChUpdateVector() = 0;

	//指定したindexのバッファは有効か？
	virtual bool isPacketAvailavle(int index, bool isNullCheckOnly = false) = 0;

	virtual void setQueueSakuraIoMac(uint8_t index, SakuraIO_I2C* sakuraio) = 0;

	//Sakura.io用バイナリを作成する
	virtual void setQueueSakuraIo(uint8_t index, uint8_t twe_type, SakuraIO_I2C* sakuraio) = 0;


	//応答の定数
	enum Code { NOOP, OK, OK_UPDATE, OK_NEW, NO_UPDATE, TOO_MANY_INFO, WEAK_LQI };

};


#endif
