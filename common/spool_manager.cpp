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
	//インデックスの取得試み
	int idx = getSpoolIndex(spool);

	//登録用の時間取得
	unsigned long opTimeMs = spool->rep*TWE_REPEAT_INTERVAL_MS;


	//既に存在するか？
	if (idx >= 0) {
		//既に存在した場合、SEQのチェック
		if (spoolTable[idx].seq == spool->seq) {
			//seqが一致した場合は終了
			//Serial.println("SAME SEQ");
			return SAME_SEQ;
		}
		dln2("NEW SEQ:", spool->mac);
	}
	else {
		//初めての場合
		spool_info nullPacket;
		memset(&nullPacket, 0, sizeof(spool_info));

		//1.バッファにあまりはあるか？
		idx = getSpoolIndex(&nullPacket);//未使用ならばmacは0なので、0位置を調べる。

										 //2.あまりがない場合、lqi最小値は？　新パケットと比較して新パケットが低い場合は破棄
		if (idx == -1) { //mac 0 が無ければ全てのバッファが使用済み
			int minLqi = 1000;//LQI最大値は256なので、1000で十分機能する
			int minIdx = -1;
			//最小値調査
			for (int i = 0; i<SPOOL_MGR_TABLE_SIZE; i++) {
				if (spoolTable[i].lqi < minLqi) {
					minLqi = spoolTable[i].lqi;
					minIdx = i;
				}
			}
			//バッファ内最小値の方が新しいパケットよりも電波が強ければ、新パケットは破棄する。
			if (minLqi > spool->lqi) {
				dln2("WEAK_LQI:", spool->mac);
				return WEAK_LQI;
			}
			//3.新パケットが高い場合は、最小値のバッファをクリア
			memset(&(spoolTable[minIdx]), 0, sizeof(spool_info));

			idx = minIdx;

			dln2("NEW :", spool->mac);

		}

		//Sakura.ioにIndex、Mac情報を登録(オフセット考慮)
		sakuraio->enqueueTx(idx + SPOOL_SAKURAIO_INDEX_OFFSET,
			(uint64_t)spool->mac, opTimeMs);
	}

	//seqが異なるか、新規データなのでバッファ更新
	memcpy(&(spoolTable[idx]), spool, sizeof(spool_info));
	Serial.println("OK end");

	//Sakura.ioにQCTを登録(オフセット考慮)
	sakuraio->enqueueTx(idx + SPOOL_SAKURAIO_INDEX_OFFSET, oct8, opTimeMs);

	//Queueのサイズを確認
	uint8_t que = 0;
	uint8_t ave = 0;
	if (sakuraio->getTxQueueLength(&ave, &que) != SAKURA_RES_OK) {
		dln("[ERR] SAKURA NOT READY");
		return SAKURA_NOT_READY;
	}

	//送信判定フラグ
	bool isTx = isTxNow;
	if (isTx) {
		dln("ixTx: true");
	}
	else {
		dln("ixTx: false");
	}

	//キューが一杯なら送信フラグを立てる
	if (que >= MAX_QUEUE) {
		dln("SEND_SAKURIO_MAX_QUEUE");
		isTx = true;
	}

	//送信過多の場合はフラグを下ろす
	if (((*txCnt) != 0) && ((millis() / (*txCnt)) < SPOOL_UPLOAD_LIMITER_MS)) {
		dln2("AVOID TX BY FREQ. LIMIT: ", (millis() / (*txCnt)));
		isTx = false;
	}

	//送信の実処理
	if (isTx) {
		//送信
		sakuraio->send();
		dln("sakuraio->send()");

		//カウンタの積算
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
