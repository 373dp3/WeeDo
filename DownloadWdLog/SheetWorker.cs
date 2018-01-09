using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Google.Apis.Auth.OAuth2;
using Google.Apis.Sheets.v4;
using Google.Apis.Sheets.v4.Data;
using Google.Apis.Services;
using Google.Apis.Util.Store;
using System.Threading;
using System.IO;
using System.Text.RegularExpressions;

namespace GetWeeDoLog
{
    public class SheetWorker
    {
        static string[] Scopes = { SheetsService.Scope.SpreadsheetsReadonly };
        static string ApplicationName = "Google Sheets API .NET Quickstart";

        private UserCredential credential = null;

        public SheetWorker(string path)
        {

            using (var stream =
                new FileStream(path, FileMode.Open, FileAccess.Read))
            {
                string credPath = System.Environment.GetFolderPath(
                    System.Environment.SpecialFolder.Personal);
                credPath = Path.Combine(credPath, ".credentials/sheets.googleapis.com-dotnet-quickstart.json");

                credential = GoogleWebAuthorizationBroker.AuthorizeAsync(
                    GoogleClientSecrets.Load(stream).Secrets,
                    Scopes,
                    "user",
                    CancellationToken.None,
                    new FileDataStore(credPath, true)).Result;
            }


        }

        public static string sheetUrlToId(string sheet_url)
        {
            var ary = sheet_url.Split('/');
            var id = "";
            for(var i=0; i<ary.Length-1; i++)
            {
                if (ary[i].CompareTo("d") == 0)
                {
                    id = ary[i + 1];
                    return id;
                }
            }
            throw new Exception("URLに/d/～～が含まれていません URLを再確認してください。");
            return id;
        }

        public List<List<string>> getRawInfo(string sheet_url)
        {
            return getInfo(sheet_url, "raw!A5:J");
        }
        public List<List<string>> getHistoryInfo(string sheet_url)
        {
            return getInfo(sheet_url, "一覧!A2:B");
        }

        private List<List<string>> getInfo(string sheet_url, string rangeA0)
        {
            if (credential == null) { throw new Exception("認証情報の取得に失敗しました。credential null"); }
            List<List<string>> ans = new List<List<string>>();

            var service = new SheetsService(new BaseClientService.Initializer()
            {
                HttpClientInitializer = credential,
                ApplicationName = ApplicationName,
            });

            String spreadsheetId = sheetUrlToId(sheet_url);
            String range = rangeA0;
            //            String range = "一覧!A1:J";
            SpreadsheetsResource.ValuesResource.GetRequest request =
                    service.Spreadsheets.Values.Get(spreadsheetId, range);

            ValueRange response = request.Execute();
            IList<IList<Object>> values = response.Values;
            if (values != null && values.Count > 0)
            {
                foreach (var row in values)
                {
                    List<string> list = new List<string>();

                    foreach (var col in row)
                    {
                        list.Add(col.ToString());
                    }
                    ans.Add(list);
                }
            }
            return ans;
        }


    }
}
