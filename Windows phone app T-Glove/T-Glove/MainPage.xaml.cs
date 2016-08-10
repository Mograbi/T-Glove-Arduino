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
using Windows.Storage.Streams;
using Newtonsoft.Json.Linq;
using TCD.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Windows.UI;
using Windows.UI.Popups;
using Windows.Networking.Proximity;
using System.Diagnostics;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using Windows.Devices.Enumeration;
using myB;
using System.Threading.Tasks;
using System.Text.RegularExpressions;
using Windows.Devices.Bluetooth.Rfcomm;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

namespace T_Glove
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainPage : Page
    {
        public enum BTStatus
        {
            CONNECTED, DISCONNECTED, CONNECTING
        }
        public static BTStatus bt_status = BTStatus.DISCONNECTED;
        public static string bt_value = string.Empty;
        public MainPage()
        {
            this.InitializeComponent();
            BT_Status.Text = "Disconnected";
        }
        private void update_bt_status(BTStatus status)
        {
            switch (status)
            {
                case BTStatus.DISCONNECTED:
                    BT_Status.Background = new SolidColorBrush(Colors.Red);
                    BT_Status.Text = "Disconnected";
                    bt_status = BTStatus.DISCONNECTED;
                    break;
                case BTStatus.CONNECTED:
                    BT_Status.Background = new SolidColorBrush(Colors.Green);
                    BT_Status.Text = "Connected";
                    bt_status = BTStatus.CONNECTED;
                    break;
                case BTStatus.CONNECTING:
                    BT_Status.Background = new SolidColorBrush(Colors.Blue);
                    BT_Status.Text = "Connecting...";
                    bt_status = BTStatus.CONNECTING;
                    break;

            }
        }
        private async void bt_Connect_Click(object sender, RoutedEventArgs e)
        {
            update_bt_status(BTStatus.CONNECTING);
            await App.BTmanager.EnumerateDevicesAsync((sender as Button).GetElementRect());

            Debug.WriteLine("======\n" + App.BTmanager.State.ToString());
            if (App.BTmanager.State.ToString() != "Connected")
            {
                update_bt_status(BTStatus.DISCONNECTED);
                return;
            }
            update_bt_status(BTStatus.CONNECTED);

            //BTstatus.Text = App.BTmanager.State.ToString();
            /*var serviceInfoCollection = await DeviceInformation.FindAllAsync(RfcommDeviceService.GetDeviceSelector(RfcommServiceId.SerialPort));
            Debug.WriteLine("bla bla bla");
            foreach (var serviceInfo in serviceInfoCollection)
                Debug.WriteLine(serviceInfo.Id + "    " + serviceInfo.Name + "////////////////" + serviceInfo.ToString());
            Debug.WriteLine("bla bla bla end");
            PopupMenu menu = new PopupMenu();
            BluetoothConnectionManager manager = new BluetoothConnectionManager();
            foreach (var serviceInfo in serviceInfoCollection)
            {
                menu.Commands.Add(new UICommand(serviceInfo.Name, new UICommandInvokedHandler(delegate (IUICommand command) { Task connect =manager.ConnectToServiceAsync(command); }), serviceInfo));
               // menu.Commands.Add(new UICommand(serviceInfo.Name, new UICommandInvokedHandler(delegate (IUICommand command) { Debug.WriteLine(command); }), serviceInfo));
            }
            //var result = await menu.ShowForSelectionAsync();
            var res = await menu.ShowForSelectionAsync((sender as Button).GetElementRect());
            //await App.BluetoothManager.EnumerateDevicesAsync();*/

        }

        private void bt_Disconnect_Click(object sender, RoutedEventArgs e)
        {
            if (App.BTmanager.State != BluetoothConnectionState.Disconnected)
            {
                App.BTmanager.AbortConnection();
            }
                
            update_bt_status(BTStatus.DISCONNECTED);
        }

        private async void get_started_Click(object sender, RoutedEventArgs e)
        {
           if (App.BTmanager.State != BluetoothConnectionState.Connected)
            {
                var messageDialog = new MessageDialog("Phone is not connected to T-Glove\nPlease connect via Bluetooth");
                await messageDialog.ShowAsync();
                return;
            }
            update_bt_status(BTStatus.CONNECTED);
            Frame.Navigate(typeof(GetStartedPage), null);
        }
        private void show_history_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(HistoryPage), null);
        }
        //Assign textbox value to variable on page leaving event
        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            if (!string.IsNullOrEmpty(BT_Status.Text))
            {
                bt_value = BT_Status.Text;
            }
        }
        //Get value from page load
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            if (!string.IsNullOrEmpty(bt_value))
            {
                BT_Status.Text = bt_value;
                if (bt_value == "Connected")
                {
                    update_bt_status(BTStatus.CONNECTED);
                }
                else
                {
                    update_bt_status(BTStatus.DISCONNECTED);
                }
            }
        }

        private void BT_Disconnect_Holding(object sender, HoldingRoutedEventArgs e)
        {
            
            App.BTmanager.Disconnect();
            update_bt_status(BTStatus.DISCONNECTED);
        }
    }
}
