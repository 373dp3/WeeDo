/*
* twe_packet_parseer.h
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
#ifndef _TWE_PACKET_PARSEER_H
#define _TWE_PACKET_PARSEER_H

#include <Arduino.h>

//LQI���ォ�����ꍇ�̌x���𔭂��邵�����l
#define TWE_LQI_ALERT_VALUE	(50)

//�d�����Ⴍ�Ȃ����ꍇ�̌X���𔭂��邵�����l
#define TWE_BATT_ALERT_VALUE (2300UL)

//TWE���b�Z�[�W����CT�Z���T���������ʕ���
#define TWE_SENSOR_TYPE_CT		(0x01)

//TWE���b�Z�[�W���̌Ăяo�����u���������ʕ���
#define TWE_SENSOR_TYPE_CALL	(0x02)

//TWE���b�Z�[�W���̒���ʒm���������ʕ���
#define TWE_SENSOR_TYPE_POLL	(0x03)

//TWE���b�Z�[�W����RAW(7OCTET)���������ʕ���(�X�v�[���^)
#define TWE_SENSOR_TYPE_RAW_SPOOL	(0x04)

//TWE���b�Z�[�W����RAW(7OCTET)���������ʕ���(�������M�^)
#define TWE_SENSOR_TYPE_RAW_NOW	(0x05)

//Sakura.io��8�I�N�e�b�g�𑗐M����ۂ̎��ʕ���(��������)
#define SAKURA_8OCT_TYPE_CALL		(0x20)
#define SAKURA_8OCT_TYPE_POLL		(0x30)
#define SAKURA_8OCT_TYPE_RAW		(0x40)
//Sakura.io��8�I�N�e�b�g�𑗐M����ۂ̎��ʕ���(�����܂�)

struct SwVector {
	byte sw : 4;
	byte isOn : 1;
	byte batt : 1;
	byte lqi : 1;
	byte overFlow : 1;
};

struct packet_ct {
	unsigned long mac;
	byte lqi;//�o�b�t�@�̗D��x����ɕK�v
	byte seq;
	byte rep;
	union
	{
		SwVector swVec;
		byte swVectorByte;
	};
	unsigned long millis;
#ifdef TWE_ADC_ENABLE
	byte ad[5];//�Ō�̂P�͊e�`�����l���̉��ʂQ�r�b�g�W����
#endif
			   //  byte flag;
};


struct spool_info {
	unsigned long mac;
	byte lqi;//�o�b�t�@�D��x����Ɏg�p
	byte seq;
	byte rep;
};


class TwePacketParser {
public:
	TwePacketParser();

	int parseTwe(char* msg, int msgLength, packet_ct* packet, uint8_t tweid);

	//RAW�t�H�[�}�b�g�pTWE�o�C�i������ buf��SakuraIO�p�o�C�i��(8 Oct)
	int parseTweRaw(char* msg, int msgLength, spool_info* packet, uint8_t* buf, uint8_t* tweid);

	//�������M�E������M�t�H�[�}�b�g�pTWE�o�C�i������ buf��SakuraIO�p�o�C�i��(8 Oct)
	int parseTweCallOrPoll(char* msg, int msgLength, spool_info* packet, uint8_t* buf, uint8_t* tweid);

	//�p�P�b�g�̃f�o�b�O���o��
	void printPacketToSerial(packet_ct* packet);

	//�����̒萔
	enum PacketStatus {
		NOOP, DST, MSGTYPE, STYPE, PVER, LQI, MAC, PARENT, STAMP, RELAY,
		BATT, SEQ, SW, REP, AD1, AD2, AD3, AD4, ADL, AD6, AD7, LRC, FINISH, OK,
		ERROR_NODATA, ERROR_LRC, ERROR_TYPE_MISSMATCH
	};

	static const int CT_PACKET_LENGTH = 49;
	static const int CALL_OR_POLL_PACKET_LENGTH = 49;

	static const int RAW_PACKET_LENGTH = 51;

private:
	//�v���g�R���o�[�W�����擾
	int parseProtocolInfo(char* msg, int msgLength, int packetLength = CT_PACKET_LENGTH);


	byte sensorType = 0;
	byte protVer = 0;

	unsigned int read1Byte(const char* msg);
	unsigned int read2Byte(const char* msg);
	unsigned long read4Byte(const char* msg);

	static const byte DP3_SERIAL_MSG_ID = 0xD3;
};


#endif
