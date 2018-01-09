using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DownloadWdLog
{
    public class AppData : INotifyPropertyChanged
    {
        private string _status = "";
        public string status
        {
            get { return _status; }
            set { _status = value; notify("status"); }
        }

        private string _url = "スプレッドシートの元データURL";
        public string url
        {
            get { return _url; }
            set {
                _url = value;
                notify("url");
            }
        }

        private string _jsonFile = "";
        public string jsonFile
        {
            get { return _jsonFile; }
            set
            {
                _jsonFile = value;
                notify("jsonFile");
            }
        }

        public string sqlitePath = "";

        public AppData()
        {
        }

        public void log(string msg)
        {
            if (status.Length == 0)
            {
                status = msg;
            }else
            {
                status = msg + "\n" + status;
            }
        }

        private void notify(string name="update")
        {
            var dmy = PropertyChanged;
            if (dmy == null) { return; }
            dmy(this, new PropertyChangedEventArgs(name));
        }

        public event PropertyChangedEventHandler PropertyChanged;

    }
}
