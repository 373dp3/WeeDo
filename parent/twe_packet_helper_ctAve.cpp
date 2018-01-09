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

	if (idx >= 0) {//バッファに存在するため、バッファ内を更新
		ave = &(buf[idx]);

		//ポインタが有効な場合、バッファのインデックスを格納
		if (queueIndex != NULL) {
			(*queueIndex) = idx;
		}

		//カウントアップは、光センサであればライトオンからダークに落ちた時に確定する。
		//swが非アクティブでかつ前回とseqが異なる場合に加算する。
		if ((packet->swVec.sw == INACTIVE_VAL) && (ave->lastPacket.seq != packet->seq)) {
			int diff = 0;
			diff = (packet->seq - ave->lastPacket.seq + 256) % 256;
			if (diff != 1) { ave->active_duration_deciSec = 0xFFFF; }
			diff = (diff + 1) >> 1;
			ave->cnt += diff;

			//Sakura.io更新処理中にアップトリガが届いているとstart_msが0で2連続Lのデータとなる。
			//その場合、diffは強制的に1にしてlastClearMsを始点時刻として近似する。
			if (ave->start_ms == 0) {
				//diff = 0;
				ave->start_ms = lastClearMs;
				ave->active_duration_deciSec = 0xFFFF;
			}

			//ON比率算出用
			unsigned long duration = (packet->millis - ave->lastPacket.millis) / 100;
			if ((((unsigned long)ave->active_duration_deciSec) + duration >= (unsigned long)0x0000FFFF)) {
				ave->active_duration_deciSec = 0xFFFF;
			}
			else {
				ave->active_duration_deciSec += duration;
			}


			//ON-OFFの1工程が完了したので終了時間(0.1秒単位)を確定する。
			dln2("debug lastpkt ms:", ave->lastPacket.millis);
			dln2("debug start_ms:", ave->start_ms);
		}

		//Aveクリア後、初めてのON信号の場合はlastPacket.millisを更新する。
		if ((ave->start_ms == 0) && (packet->swVec.isOn == 1)) {
			ave->start_ms = packet->millis;
		}


		//最終パケット情報のSWが0なら、旧パケットのSWで上書き
		//ON/OFFはビットフラグで。信号の種類として用いている
		if (ave->lastPacket.swVec.sw != INACTIVE_VAL) {
			packet->swVec.sw = ave->lastPacket.swVec.sw;
		}

		//seqが同じならリピートと判断して破棄
		if (ave->lastPacket.seq == packet->seq) { return NO_UPDATE; }

		//Repで上書きカウントしてしまう場合があるので、デバッグ用
		dln2("- Pre SEQ.:", ave->lastPacket.seq);
		dln2("- Pre REP.:", ave->lastPacket.rep);
		dln2("- New SEQ.:", packet->seq);
		dln2("- New REP.:", packet->rep);

		//前回、今回ともに立ち上がりトリガの場合は更新しない。それ以外は更新する。
		if ((ave->lastPacket.swVec.isOn == INACTIVE_VAL)
			|| (packet->swVec.isOn == INACTIVE_VAL)) {
			memcpy(&(ave->lastPacket), packet, sizeof(packet_ct));
		}

		return OK_UPDATE;
	}

	// ----- バッファに存在しない場合 ------
	packet_ct nullPacket;
	memset(&nullPacket, 0, sizeof(packet_ct));

	//1.バッファにあまりはあるか？
	idx = getBufferIndex(&nullPacket);//未使用ならばmacは0なので、0位置を調べる。

									  //2.あまりがない場合、lqi最小値は？　また新パケットと比較して新パケットが低い場合は破棄
	if (idx == -1) { //mac 0 が無ければ全てのバッファが使用済み
		int minLqi = 1000;//LQI最大値は256なので、1000で十分機能する
		int minIdx = -1;
		//最小値調査
		for (int i = 0; i<TWE_QUEUE_SIZE; i++) {
			if (buf[i].lastPacket.lqi < minLqi) {
				minLqi = buf[i].lastPacket.lqi;
				minIdx = i;
			}
		}
		//バッファ内最小値の方が新しいパケットよりも電波が強ければ、新パケットは破棄する。
		if (minLqi > packet->lqi) {
			return WEAK_LQI;
		}
		//3.新パケットが高い場合は、最小値のバッファをクリア
		memset(&(buf[minIdx]), 0, sizeof(packet_ave));
	}
	//4.クリアな書込位置を探索
	idx = getBufferIndex(&nullPacket);//未使用ならばmacは0なので、0位置を調べる。

									  //5.バッファ書込
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

	//6.更新ベクタを適用
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
	//バッファのクリア
	for (int i = 0; i<TWE_QUEUE_SIZE; i++) {
		buf[i].cnt = 0;
		//SWがON(処理が継続中)の場合、始点時刻を保持する
		if (buf[i].lastPacket.swVec.isOn == 1) {
			buf[i].start_ms = buf[i].lastPacket.millis;
		}
		else {
			//何の処理も行われていなかったのでクリアする。
			buf[i].start_ms = 0;
		}
		buf[i].active_duration_deciSec = 0;
		buf[i].lastPacket.swVec.sw = 0;
	}

	//更新Ch判定用ベクタのクリア
	updateChVector = 0UL;

	//Sakura.ioアップロード～バッファクリア中に届き破棄されたパケットの
	//近似値を提供する為の時刻変数
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

	//オクテット種別
	(*pBuf) = (uint8_t)0x10;//CT子機を示す種別コード
	pBuf++;

	//最終状態：カウントが255を超えている場合に警告
	pkt->lastPacket.swVec.overFlow = (pkt->cnt > 255) ? 1 : 0;

	//SWベクタ
	(*pBuf) = (uint8_t)(pkt->lastPacket.swVectorByte);
	pBuf++;

	//出来高数　1バイト
	(*pBuf) = (uint8_t)pkt->cnt;
	pBuf++;

	//計測時間　2バイト (dsec 0.1秒単位)
	unsigned long diff = (pkt->lastPacket.millis - pkt->start_ms) / 100UL;
	(*pBuf) = (uint8_t)(diff >> 8); pBuf++;
	(*pBuf) = (uint8_t)(0x00FFUL & diff); pBuf++;

	//ON比率　1バイト
	uint8_t ratio = 0;
	//パケットロス時の特例処理が必要か？
	if (pkt->active_duration_deciSec == 0xFFFF) {
		//パケットロスあり
		ratio = 0xFF;
	}
	else {
		//パケットロス無し
		if ((pkt->lastPacket.millis - pkt->start_ms) > 0) {
			if (pkt->lastPacket.swVec.isOn == 0) {
				//ONとOFFの数が違うため、補正
				uint16_t hiOneDurationMs = pkt->active_duration_deciSec * 100UL / pkt->cnt;
				ratio = (uint8_t)(200.0*(pkt->active_duration_deciSec * 100UL - hiOneDurationMs)
					/ (pkt->lastPacket.millis - hiOneDurationMs - pkt->start_ms));
			}
			else {
				//通常の計算式
				ratio = (uint8_t)(200.0*(pkt->active_duration_deciSec * 100UL) / (pkt->lastPacket.millis - pkt->start_ms));
			}
		}
	}
	(*pBuf) = (uint8_t)(ratio); pBuf++;

	//予備　2バイト(AD1～AD2までを仮に割り当てるか)
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
	//パケットの取得
	packet_ave* pkt = getPacketAve(index);
	if (pkt == NULL) { return false; }//範囲外エラー

	if (isNullCheckOnly) { return true; }

	//MAC部分が0の場合、未使用のバッファと判断する。
	if (pkt->lastPacket.mac == 0) { return false; }

	//cntが0の場合、処理しない。
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