# WeeDo
Sakura.ioとGoogleスプレッドシートを用いたビジネス向けIoTを実現する為の各種ソフトウェアです。
可能な限りオープンソース（MITライセンス）にてご提供しております。

## 概要図
<img src="https://raw.githubusercontent.com/373dp3/WeeDo/master/img/overbiew.jpg" alt="概要">
同一フロア内の機器から取得したデータを近距離無線モジュールTWE-Liteで親機に送信。
4G LTE通信モジュールSakura.ioとArduinoの組み合わせで情報を中継・転送を行いGoogleスプレッドシートに
値を挿入するためのソフトウェア群です。

## 特徴
1. 超低コスト志向 (月60円のLTE回線であるSakura.ioを複数の無線センサ子機で共有、親機OSレス)
2. 表計算ソフトの知識でフロントエンドを構成する。
3. 商用向け転用に利用しやすいMITライセンス

## 活用例
1. RFIDリーダを用いたプレゼンス管理
2. 工場の可動率管理（サイクルタイムモニタ）
3. 職制・ライン外担当者等の呼び出し装置
4. 環境温度の定点観測装置

等など、ビジネス向けIoTシステムを月額ワンコイン以下の通信費で維持。
Googleスプレッドシートを用いますので表示内容をご自身で簡単にカスタマイズする事ができます。

## フォルダ構成
<table>
  <tr>
    <td>DownloadWdLog </td>
    <td>Googleスプレッドシートに追記したログを一括取得してCSVファイルに保存する為のツール </td>
  </tr>
  <tr>
    <td>bin </td>
    <td>各種インストーラ、コンパイル済みバイナリ </td>
  </tr>
  <tr>
    <td>bin </td>
    <td>Googleスプレッドシートに追記したログを一括取得してCSVファイルに保存する為のツール </td>
  </tr>
  <tr>
    <td>common </td>
    <td>parent, seed70mm_frid_client, simple_raw_clientから参照している共通コード </td>
  </tr>
  <tr>
    <td>eagle </td>
    <td>電子回路CAD EAGLE用の各種ファイル </td>
  </tr>
  <tr>
    <td>parent </td>
    <td>親機のArduino向けソースコード </td>
  </tr>
  <tr>
    <td>seed70mm_rfid_client </td>
    <td>seeedstudioで販売している70mm RFIDと接続するArduino向けソースコード </td>
  </tr>
  <tr>
    <td>simple_raw_client </td>
    <td>TWE-LiteのRAW用ファームウェアと組み合わせて使用する為のArduino向けソースコード </td>
  </tr>
  <tr>
    <td>twe-lite </td>
    <td>2.4GHz無線モジュールTWE-Lite向けソースコード </td>
  </tr>
</table>
(VPSサーバにてGoogleスプレッドシートに値を挿入する処理については公開準備中です)

## 公式サイト
<a href="https://secure.dp3.jp/WeeDo/">https://secure.dp3.jp/WeeDo/</a>
