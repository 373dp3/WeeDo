/*
* twe_packet_helper.cpp
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
#include "twe_packet_helper.h"
#include "dmsg.h"

TwePacketHelper::TwePacketHelper() { }

char* TwePacketHelper::retCodeToString(int code) {
	//NOOP,  OK, OK_NEW, TOO_MANY_INFO, WEAK_LQI
	switch (code) {
	case NOOP:  return (char*)"NOOP";
	case OK:  return (char*)"OK";
	case OK_NEW:  return (char*)"OK_NEW";
	case TOO_MANY_INFO:  return (char*)"TOO_MANY_INFO";
	case WEAK_LQI:  return (char*)"WEAK_LQI";
	case NO_UPDATE: return (char*)"NO_UPDATE";
	case OK_UPDATE: return (char*)"OK_UPDATE";
	default:
		return (char*)"UNKOWN";
	}
	return (char*)"";
}
