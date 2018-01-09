/*
* twe_packet_parseer.cpp
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
#include "twe_packet_parseer.h"
#include "dmsg.h"

TwePacketParser::TwePacketParser() {
}

int TwePacketParser::parseProtocolInfo(char* msg, int msgLength, int packetLength = CT_PACKET_LENGTH) {
	int tmpLength = 0;
	//文字数のチェック
	for (int i = 0; i<msgLength; i++) {
		if (msg[i] == '\0') { break; }
		if (msg[i] == '\r') { break; }
		if (msg[i] == '\n') { break; }
		tmpLength++;
	}
	//プロトコルのチェック
	char* ptr;
	byte status = NOOP;
	for (int i = 0; i<tmpLength; i++) {
		ptr = &(msg[i]);
		unsigned long ans = 0;
		switch (status) {
		case NOOP:
			if (*ptr == ':') {
				if (tmpLength - i >= packetLength) {
					//PID1
					ptr = &(msg[i + 3]);
					ans = read1Byte(ptr);
					if (ans != DP3_SERIAL_MSG_ID) {
						dln2("SERIAL_MSG_ID no match: 0x", String(ans, HEX));
						return 0;//ERROR_NODATA
					}
					ptr = &(msg[i + 5]);
					sensorType = read1Byte(ptr);
					ptr = &(msg[i + 7]);
					protVer = read1Byte(ptr);

					//LRCによる誤り検出
					byte pty = 0;
					for (int j = 0; j<packetLength - 1; j += 2) {
						ptr = &(msg[i + j + 1]);
						pty += read1Byte(ptr);
					}
					pty = ~pty + 1;
					//dln2("PTY: 0x", String(pty, HEX));
					ptr = &(msg[i + packetLength]);
					ans = read1Byte(ptr);
					//dln2("LRC: 0x", String(ans, HEX));
					if (pty != ans) { return -1; /*ERROR_LRC*/ }
					status = FINISH;
				}
				else {
					status = FINISH;
				}
			}
			break;
		}
	}

	return tmpLength;
}
/*/
int TwePacketParser::parseCt(char* msg, int msgLength, packet_ct* packet) {
return parseTwe(msg, msgLength, packet, TWE_SENSOR_TYPE_CT);
}
int TwePacketParser::parseCallSwitch(char* msg, int msgLength, packet_ct* packet) {
return parseTwe(msg, msgLength, packet, TWE_SENSOR_TYPE_CALL);
}
//*/
int TwePacketParser::parseTwe(char* msg, int msgLength, packet_ct* packet, uint8_t tweid) {
	int tmpLength = parseProtocolInfo(msg, msgLength);
	if (tmpLength == 0) { return ERROR_NODATA; }
	if (tmpLength == -1) { return ERROR_LRC; }

	//メッセージのパース
	packet->millis = millis();
	packet->swVec.overFlow = 0;
	char* ptr;
	PacketStatus status = NOOP;
	unsigned long ans = 0;
	for (int i = 0; i<tmpLength; i++) {
		ptr = &(msg[i]);
		switch (status) {
		case NOOP:
			if (*ptr == ':') {
				if (tmpLength - i >= CT_PACKET_LENGTH) {
					status = DST;
				}
				else {
					status = FINISH;
				}
			}
			break;
		case DST:
			status = MSGTYPE;
			ans = read1Byte(ptr); i++;
			break;
		case MSGTYPE:
			ans = read1Byte(ptr); i++;
			status = STYPE;
			break;
		case STYPE:
			ans = read1Byte(ptr); i++;
			//センサーがCTで無ければ処理を行わない
			if (ans != tweid) { return ERROR_TYPE_MISSMATCH; }
			status = PVER;
			break;
		case PVER:
			ans = read1Byte(ptr); i++;
			status = LQI;
			break;
		case LQI:
			ans = read1Byte(ptr); i++;
			packet->lqi = ans;
			//最終状態：LQI50未満なら電波強度注意ビットを立てる
			packet->swVec.lqi = (ans < TWE_LQI_ALERT_VALUE) ? 1 : 0;
			status = MAC;
			break;
		case MAC:
			ans = read4Byte(ptr); i += 7;
			packet->mac = ans;
			status = PARENT;
			break;
		case PARENT:
			ans = read1Byte(ptr); i++;
			status = STAMP;
			break;
		case STAMP:
			ans = read2Byte(ptr); i += 3;
			status = RELAY;
			break;
		case RELAY:
			ans = read1Byte(ptr); i++;
			status = BATT;
			break;
		case BATT:
			ans = read2Byte(ptr); i += 3;
			//最終状態：バッテリ2.25v未満でビットを立てる
			packet->swVec.batt = (ans < TWE_BATT_ALERT_VALUE) ? 1 : 0;
			status = SEQ;
			break;
		case SEQ:
			ans = read1Byte(ptr); i++;
			packet->seq = ans;
			status = SW;
			break;
		case SW:
			ans = read1Byte(ptr); i++;
			packet->swVec.sw = ans;
			packet->swVec.isOn = (ans != 0) ? 1 : 0;
			status = REP;
			break;
		case REP:
			ans = read1Byte(ptr); i++;
			packet->rep = ans;
			packet->millis -= ans * 200UL;//rep分だけオフセット
			status = AD1;
			break;

		case AD1:
			ans = read1Byte(ptr); i++;
#ifdef TWE_ADC_ENABLE
			packet->ad[0] = ans;
#endif
			status = AD2;
			break;
		case AD2:
			ans = read1Byte(ptr); i++;
#ifdef TWE_ADC_ENABLE
			packet->ad[1] = ans;
#endif
			status = AD3;
			break;
		case AD3:
			ans = read1Byte(ptr); i++;
#ifdef TWE_ADC_ENABLE
			packet->ad[2] = ans;
#endif
			status = AD4;
			break;
		case AD4:
			ans = read1Byte(ptr); i++;
#ifdef TWE_ADC_ENABLE
			packet->ad[3] = ans;
#endif
			status = ADL;
			break;
		case ADL:
			ans = read1Byte(ptr); i++;
#ifdef TWE_ADC_ENABLE
			packet->ad[4] = ans;
#endif
			status = LRC;
			break;
		case LRC:
			ans = read1Byte(ptr); i++;
			status = FINISH;
			break;

		}//switch
	}//for

	return OK;
}

int TwePacketParser::parseTweCallOrPoll(char* msg, int msgLength, spool_info* packet, uint8_t* buf, uint8_t* tweid)
{
	int tmpLength = parseProtocolInfo(msg, msgLength);
	if (tmpLength == 0) { return ERROR_NODATA; }
	if (tmpLength == -1) { return ERROR_LRC; }

	union
	{
		SwVector swVec;
		byte swVectorByte;
	}SwBuf;

	//メッセージのパース
	//packet->millis = millis();
	//packet->swVec.overFlow = 0;
	char* ptr;
	PacketStatus status = NOOP;
	unsigned long ans = 0;
	for (int i = 0; i<tmpLength; i++) {
		ptr = &(msg[i]);
		switch (status) {
		case NOOP:
			if (*ptr == ':') {
				if (tmpLength - i >= CALL_OR_POLL_PACKET_LENGTH) {
					status = DST;
				}
				else {
					status = FINISH;
					//RAWデータはパケット長が長いため、このループに入る。
					return ERROR_TYPE_MISSMATCH;
				}
			}
			break;
		case DST:
			status = MSGTYPE;
			ans = read1Byte(ptr); i++;
			break;
		case MSGTYPE:
			ans = read1Byte(ptr); i++;
			status = STYPE;
			break;
		case STYPE:
			ans = read1Byte(ptr); i++;
			//センサーがPOLLもしくはCALLで無ければ処理を行わない
			if ((ans != TWE_SENSOR_TYPE_CALL) && (ans != TWE_SENSOR_TYPE_POLL)) {
				return ERROR_TYPE_MISSMATCH;
			}
			if (ans == TWE_SENSOR_TYPE_CALL) {
				buf[0] = SAKURA_8OCT_TYPE_CALL;
			}
			else {
				buf[0] = SAKURA_8OCT_TYPE_POLL;
			}
			//初期化
			buf[1] = 0;
			memset(&SwBuf, 0, sizeof(SwBuf));

			*tweid = ans;
			status = PVER;
			break;
		case PVER:
			ans = read1Byte(ptr); i++;
			status = LQI;
			break;
		case LQI:
			ans = read1Byte(ptr); i++;
			packet->lqi = ans;
			//最終状態：LQI50未満なら電波強度注意ビットを立てる
			//packet->swVec.lqi = (ans < TWE_LQI_ALERT_VALUE) ? 1 : 0;
			SwBuf.swVec.lqi = (ans < TWE_LQI_ALERT_VALUE) ? 1 : 0;
			status = MAC;
			break;
		case MAC:
			ans = read4Byte(ptr); i += 7;
			packet->mac = ans;
			status = PARENT;
			break;
		case PARENT:
			ans = read1Byte(ptr); i++;
			status = STAMP;
			break;
		case STAMP:
			ans = read2Byte(ptr); i += 3;
			status = RELAY;
			break;
		case RELAY:
			ans = read1Byte(ptr); i++;
			status = BATT;
			break;
		case BATT:
			ans = read2Byte(ptr); i += 3;
			//最終状態：バッテリ2.25v未満でビットを立てる
			//packet->swVec.batt = (ans < TWE_BATT_ALERT_VALUE) ? 1 : 0;
			SwBuf.swVec.batt = (ans < TWE_BATT_ALERT_VALUE) ? 1 : 0;
			status = SEQ;
			break;
		case SEQ:
			ans = read1Byte(ptr); i++;
			packet->seq = ans;
			status = SW;
			break;
		case SW:
			ans = read1Byte(ptr); i++;
			//packet->swVec.sw = ans;
			//packet->swVec.isOn = (ans != 0) ? 1 : 0;
			SwBuf.swVec.sw = ans;
			SwBuf.swVec.isOn = (ans != 0) ? 1 : 0;
			status = REP;
			break;
		case REP:
			ans = read1Byte(ptr); i++;
			packet->rep = ans;
			//packet->millis -= ans * 200UL;//rep分だけオフセット
			status = AD1;
			break;

		case AD1:
			buf[1] = SwBuf.swVectorByte;
			ans = read1Byte(ptr); i++;
			buf[2] = ans;
			status = AD2;
			break;
		case AD2:
			ans = read1Byte(ptr); i++;
			buf[3] = ans;
			status = AD3;
			break;
		case AD3:
			ans = read1Byte(ptr); i++;
			buf[4] = ans;
			status = AD4;
			break;
		case AD4:
			ans = read1Byte(ptr); i++;
			buf[5] = ans;
			status = ADL;
			break;
		case ADL:
			ans = read1Byte(ptr); i++;
			buf[6] = ans;
			status = LRC;
			break;
		case LRC:
			ans = read1Byte(ptr); i++;
			status = FINISH;
			break;

		}//switch
	}//for

	return OK;

}

int TwePacketParser::parseTweRaw(char* msg, int msgLength, spool_info* packet, uint8_t* buf, uint8_t* tweid)
{
	int tmpLength = parseProtocolInfo(msg, msgLength, RAW_PACKET_LENGTH);
	if (tmpLength == 0) { return ERROR_NODATA; }
	if (tmpLength == -1) { return ERROR_LRC; }

	//メッセージのパース
	//packet->millis = millis();
	//packet->swVec.overFlow = 0;
	char* ptr;
	PacketStatus status = NOOP;
	unsigned long ans = 0;
	for (int i = 0; i<tmpLength; i++) {
		ptr = &(msg[i]);
		switch (status) {
		case NOOP:
			if (*ptr == ':') {
				if (tmpLength - i >= RAW_PACKET_LENGTH) {
					status = DST;
				}
				else {
					status = FINISH;
					//RAWデータはパケット長が長いため、このループに入る。
					return ERROR_TYPE_MISSMATCH;
				}
			}
			break;
		case DST:
			status = MSGTYPE;
			ans = read1Byte(ptr); i++;
			break;
		case MSGTYPE:
			ans = read1Byte(ptr); i++;
			status = STYPE;
			break;
		case STYPE:
			ans = read1Byte(ptr); i++;
			//センサーがRAWで無ければ処理を行わない
			if ((ans != TWE_SENSOR_TYPE_RAW_NOW) && (ans != TWE_SENSOR_TYPE_RAW_SPOOL)) {
				return ERROR_TYPE_MISSMATCH;
			}
			(*tweid) = ans;

			status = PVER;
			break;
		case PVER:
			ans = read1Byte(ptr); i++;
			status = LQI;
			break;
		case LQI:
			ans = read1Byte(ptr); i++;
			packet->lqi = ans;
			//最終状態：LQI50未満なら電波強度注意ビットを立てる
			//packet->swVec.lqi = (ans < TWE_LQI_ALERT_VALUE) ? 1 : 0;
			status = MAC;
			break;
		case MAC:
			ans = read4Byte(ptr); i += 7;
			packet->mac = ans;
			status = PARENT;
			break;
		case PARENT:
			ans = read1Byte(ptr); i++;
			status = STAMP;
			break;
		case STAMP:
			ans = read2Byte(ptr); i += 3;
			status = RELAY;
			break;
		case RELAY:
			ans = read1Byte(ptr); i++;
			status = BATT;
			break;
		case BATT:
			ans = read2Byte(ptr); i += 3;
			//最終状態：バッテリ2.25v未満でビットを立てる
			//packet->swVec.batt = (ans < TWE_BATT_ALERT_VALUE) ? 1 : 0;
			status = SEQ;
			break;
		case SEQ:
			ans = read1Byte(ptr); i++;
			packet->seq = ans;
			status = SW;
			break;
		case SW:
			ans = read1Byte(ptr); i++;
			//packet->swVec.sw = ans;
			//packet->swVec.isOn = (ans != 0) ? 1 : 0;
			status = REP;
			break;
		case REP:
			ans = read1Byte(ptr); i++;
			packet->rep = ans;
			//packet->millis -= ans * 200UL;//rep分だけオフセット
			status = AD1;
			break;

		case AD1:
			ans = read1Byte(ptr); i++;
			buf[0] = SAKURA_8OCT_TYPE_RAW;
			buf[1] = ans;
			status = AD2;
			break;
		case AD2:
			ans = read1Byte(ptr); i++;
			buf[2] = ans;
			status = AD3;
			break;
		case AD3:
			ans = read1Byte(ptr); i++;
			buf[3] = ans;
			status = AD4;
			break;
		case AD4:
			ans = read1Byte(ptr); i++;
			buf[4] = ans;
			status = ADL;
			break;
		case ADL:
			ans = read1Byte(ptr); i++;
			buf[5] = ans;
			status = AD6;
			break;
		case AD6:
			ans = read1Byte(ptr); i++;
			buf[6] = ans;
			status = AD7;
			break;
		case AD7:
			ans = read1Byte(ptr); i++;
			buf[7] = ans;
			status = LRC;
			break;

		case LRC:
			ans = read1Byte(ptr); i++;
			status = FINISH;
			break;

		}//switch
	}//for

	return OK;

}



unsigned int TwePacketParser::read1Byte(const char* msg) {
	char buf[3] = { msg[0], msg[1], '\0' };
	return strtol(buf, NULL, 16);
}
unsigned int TwePacketParser::read2Byte(const char* msg) {
	char buf[5] = { msg[0], msg[1], msg[2], msg[3], '\0' };
	return strtol(buf, NULL, 16);
}
unsigned long TwePacketParser::read4Byte(const char* msg) {
	unsigned long ans1 = 0;
	unsigned long ans2 = 0;
	char buf1[5] = { msg[0], msg[1], msg[2], msg[3], '\0' };
	ans1 = strtol(buf1, NULL, 16);
	char buf2[5] = { msg[4], msg[5], msg[6], msg[7], '\0' };
	ans2 = strtol(buf2, NULL, 16);
	ans2 += (ans1 << 16);
	return ans2;
}
void TwePacketParser::printPacketToSerial(packet_ct* packet) {
	d("[CT]");
	d("st:");
	d(sensorType);
	d(" pv:");
	d(protVer);

	d(" lqi:");
	d(packet->lqi);
	d(" mac:");
	d(packet->mac);
	d(" seq:");
	d(packet->seq);
	d(" sw:");
	d(packet->swVec.sw);
	d(" v:");
	d(packet->swVectorByte);
	d(" rep:");
	d(packet->rep);
	d(" ms:");
	dln(packet->millis);
}

