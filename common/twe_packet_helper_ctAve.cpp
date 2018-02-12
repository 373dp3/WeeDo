/*
* twe_packet_helper_ctAve.cpp
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

#include "twe_packet_helper_ctAve.h"
#include "dmsg.h"

TwePacketHelperCtAve::TwePacketHelperCtAve() { }

void TwePacketHelperCtAve::init(packet_ave* ave_buf) {
	buf = ave_buf;
}

int TwePacketHelperCtAve::update(packet_ct* packet, int* queueIndex = NULL, uint8_t twe_type = TWE_SENSOR_TYPE_CT) {

	int idx = getBufferIndex(packet);
	packet_ave* ave;

	if (idx >= 0) {//�o�b�t�@�ɑ��݂��邽�߁A�o�b�t�@�����X�V
		ave = &(buf[idx]);

		//�|�C���^���L���ȏꍇ�A�o�b�t�@�̃C���f�b�N�X���i�[
		if (queueIndex != NULL) {
			(*queueIndex) = idx;
		}

		//�J�E���g�A�b�v�́A���Z���T�ł���΃��C�g�I������_�[�N�ɗ��������Ɋm�肷��B
		//sw����A�N�e�B�u�ł��O���seq���قȂ�ꍇ�ɉ��Z����B
		if ((packet->swVec.sw == INACTIVE_VAL) && (ave->lastPacket.seq != packet->seq)) {
			int diff = 0;
			diff = (packet->seq - ave->lastPacket.seq + 256) % 256;
			if (diff != 1) { ave->active_duration_deciSec = 0xFFFF; }
			diff = (diff + 1) >> 1;
			ave->cnt += diff;

			//Sakura.io�X�V�������ɃA�b�v�g���K���͂��Ă����start_ms��0��2�A��L�̃f�[�^�ƂȂ�B
			//���̏ꍇ�Adiff�͋����I��1�ɂ���lastClearMs���n�_�����Ƃ��ċߎ�����B
			if (ave->start_ms == 0) {
				//diff = 0;
				ave->start_ms = lastClearMs;
				ave->active_duration_deciSec = 0xFFFF;
			}

			//ON�䗦�Z�o�p
			unsigned long duration = (packet->millis - ave->lastPacket.millis) / 100;
			if ((((unsigned long)ave->active_duration_deciSec) + duration >= (unsigned long)0x0000FFFF)) {
				ave->active_duration_deciSec = 0xFFFF;
			}
			else {
				ave->active_duration_deciSec += duration;
			}


			//ON-OFF��1�H�������������̂ŏI������(0.1�b�P��)���m�肷��B
			dln2("debug lastpkt ms:", ave->lastPacket.millis);
			dln2("debug start_ms:", ave->start_ms);
		}

		//Ave�N���A��A���߂Ă�ON�M���̏ꍇ��lastPacket.millis���X�V����B
		if ((ave->start_ms == 0) && (packet->swVec.isOn == 1)) {
			ave->start_ms = packet->millis;
		}


		//�ŏI�p�P�b�g����SW��0�Ȃ�A���p�P�b�g��SW�ŏ㏑��
		//ON/OFF�̓r�b�g�t���O�ŁB�M���̎�ނƂ��ėp���Ă���
		if (ave->lastPacket.swVec.sw != INACTIVE_VAL) {
			packet->swVec.sw = ave->lastPacket.swVec.sw;
		}

		//seq�������Ȃ烊�s�[�g�Ɣ��f���Ĕj��
		if (ave->lastPacket.seq == packet->seq) { return NO_UPDATE; }

		//Rep�ŏ㏑���J�E���g���Ă��܂��ꍇ������̂ŁA�f�o�b�O�p
		dln2("- Pre SEQ.:", ave->lastPacket.seq);
		dln2("- Pre REP.:", ave->lastPacket.rep);
		dln2("- New SEQ.:", packet->seq);
		dln2("- New REP.:", packet->rep);

		//�O��A����Ƃ��ɗ����オ��g���K�̏ꍇ�͍X�V���Ȃ��B����ȊO�͍X�V����B
		if ((ave->lastPacket.swVec.isOn == INACTIVE_VAL)
			|| (packet->swVec.isOn == INACTIVE_VAL)) {
			memcpy(&(ave->lastPacket), packet, sizeof(packet_ct));
		}

		return OK_UPDATE;
	}

	// ----- �o�b�t�@�ɑ��݂��Ȃ��ꍇ ------
	packet_ct nullPacket;
	memset(&nullPacket, 0, sizeof(packet_ct));

	//1.�o�b�t�@�ɂ��܂�͂��邩�H
	idx = getBufferIndex(&nullPacket);//���g�p�Ȃ��mac��0�Ȃ̂ŁA0�ʒu�𒲂ׂ�B

									  //2.���܂肪�Ȃ��ꍇ�Alqi�ŏ��l�́H�@�܂��V�p�P�b�g�Ɣ�r���ĐV�p�P�b�g���Ⴂ�ꍇ�͔j��
	if (idx == -1) { //mac 0 ��������ΑS�Ẵo�b�t�@���g�p�ς�
		int minLqi = 1000;//LQI�ő�l��256�Ȃ̂ŁA1000�ŏ\���@�\����
		int minIdx = -1;
		//�ŏ��l����
		for (int i = 0; i<TWE_QUEUE_SIZE; i++) {
			if (buf[i].lastPacket.lqi < minLqi) {
				minLqi = buf[i].lastPacket.lqi;
				minIdx = i;
			}
		}
		//�o�b�t�@���ŏ��l�̕����V�����p�P�b�g�����d�g��������΁A�V�p�P�b�g�͔j������B
		if (minLqi > packet->lqi) {
			return WEAK_LQI;
		}
		//3.�V�p�P�b�g�������ꍇ�́A�ŏ��l�̃o�b�t�@���N���A
		memset(&(buf[minIdx]), 0, sizeof(packet_ave));
	}
	//4.�N���A�ȏ����ʒu��T��
	idx = getBufferIndex(&nullPacket);//���g�p�Ȃ��mac��0�Ȃ̂ŁA0�ʒu�𒲂ׂ�B

									  //5.�o�b�t�@����
	ave = &(buf[idx]);
	if (twe_type == TWE_SENSOR_TYPE_CT) {
		ave->cnt = 0;
	}
	else {
		ave->cnt = 1;
	}

	ave->start_ms = packet->millis;
	ave->active_duration_deciSec = 0;
	memcpy(&(ave->lastPacket), packet, sizeof(packet_ct));

	//6.�X�V�x�N�^��K�p
	updateChVector |= (1UL << idx);

	return OK_NEW;
}

int TwePacketHelperCtAve::getBufferIndex(packet_ct* packet) {
	for (int i = 0; i<TWE_QUEUE_SIZE; i++) {
		if (buf[i].lastPacket.mac == packet->mac) {
			return i;
		}
	}
	return -1;
}


void TwePacketHelperCtAve::clearCntAndChUpdateInfo() {
	//�o�b�t�@�̃N���A
	for (int i = 0; i<TWE_QUEUE_SIZE; i++) {
		buf[i].cnt = 0;
		//SW��ON(�������p����)�̏ꍇ�A�n�_������ێ�����
		if (buf[i].lastPacket.swVec.isOn == 1) {
			buf[i].start_ms = buf[i].lastPacket.millis;
		}
		else {
			//SW�ɗL���l���i�[����Ă���ꍇ�A�����t�@�[���Ɣ��f���āA
			//�����ʂ�n�_�������N���A����BSW��0�̏ꍇ�́A�P�������ꂽ
			//20180211�t�@�[���Ƃ��Ĕ��f����B
			if (buf[i].lastPacket.swVec.sw != 0) {
				//���̏������s���Ă��Ȃ������̂ŃN���A����B
				buf[i].start_ms = 0;
			}
			else {
				//�P�����ł̏ꍇ�͑O��̎��Ԃ�ێ�����B
				buf[i].start_ms = buf[i].lastPacket.millis;
			}
		}
		buf[i].active_duration_deciSec = 0;
		buf[i].lastPacket.swVec.sw = 0;
	}

	//�X�VCh����p�x�N�^�̃N���A
	updateChVector = 0UL;

	//Sakura.io�A�b�v���[�h�`�o�b�t�@�N���A���ɓ͂��j�����ꂽ�p�P�b�g��
	//�ߎ��l��񋟂���ׂ̎����ϐ�
	lastClearMs = millis();

}

unsigned long TwePacketHelperCtAve::getChUpdateVector()
{
	return updateChVector;
}

packet_ave* TwePacketHelperCtAve::getPacketAve(int index)
{
	if (index < 0) { return NULL; }
	if (index >= TWE_QUEUE_SIZE) { return NULL; }
	return &(buf[index]);
}

void TwePacketHelperCtAve::buildSakuraioBin(uint8_t * pBuf, packet_ave* pkt)
{
	if (pBuf == NULL) { return; }
	if (pkt == NULL) { return; }

	//�I�N�e�b�g���
	(*pBuf) = (uint8_t)0x10;//CT�q�@��������ʃR�[�h
	pBuf++;

	//�ŏI��ԁF�J�E���g��255�𒴂��Ă���ꍇ�Ɍx��
	pkt->lastPacket.swVec.overFlow = (pkt->cnt > 255) ? 1 : 0;

	//SW�x�N�^
	(*pBuf) = (uint8_t)(pkt->lastPacket.swVectorByte);
	pBuf++;

	//�o�������@1�o�C�g
	(*pBuf) = (uint8_t)pkt->cnt;
	pBuf++;

	//�v�����ԁ@2�o�C�g (dsec 0.1�b�P��)
	unsigned long diff = (pkt->lastPacket.millis - pkt->start_ms) / 100UL;
	(*pBuf) = (uint8_t)(diff >> 8); pBuf++;
	(*pBuf) = (uint8_t)(0x00FFUL & diff); pBuf++;

	//ON�䗦�@1�o�C�g
	uint8_t ratio = 0;
	//�p�P�b�g���X���̓��Ꮘ�����K�v���H
	if (pkt->active_duration_deciSec == 0xFFFF) {
		//�p�P�b�g���X����
		ratio = 0xFF;
	}
	else {
		//�p�P�b�g���X����
		if ((pkt->lastPacket.millis - pkt->start_ms) > 0) {
			if (pkt->lastPacket.swVec.isOn == 0) {
				//ON��OFF�̐����Ⴄ���߁A�␳
				uint16_t hiOneDurationMs = pkt->active_duration_deciSec * 100UL / pkt->cnt;
				ratio = (uint8_t)(200.0*(pkt->active_duration_deciSec * 100UL - hiOneDurationMs)
					/ (pkt->lastPacket.millis - hiOneDurationMs - pkt->start_ms));
			}
			else {
				//�ʏ�̌v�Z��
				ratio = (uint8_t)(200.0*(pkt->active_duration_deciSec * 100UL) / (pkt->lastPacket.millis - pkt->start_ms));
			}
		}
	}
	(*pBuf) = (uint8_t)(ratio); pBuf++;

	//�\���@2�o�C�g(AD1�`AD2�܂ł����Ɋ��蓖�Ă邩)
#ifdef TWE_ADC_ENABLE
	(*pBuf) = (uint8_t)(pkt->lastPacket.ad[0]); pBuf++;
	(*pBuf) = (uint8_t)(pkt->lastPacket.ad[1]); pBuf++;
#else
	(*pBuf) = (uint8_t)(0); pBuf++;
	(*pBuf) = (uint8_t)(0); pBuf++;
#endif

}

void TwePacketHelperCtAve::printIdxInfoToSerial(int index) {
	if (index >= TWE_QUEUE_SIZE) { return; }

	packet_ave* ave = &(buf[index]);
	dln2(" AVE.mac: ", ave->lastPacket.mac);
	dln2(" AVE.cnt: ", ave->cnt);
	dln2(" AVE.start_ms: ", ave->start_ms);
	dln2(" AVE.active_duration_sec: ", ave->active_duration_deciSec / 10.0);
	dln2(" AVE ratio1: ", 100.0*(ave->active_duration_deciSec * 100UL) / (ave->lastPacket.millis - ave->start_ms));
	//dln2(" AVE ratio2: ", 100.0*(ave->active_duration_deciSec * 100UL) / (ave->finish_delta_deciSec*100UL));
}

bool TwePacketHelperCtAve::isPacketAvailavle(int index, bool isNullCheckOnly = false) {
	//�p�P�b�g�̎擾
	packet_ave* pkt = getPacketAve(index);
	if (pkt == NULL) { return false; }//�͈͊O�G���[

	if (isNullCheckOnly) { return true; }

	//MAC������0�̏ꍇ�A���g�p�̃o�b�t�@�Ɣ��f����B
	if (pkt->lastPacket.mac == 0) { return false; }

	//cnt��0�̏ꍇ�A�������Ȃ��B
	if (pkt->cnt == 0) { return false; }

	return true;
}

void TwePacketHelperCtAve::setQueueSakuraIo(uint8_t index, uint8_t twe_type, SakuraIO_I2C* sakuraio) {
	uint8_t txbuf[8];

	packet_ave* pkt = getPacketAve(index);
	buildSakuraioBin(txbuf, pkt);

	sakuraio->enqueueTx(index, txbuf, millis() - pkt->lastPacket.millis);

}

void TwePacketHelperCtAve::setQueueSakuraIoMac(uint8_t index, SakuraIO_I2C* sakuraio) {
	packet_ave* pkt = getPacketAve(index);

	sakuraio->enqueueTx(index,
		(uint64_t)pkt->lastPacket.mac,
		millis() - pkt->lastPacket.millis);
}