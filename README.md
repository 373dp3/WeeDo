# WeeDo
Sakura.ioとGoogleスプレッドシートを用いたビジネス向けIoTを実現する為の各種ソフトウェアです。
可能な限りオープンソース（MITライセンス）にてご提供しております。

##概要
<img src="https://raw.githubusercontent.com/373dp3/WeeDo/master/img/overbiew.jpg" alt="概要">
同一フロア内の機器から取得したデータを近距離無線モジュールTWE-Liteで親機に送信。
4G LTE通信モジュールSakura.ioとArduinoの組み合わせで情報を中継・転送を行いGoogleスプレッドシートに
値を挿入するためのソフトウェア群です。

##特徴
1. 超低コスト志向(月60円のLTE回線であるSakura.io、親機のOSレス構成)
2. 表計算ソフトの知識でフロントエンドを構成する。
3. 商用向け転用に利用しやすいMITライセンス

##フォルダ構成
- DownloadWdLog : Googleスプレッドシートに追記したログを一括取得してCSVファイルに保存する為のツール。
- bin : 各種インストーラ、コンパイル済みバイナリ
- common : parent, seed70mm_frid_client, simple_raw_clientから参照している共通コード
- parent : 親機のArduino向けソースコード
- seed70mm_rfid_client : seeedstudioで販売している70mm RFIDと接続するArduino向けソースコード
- simple_raw_client : TWE-LiteのRAW用ファームウェアと組み合わせて使用する為のArduino向けソースコード
- twe-lite : 2.4GHz無線モジュールTWE-Lite向けソースコード
(VPSサーバにてGoogleスプレッドシートに値を挿入する処理については公開準備中です)

