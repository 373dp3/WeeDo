/*
* RawClient.h
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
// RawClient.h

#ifndef _RAWCLIENT_h
#define _RAWCLIENT_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <SoftwareSerial.h>

struct RawData
{
	uint8_t data[7];
};

//TWE-Lite中の自モジュールへの命令を示すコード
#define SERCMD_ADDR_TO_MODULE (0xDB)

//TWE-Lite中の自モジュール命令コード
#define SERCMD_ID_GET_MODULE_ADDRESS  (0x90)

//WeeDo用の拡張7オクテット送信命令
#define WEEDO_SEND_OCTET  (0x37)

class RawClient {
public:
	//Twe-liteの設定と一致させて初期化
	RawClient(int rxPin, int txPin, int bps);

	//Rxを別の用途で使用したい場合
	RawClient(SoftwareSerial* serial);

	//ハードウェアシリアルを用いる場合
	RawClient();

	~RawClient();

	//情報の送信
	void sendData(RawData* data);

private:
	int buf2hex(const uint8_t* pBuf, char* ans, uint8_t bufLen, uint8_t chLen);

	const char table[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

	//自分ID,CMD,DATA[5],LRC
	uint8_t _rc_buf[10] = {
		SERCMD_ADDR_TO_MODULE,
		WEEDO_SEND_OCTET,
		1,2,3,4,5,6,7,
		0
	};

	SoftwareSerial * _rawSoftSerial;

};

#endif

