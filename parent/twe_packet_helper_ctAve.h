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
	unsigned long start_ms;					//�ŏ���ON����
	unsigned int active_duration_deciSec;
	packet_ct lastPacket;
};

class TwePacketHelperCtAve : public TwePacketHelper {
public:
	TwePacketHelperCtAve();

	void init(packet_ave* ave_buf);

	//�ώZ�Ə��̍X�V
	int update(packet_ct* packet, int* queueIndex = NULL, uint8_t twe_type = TWE_SENSOR_TYPE_CT);

	//index�Ŏw�肵���ʒu�ɂ���o�b�t�@���e��Serial�ɏo��
	void printIdxInfoToSerial(int index);

	//�ώZ���ƍX�VCh����x�N�^�̃N���A
	void clearCntAndChUpdateInfo();

	//�O�񂩂�X�V���ꂽCh�̃r�b�g��1�ɂȂ��Ă���x�N�^
	unsigned long getChUpdateVector();

	//�w�肵��index�̃o�b�t�@�|�C���^��Ԃ��B�͈͊O��NULL
	packet_ave* getPacketAve(int index);

	//�w�肵��index�̃o�b�t�@�͗L�����H
	bool isPacketAvailavle(int index, bool isNullCheckOnly = false);

	//Mac�A�h���X�����e�[�u���p�̏���Sakuraio�C���X�^���X�Ɋi�[����
	void setQueueSakuraIoMac(uint8_t index, SakuraIO_I2C* sakuraio);

	//8�I�N�e�b�g�̏���Sakuraio�C���X�^���X�Ɋi�[����
	void setQueueSakuraIo(uint8_t index, uint8_t twe_type, SakuraIO_I2C* sakuraio);

private:

	//Sakura.io�p�o�C�i�����쐬����
	void buildSakuraioBin(uint8_t* pBuf, packet_ave* pkt);

	int getBufferIndex(packet_ct* packet);
	static const byte INACTIVE_VAL = 0x00;
	packet_ave* buf;

	//Sakura.io�A�b�v���[�h�`�o�b�t�@�N���A�������ɓ͂��j�����ꂽ�p�P�b�g
	//�̓����������ߎ��l�Ƃ��Ē񋟂��邽�߂̎����ϐ�
	unsigned long lastClearMs = 0;

	//�X�V����Ch�𔻒肷��x�N�^
	unsigned long updateChVector = 0UL;
};

#endif

