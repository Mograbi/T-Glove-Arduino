using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using WinRTXamlToolkit.Controls.DataVisualization.Charting;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Table;
using Microsoft.WindowsAzure.Storage.Auth;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace T_Glove
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class HistoryPage : Page
    {
        public List<Session.ChartParams> Global_History_ROLL = new List<Session.ChartParams>();
        public List<Session.ChartParams> Global_History_FIST = new List<Session.ChartParams>();
        public enum HistoryDisplayMode { ROLL,FIST,NONE };
        public enum HistorySource { GLOBAL,LAST,NONE};
        public HistoryDisplayMode history_display_mode = HistoryDisplayMode.NONE;
        public HistorySource history_source = HistorySource.NONE;
        public HistoryPage()
        {
            this.InitializeComponent();
        }

        private void back_to_main_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(MainPage), null);
        }
        /*private void draw(Session session)
        {
            if (session != null)
            {

                List<Session.ChartParams> fistGoodness = session.build_chart(Session.MOVMENT_MODE.FIST);
                (FistColumn.Series[0] as ColumnSeries).ItemsSource = fistGoodness;
                List<Session.ChartParams> rollGoodness = session.build_chart(Session.MOVMENT_MODE.ROLL);
                (RollColumn.Series[0] as ColumnSeries).ItemsSource = rollGoodness;
            }
        }*/
        private void last_training_radio_Checked(object sender, RoutedEventArgs e)
        {
            history_source = HistorySource.LAST;
            display_last();
        }
        private void display_last()
        {
            List<Session.ChartParams> fistGoodness = App.lastSession.build_chart(Session.MOVMENT_MODE.FIST);
            List<Session.ChartParams> rollGoodness = App.lastSession.build_chart(Session.MOVMENT_MODE.ROLL);
            display_chart(fistGoodness, rollGoodness);
        }

        private void regresh_global_history(object sender, RoutedEventArgs e)
        {
            Update_global_history();
        }
        private async void Update_global_history()
        {
            CloudTableClient tableClient = App.storageAccount.CreateCloudTableClient();

            CloudTable table = tableClient.GetTableReference("sessionhistory");

            TableQuery<Session> query = new TableQuery<Session>().Where(TableQuery.GenerateFilterCondition("PartitionKey", QueryComparisons.Equal, "default"));
            TableQuerySegment<Session> segment = null;
            TableContinuationToken token = null;
            do
            {
                segment = await table.ExecuteQuerySegmentedAsync<Session>(query, token);
                if (segment == null)
                {
                    break;
                }
                token = segment.ContinuationToken;
                int i = 0;
                foreach (Session session in segment.Results)
                {
                    //session.finger_flex_goodness= (session.finget_flex_goodness_serialized != null)?session.finget_flex_goodness_serialized.Split(',').Select(Int32.Parse).ToList():new List<int>();

                    if (!string.IsNullOrEmpty(session.finget_flex_goodness_serialized))
                    {
                        session.finger_flex_goodness = (session.finget_flex_goodness_serialized != null) ? session.finget_flex_goodness_serialized.Split(',').Select(Int32.Parse).ToList() : new List<int>();
                    }
                    //  session.hand_roll_goodness = (session.hand_roll_goodness_serialized != null)?session.hand_roll_goodness_serialized.Split(',').Select(Int32.Parse).ToList(): new List<int>();
                    if (!string.IsNullOrEmpty(session.hand_roll_goodness_serialized))
                    {
                        session.hand_roll_goodness = (session.hand_roll_goodness_serialized != null) ? session.hand_roll_goodness_serialized.Split(',').Select(Int32.Parse).ToList() : new List<int>();
                    }
                    int average_ROLL = session.GetGoodnessAverage(Session.MOVMENT_MODE.ROLL);
                    int average_FIST = session.GetGoodnessAverage(Session.MOVMENT_MODE.FIST);
                    i++;
                    Global_History_ROLL.Add(new Session.ChartParams() { Name = "S" + i.ToString(), Amount = average_ROLL });
                    Global_History_FIST.Add(new Session.ChartParams() { Name = "S" + i.ToString(), Amount = average_FIST });
                }
            }
            while (token != null);
        }

        private void display_global_history()
        {
            display_chart(Global_History_FIST, Global_History_ROLL);
        }
        private void display_chart(List<Session.ChartParams> fist, List<Session.ChartParams> roll)
        {
            if (history_display_mode == HistoryDisplayMode.FIST)
            {
                (FistColumn.Series[0] as ColumnSeries).ItemsSource = fist;
                FistColumn.Visibility = Windows.UI.Xaml.Visibility.Visible;
                RollColumn.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            }
            else
            {
                (RollColumn.Series[0] as ColumnSeries).ItemsSource = roll;
                RollColumn.Visibility = Windows.UI.Xaml.Visibility.Visible;
                FistColumn.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            }
        }
        private void all_history_radio_Checked(object sender, RoutedEventArgs e)
        {
            history_source = HistorySource.GLOBAL;
            Update_global_history();
            display_global_history();
        }


        private void show_roll_checked(object sender, RoutedEventArgs e)
        {
            history_display_mode = HistoryDisplayMode.ROLL;
            if(history_source == HistorySource.LAST)
            {
                display_last();
            }
            else if(history_source == HistorySource.GLOBAL)
            {
                Update_global_history();
                display_global_history();
            }
        }
        private void show_fist_Checked(object sender, RoutedEventArgs e)
        {
            history_display_mode = HistoryDisplayMode.FIST;
            if (history_source == HistorySource.LAST)
            {
                display_last();
            }
            else if (history_source == HistorySource.GLOBAL)
            {
                Update_global_history();
                display_global_history();
            }
        }

        private void Grid_Loaded(object sender, RoutedEventArgs e)
        {
            List<Session.ChartParams> emty_list = new List<Session.ChartParams>();
            (FistColumn.Series[0] as ColumnSeries).ItemsSource = emty_list;
            (RollColumn.Series[0] as ColumnSeries).ItemsSource = emty_list;
        }
        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            Windows.Graphics.Display.DisplayInformation.AutoRotationPreferences = Windows.Graphics.Display.DisplayOrientations.Portrait;
        }
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            Windows.Graphics.Display.DisplayInformation.AutoRotationPreferences = Windows.Graphics.Display.DisplayOrientations.Landscape;
        }

    }
}
