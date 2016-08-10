using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text.RegularExpressions;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Table;
using Microsoft.WindowsAzure.Storage.Auth;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace T_Glove
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class FreeStylePage : Page
    {
        private bool session_started = false;
        public FreeStylePage()
        {
            this.InitializeComponent();
            App.BTmanager.MessageReceived += BluetoothManager_MessageReceived;
            App.BTmanager.ExceptionOccured += BluetoothManager_ExceptionOccured;
        }

        private void back_Click(object sender, RoutedEventArgs e)
        {
            stop_function();
            Frame.Navigate(typeof(GetStartedPage), null);
        }

        private void start_Click(object sender, RoutedEventArgs e)
        {
            if (!session_started)
            {
                App.currentSession = new Session();
                App.currentSession.SESSION_ALIVE = true;
                start_button.Background = new SolidColorBrush(Colors.Green);
                stop_button.Background = new SolidColorBrush(Colors.LightGray);
                session_started = true;
            }
        }
        public void stop_function()
        {
            if (session_started)
            {
                List<Session.ChartParams> fingerGoodness = new List<Session.ChartParams>();
                App.lastSession = App.currentSession.copy_session();
                App.currentSession = new Session();
                stop_button.Background = new SolidColorBrush(Colors.Red);
                start_button.Background = new SolidColorBrush(Colors.LightGray);
                session_started = false;

                // BASEL debug : adding values to the session TODO : delete 

               /* App.lastSession.finger_flex_goodness.Add(50);
                App.lastSession.finger_flex_goodness.Add(60);
                App.lastSession.finger_flex_goodness.Add(70);
                App.lastSession.finger_flex_goodness.Add(80);
                App.lastSession.finger_flex_goodness.Add(90);
                */
                // BASEL debug end  
                App.lastSession.finget_flex_goodness_serialized = string.Join(",", App.lastSession.finger_flex_goodness.ToArray());
                App.lastSession.hand_roll_goodness_serialized = string.Join(",", App.lastSession.hand_roll_goodness.ToArray());
                App.lastSession.RowKey = (App.sessionIdCounter).ToString();
                App.sessionIdCounter++;
                // saving to cloud
                CloudTableClient tableClient = App.storageAccount.CreateCloudTableClient();

                CloudTable table = tableClient.GetTableReference("sessionhistory");
                table.CreateIfNotExistsAsync();

                TableOperation insertOperation = TableOperation.Insert(App.lastSession);
                table.ExecuteAsync(insertOperation);

                App.UpdateSessionIDCounter();

            }
        }
        private void stop_Click(object sender, RoutedEventArgs e)
        {
            stop_function();
        }
        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            stop_function();
        }
        private async void BluetoothManager_MessageReceived(object sender, string message)
        {
            //Debug.WriteLine("+++++++\n");
            //Debug.WriteLine(message);
            //BTmessage.Text = "+++++++\n";
            //BTmessage.Text = message;
            string expr = @"<(-?\d+),(-?\d+)>";
            MatchCollection mc = Regex.Matches(message, expr);
            foreach (Match m in mc)
            {
                //BTmessage.Text=m.ToString();
                int v1 = Int32.Parse(m.Groups[1].Value);
                int v2 = Int32.Parse(m.Groups[2].Value);
                App.currentSession.record_raw_vals(v1, v2);

            }
        }
        private async void BluetoothManager_ExceptionOccured(object sender, Exception ex)
        {
            var md = new MessageDialog(ex.Message, "We've got a problem with bluetooth...");
            md.Commands.Add(new UICommand("Ah.. thanks.."));
            md.DefaultCommandIndex = 0;
            var result = await md.ShowAsync();
        }

        private void goto_history_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(HistoryPage), null);
        }
    }
}
