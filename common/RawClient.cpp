/*
* RawClient.cpp
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

#include "RawClient.h"

RawClient::RawClient(int rxPin, int txPin, int bps)
{
	_rawSoftSerial = new SoftwareSerial(rxPin, txPin);
	_rawSoftSerial->begin(bps);
}


RawClient::RawClient(SoftwareSerial * serial)
{
	_rawSoftSerial = serial;
}

RawClient::RawClient() {
	_rawSoftSerial = NULL;
}

RawClient::~RawClient()
{
}

void RawClient::sendData(RawData * data)
{
	char tmp[64];
	memset(tmp, 0, sizeof(tmp));

	//値の格納
	for (int i = 0; i<7; i++) {
		_rc_buf[2 + i] = data->data[i];
	}

	//LRC
	_rc_buf[sizeof(_rc_buf) - 1] = 0;
	for (int i = 0; i<sizeof(_rc_buf) - 1; i++) {
		_rc_buf[sizeof(_rc_buf) - 1] += _rc_buf[i];
	}
	_rc_buf[sizeof(_rc_buf) - 1] = ~_rc_buf[sizeof(_rc_buf) - 1] + 1;

	//コマンド「:」
	tmp[0] = ':';

	buf2hex(_rc_buf, &(tmp[1]), sizeof(_rc_buf), sizeof(tmp) - 1);

	Serial.println(tmp);

	if (_rawSoftSerial != NULL) {
		_rawSoftSerial->println(tmp);
	}
}

//uint8_t配列から大文字HEX CHAR配列を作成する。
int RawClient::buf2hex(const uint8_t* pBuf, char* ans, uint8_t bufLen, uint8_t chLen) {
	for (int i = 0; i<bufLen; i++) {
		if (i * 2 + 1 >= chLen) { return 0; }
		char c1 = table[pBuf[i] & 0x0F];
		char c2 = table[(pBuf[i] >> 4) & 0x0F];
		ans[i * 2 + 0] = c2;
		ans[i * 2 + 1] = c1;
	}
	return 1;
}