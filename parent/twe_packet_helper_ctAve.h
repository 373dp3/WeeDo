/*
* twe_packet_helper_ctAve.h
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
// twe_packet_helper_ctAve.h

#ifndef _TWE_PACKET_HELPER_CTAVE_h
#define _TWE_PACKET_HELPER_CTAVE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "twe_packet_helper.h"

struct packet_ave {
	unsigned int cnt;
	unsigned long start_ms;					//最初のON時間
	unsigned int active_duration_deciSec;
	packet_ct lastPacket;
};

class TwePacketHelperCtAve : public TwePacketHelper {
public:
	TwePacketHelperCtAve();

	void init(packet_ave* ave_buf);

	//積算と情報の更新
	int update(packet_ct* packet, int* queueIndex = NULL, uint8_t twe_type = TWE_SENSOR_TYPE_CT);

	//indexで指定した位置にあるバッファ内容をSerialに出力
	void printIdxInfoToSerial(int index);

	//積算情報と更新Ch判定ベクタのクリア
	void clearCntAndChUpdateInfo();

	//前回から更新されたChのビットが1になっているベクタ
	unsigned long getChUpdateVector();

	//指定したindexのバッファポインタを返す。範囲外はNULL
	packet_ave* getPacketAve(int index);

	//指定したindexのバッファは有効か？
	bool isPacketAvailavle(int index, bool isNullCheckOnly = false);

	//Macアドレス解決テーブル用の情報をSakuraioインスタンスに格納する
	void setQueueSakuraIoMac(uint8_t index, SakuraIO_I2C* sakuraio);

	//8オクテットの情報をSakuraioインスタンスに格納する
	void setQueueSakuraIo(uint8_t index, uint8_t twe_type, SakuraIO_I2C* sakuraio);

private:

	//Sakura.io用バイナリを作成する
	void buildSakuraioBin(uint8_t* pBuf, packet_ave* pkt);

	int getBufferIndex(packet_ct* packet);
	static const byte INACTIVE_VAL = 0x00;
	packet_ave* buf;

	//Sakura.ioアップロード～バッファクリア処理中に届き破棄されたパケット
	//の到着時刻を近似値として提供するための時刻変数
	unsigned long lastClearMs = 0;

	//更新したChを判定するベクタ
	unsigned long updateChVector = 0UL;
};

#endif

