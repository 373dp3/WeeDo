/*
* spool_manager.cpp
*
* Copyright (c) 2017 WeeDo
* Author      : Toshiaki Minami (min@dp3.jp)
* Create Time: 2017/12/14
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
#include "spool_manager.h"

SpoolMgr::SpoolMgr(SakuraIO_I2C * sakuraio_i2c)
{
	for (int i = 0; i < SPOOL_MGR_TABLE_SIZE; i++) {
		memset(&(spoolTable[i]), 0, sizeof(spool_info));
	}

	sakuraio = sakuraio_i2c;
}

int SpoolMgr::put(spool_info* spool, uint8_t* oct8, unsigned long *txCnt, bool isTxNow)
{
	//�C���f�b�N�X�̎擾����
	int idx = getSpoolIndex(spool);

	//�o�^�p�̎��Ԏ擾
	unsigned long opTimeMs = spool->rep*TWE_REPEAT_INTERVAL_MS;


	//���ɑ��݂��邩�H
	if (idx >= 0) {
		//���ɑ��݂����ꍇ�ASEQ�̃`�F�b�N
		if (spoolTable[idx].seq == spool->seq) {
			//seq����v�����ꍇ�͏I��
			//Serial.println("SAME SEQ");
			return SAME_SEQ;
		}
		dln2("NEW SEQ:", spool->mac);
	}
	else {
		//���߂Ă̏ꍇ
		spool_info nullPacket;
		memset(&nullPacket, 0, sizeof(spool_info));

		//1.�o�b�t�@�ɂ��܂�͂��邩�H
		idx = getSpoolIndex(&nullPacket);//���g�p�Ȃ��mac��0�Ȃ̂ŁA0�ʒu�𒲂ׂ�B

										 //2.���܂肪�Ȃ��ꍇ�Alqi�ŏ��l�́H�@�V�p�P�b�g�Ɣ�r���ĐV�p�P�b�g���Ⴂ�ꍇ�͔j��
		if (idx == -1) { //mac 0 ��������ΑS�Ẵo�b�t�@���g�p�ς�
			int minLqi = 1000;//LQI�ő�l��256�Ȃ̂ŁA1000�ŏ\���@�\����
			int minIdx = -1;
			//�ŏ��l����
			for (int i = 0; i<SPOOL_MGR_TABLE_SIZE; i++) {
				if (spoolTable[i].lqi < minLqi) {
					minLqi = spoolTable[i].lqi;
					minIdx = i;
				}
			}
			//�o�b�t�@���ŏ��l�̕����V�����p�P�b�g�����d�g��������΁A�V�p�P�b�g�͔j������B
			if (minLqi > spool->lqi) {
				dln2("WEAK_LQI:", spool->mac);
				return WEAK_LQI;
			}
			//3.�V�p�P�b�g�������ꍇ�́A�ŏ��l�̃o�b�t�@���N���A
			memset(&(spoolTable[minIdx]), 0, sizeof(spool_info));

			idx = minIdx;

			dln2("NEW :", spool->mac);

		}

		//Sakura.io��Index�AMac����o�^(�I�t�Z�b�g�l��)
		sakuraio->enqueueTx(idx + SPOOL_SAKURAIO_INDEX_OFFSET,
			(uint64_t)spool->mac, opTimeMs);
	}

	//seq���قȂ邩�A�V�K�f�[�^�Ȃ̂Ńo�b�t�@�X�V
	memcpy(&(spoolTable[idx]), spool, sizeof(spool_info));
	Serial.println("OK end");

	//Sakura.io��QCT��o�^(�I�t�Z�b�g�l��)
	sakuraio->enqueueTx(idx + SPOOL_SAKURAIO_INDEX_OFFSET, oct8, opTimeMs);

	//Queue�̃T�C�Y���m�F
	uint8_t que = 0;
	uint8_t ave = 0;
	if (sakuraio->getTxQueueLength(&ave, &que) != SAKURA_RES_OK) {
		dln("[ERR] SAKURA NOT READY");
		return SAKURA_NOT_READY;
	}

	//���M����t���O
	bool isTx = isTxNow;
	if (isTx) {
		dln("ixTx: true");
	}
	else {
		dln("ixTx: false");
	}

	//�L���[����t�Ȃ瑗�M�t���O�𗧂Ă�
	if (que >= MAX_QUEUE) {
		dln("SEND_SAKURIO_MAX_QUEUE");
		isTx = true;
	}

	//���M�ߑ��̏ꍇ�̓t���O�����낷
	if (((*txCnt) != 0) && ((millis() / (*txCnt)) < SPOOL_UPLOAD_LIMITER_MS)) {
		dln2("AVOID TX BY FREQ. LIMIT: ", (millis() / (*txCnt)));
		isTx = false;
	}

	//���M�̎�����
	if (isTx) {
		//���M
		sakuraio->send();
		dln("sakuraio->send()");

		//�J�E���^�̐ώZ
		(*txCnt)++;
	}

	lastUpdateTimeMs = millis();

	return OK;
}

bool SpoolMgr::isQueueAvailable()
{
	uint8_t que = 0;
	uint8_t ave = 0;
	if (sakuraio->getTxQueueLength(&ave, &que) == SAKURA_RES_OK) {
		if (que > 0) {
			return true;
		}
	}
	return false;
}

unsigned long SpoolMgr::getLastUpdateMs()
{
	return lastUpdateTimeMs;
}

int SpoolMgr::getSpoolIndex(spool_info* spool)
{
	for (int i = 0; i<SPOOL_MGR_TABLE_SIZE; i++) {
		if (spoolTable[i].mac == spool->mac) {
			return i;
		}
	}
	return -1;
}
