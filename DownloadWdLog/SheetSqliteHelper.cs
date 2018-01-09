using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Data.SQLite;
using System.IO;

namespace GetWeeDoLog
{
    public class SheetSqliteHelper
    {
        SQLiteConnection conn = null;
        
        public SheetSqliteHelper(string dbPath, string sheetUrl)
        {
            conn = new SQLiteConnection();
            FileInfo dbFile = new FileInfo(
                dbPath + "\\"
                + SheetWorker.sheetUrlToId(sheetUrl) + ".db");

            bool isNewFile = !dbFile.Exists;

            conn.ConnectionString = "Data Source=" + dbFile.FullName + ";Version=3;";
            conn.Open();

            if (isNewFile)
            {
                createTable();
            }
        }

        private void createTable()
        {
            string[] queryAry = {
                "CREATE TABLE [sheetList] ([yearmon] INTEGER, [url] TEXT, [flg] INTEGER, PRIMARY KEY(yearmon,url));",
                "CREATE TABLE [rawTable] ([type] INTEGER, [datetime] TEXT, [mac] INTEGER, [d1] TEXT DEFAULT '', [d2] TEXT DEFAULT '', [d3] TEXT DEFAULT '', [d4] TEXT DEFAULT '', [d5] TEXT DEFAULT '', [d6] TEXT DEFAULT '', [d7] TEXT DEFAULT '', PRIMARY KEY(type, datetime, mac, d1, d2, d3, d4, d5, d6, d7));"
            };

            using (var tran = conn.BeginTransaction())
            {
                foreach (var query in queryAry)
                {
                    var cmd = conn.CreateCommand();
                    cmd.CommandText = query;
                    cmd.ExecuteNonQuery();

                }
                tran.Commit();
            }
        }

        public void vacuum()
        {
            using (var cmd = conn.CreateCommand())
            {
                cmd.CommandText = "vacuum; ";
                cmd.ExecuteNonQuery();
            }
        }

        public void insertOrUpdateHistory(List<List<string>> data)
        {
            //回避リストの作成
            var readCmd = conn.CreateCommand();
            var avoidUrlList = new List<string>();
            readCmd.CommandText = "select url from sheetList where flg=1";
            using (var reader = readCmd.ExecuteReader())
            {
                for (var i = 0; reader.Read(); i++)
                {
                    if (reader.FieldCount != 1) { continue; }
                    avoidUrlList.Add(reader[0].ToString());
                }
            }

            using (var tran = conn.BeginTransaction())
            {
                //データの挿入ループ
                foreach (var row in data)
                {
                    //既に取得済み(flg=1)の場合は回避
                    if (avoidUrlList.Contains(row[1])) { continue; }

                    //挿入処理
                    using (var cmd = conn.CreateCommand())
                    {
                        cmd.CommandText =
                            "REPLACE INTO sheetList(yearmon, url, flg) "
                            + "VALUES(" + row[0] + ",\"" + row[1] + "\", 0)";

                        cmd.ExecuteNonQuery();
                    }
                }
                tran.Commit();
            }
        }

        public List<string> getUrlJobList()
        {
            var ans = new List<string>();

            //回避リストの作成
            var readCmd = conn.CreateCommand();
            readCmd.CommandText = "select url from sheetList where flg=0";
            using (var reader = readCmd.ExecuteReader())
            {
                for (var i = 0; reader.Read(); i++)
                {
                    if (reader.FieldCount != 1) { continue; }
                    ans.Add(reader[0].ToString());
                }
            }

            return ans;
        }

        public void urlJobDone(string url)
        {
            var yyyymm = "";
            var readCmd = conn.CreateCommand();
            readCmd.CommandText = "select yearmon from sheetList where url = '" + url + "'";
            using (var reader = readCmd.ExecuteReader())
            {
                reader.Read();
                yyyymm = reader[0].ToString();
            }

            //挿入処理
            using (var cmd = conn.CreateCommand())
            {
                cmd.CommandText =
                    "REPLACE INTO sheetList(yearmon, url, flg) "
                    + "VALUES(" + yyyymm + ",\"" + url + "\", 1)";

                cmd.ExecuteNonQuery();
            }

        }

        public void insertOrUpdateRaw(List<List<string>> data)
        {

            using (var tran = conn.BeginTransaction())
            {
                //データの挿入ループ
                foreach (var row in data)
                {
                    //挿入処理
                    using (var cmd = conn.CreateCommand())
                    {
                        string[] d = { "", "", "", "", "", "", "", "", "", "" };
                        for(var i=0; i<row.Count; i++)
                        {
                            d[i] = row[i];
                        }

                        var values = "";

                        DateTime dt = DateTime.Parse(d[1]);
                        d[1] = dt.ToString("yyyy/MM/dd HH:mm:ss");

                        for (var i=0; i<d.Length; i++)
                        {
                            switch (i)
                            {
                                case 0:
                                    values = d[i];
                                    break;
                                case 2:
                                    values += ", " + d[i];
                                    break;
                                default:
                                    values += ", '" + d[i] + "'";
                                    break;
                            }
                        }
                        
                        cmd.CommandText =
                            "REPLACE INTO rawTable(type, datetime, mac, d1, d2, d3, d4, d5, d6, d7) "
                            + "VALUES(" + values +")";

                        cmd.ExecuteNonQuery();
                    }
                }
                tran.Commit();
            }
        }

        public void saveToCsv(string csvPath)
        {
            var enc = Encoding.GetEncoding("Shift_JIS");
            FileInfo fi = new FileInfo(csvPath);
            if (fi.Exists) { fi.Delete(); }
            using (var sw = new StreamWriter(csvPath, true, enc))
            {
                sw.WriteLine("型,日時,MAC,D1,D2,D3,D4,D5,D6,D7");
                var readCmd = conn.CreateCommand();
                readCmd.CommandText = 
                    "select type, datetime, mac, d1, d2, d3, d4, d5, d6, d7 "
                    +"from rawTable order by datetime";
                using (var reader = readCmd.ExecuteReader())
                {
                    for (var i = 0; reader.Read(); i++)
                    {
                        for(var j=0; j< reader.FieldCount; j++)
                        {
                            sw.Write(reader[j].ToString()+",");
                        }
                        sw.WriteLine("");
                    }
                }

                sw.Close();
            }

        }
    }
}
