using GetWeeDoLog;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace DownloadWdLog
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        public AppData appData = new AppData();
        public MainWindow()
        {
            InitializeComponent();
            this.DataContext = appData;
            appData.url = Properties.Settings.Default.preUrl;
            appData.jsonFile = Properties.Settings.Default.jsonPath;
            appData.sqlitePath = Environment.GetFolderPath(
                Environment.SpecialFolder.LocalApplicationData
                ) + "\\DownloadWdLog";

            var assmVer = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location).ProductVersion.ToString();
            this.Title += " " + assmVer;

        }

        private void button_Click(object sender, RoutedEventArgs e)
        {
            appData.status = "";
            Properties.Settings.Default.preUrl = this.appData.url;
            Properties.Settings.Default.Save();

            if (appData.jsonFile.Length == 0)
            {
                MessageBox.Show("client_id.jsonを選択してください。");
                return;
            }
            if (appData.url.Length == 0)
            {
                MessageBox.Show("スプレッドシート元データURLを入力してください。");
                return;
            }

            this.textBoxStatus.IsReadOnly = true;

            Task th = Task.Factory.StartNew(() => {
                try
                {
                    doTask();
                }
                catch (Google.GoogleApiException ge)
                {
                    appData.log("Google API呼び出しでエラーが発生しました。");
                    if (ge.Message.Contains("一覧"))
                    {
                        appData.log("参照先のシートに「一覧」というシートが含まれていません。アカウント開設通知書に記載の「情報追記先URL」を入力してください。");
                    }else
                    {
                        appData.log(ge.ToString());
                    }                
                }catch(Exception ex)
                {
                    appData.log(ex.ToString());
                }
            });

        }

        private void doTask()
        {
            var baseUrl = this.appData.url;

            DirectoryInfo di = (new FileInfo(appData.sqlitePath)).Directory;
            if (di.Exists == false) { di.Create(); }
            SheetWorker worker = new SheetWorker(appData.jsonFile);
            var db = new SheetSqliteHelper(appData.sqlitePath, baseUrl);

            //履歴一覧の取得
            appData.log("一覧の取得");
            var histList = worker.getHistoryInfo(baseUrl);
            db.insertOrUpdateHistory(histList);

            //履歴一覧の更新
            var list = db.getUrlJobList();
            appData.log("未受信履歴ファイル： " + list.Count + "件");
            foreach (var url in list)
            {
                appData.log("履歴取得：" + url);

                //取得処理
                var data = worker.getRawInfo(url);

                //取得結果の保存
                appData.log("キャッシュ更新中： " + data.Count + "件");
                db.insertOrUpdateRaw(data);

                //DBへ取得済みフラグ更新
                db.urlJobDone(url);
            }

            //最新情報の更新
            appData.log("最新情報の取得");
            var curData = worker.getRawInfo(baseUrl);
            appData.log("情報数:" + curData.Count + "件");
            db.insertOrUpdateRaw(curData);
            db.vacuum();
            appData.log("完了しました。");
            Dispatcher.BeginInvoke(new Action(() =>
            {
                this.textBoxStatus.IsReadOnly = false;
                saveToCsv(baseUrl);
            }), null);

        }

        private void saveToCsv(string baseUrl)
        {
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Filter = "CSVファイル(*.cvs)|*.csv";
            if (Properties.Settings.Default.csvPath.Length>0)
            {
                FileInfo fi = new FileInfo(Properties.Settings.Default.csvPath);
                dlg.InitialDirectory = fi.Directory.FullName;
                dlg.FileName = fi.Name;
            }
            if (dlg.ShowDialog() == true)
            {
                Properties.Settings.Default.csvPath = dlg.FileName;
                Properties.Settings.Default.Save();

                appData.log("CSVファイルへ保存中・・・");
                var db = new SheetSqliteHelper(appData.sqlitePath, baseUrl);
                db.saveToCsv(dlg.FileName);
                appData.log("CSVファイルへの保存を完了しました。");
            }
        }

        private void button1_Click(object sender, RoutedEventArgs e)
        {
            var dlg = new OpenFileDialog();
            dlg.Filter = "JSONファイル (*.json)|*.json";

            if (dlg.ShowDialog() == true)
            {
                appData.jsonFile = dlg.FileName;
                Properties.Settings.Default.jsonPath = dlg.FileName;
                Properties.Settings.Default.Save();
            }
        }
    }
}
