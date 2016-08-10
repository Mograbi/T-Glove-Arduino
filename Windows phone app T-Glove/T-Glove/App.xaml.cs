using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.ApplicationModel;
using Windows.ApplicationModel.Activation;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using Microsoft.WindowsAzure.MobileServices;
using Windows.Devices.Bluetooth.Rfcomm;
using Windows.Storage.Streams;
using Windows.Networking.Sockets;
using Microsoft.WindowsAzure.Storage;
using Microsoft.WindowsAzure.Storage.Table;
using Microsoft.WindowsAzure.Storage.Auth;
using myB;

namespace T_Glove
{
    public class bluetoothConnectionParams
    {
        public RfcommDeviceService chatService;
        public StreamSocket chatSocket;
        public DataWriter chatWriter;
        public DataReader chatReader;
        public bool isConnectedToBluetooth;
    }
    //Basel
    public class SessionId : TableEntity
    {
        public  Int64 id{get;set;}

        public SessionId()
        {
            id = -1;
            this.PartitionKey = "default";
            this.RowKey = "0";
        }
        public SessionId(Int64 id) {
            this.id = id;
            this.PartitionKey = "default";
            this.RowKey = "0";
        }
        public static SessionId operator++(SessionId IDcounter)
        {
            IDcounter.id++;
            return IDcounter;
        }
        override public  string ToString()
        {
            return id.ToString();
        }
    }


    /// <summary>
    /// Provides application-specific behavior to supplement the default Application class.
    /// </summary>
    sealed partial class App : Application
    {
        public static MobileServiceClient myService = new MobileServiceClient("https://spider.azure-mobile.net/", "XgOVRpLAmIIxaPJSnaxSdNTNFnevVh41");
        public static bluetoothConnectionParams connectionParams = new bluetoothConnectionParams();
        public static BluetoothConnectionManager BTmanager = new BluetoothConnectionManager();
        public static Session currentSession = new Session();
        public static Session lastSession = new Session();
        public static int num;
        //public static Int64 SessionIdCounter = 0;
        public static SessionId sessionIdCounter = new SessionId();
        //public static BluetoothManager manager = new BluetoothManager();
        public static BluetoothConnectionManager BluetoothManager { get { return App.Current.Resources["BluetoothManager"] as BluetoothConnectionManager; } }
        public static CloudStorageAccount storageAccount;

        /// <summary>
        /// Initializes the singleton application object.  This is the first line of authored code
        /// executed, and as such is the logical equivalent of main() or WinMain().
        /// </summary>
         public App()
        {
            Microsoft.ApplicationInsights.WindowsAppInitializer.InitializeAsync(
                Microsoft.ApplicationInsights.WindowsCollectors.Metadata |
                Microsoft.ApplicationInsights.WindowsCollectors.Session);
            this.InitializeComponent();
            this.Suspending += OnSuspending;
            StorageCredentials credentials = new StorageCredentials("tglove", "eUebvIJBbRcirbeoWX+x3/S87jbNoiOaSRQw+umkjYdWn1Lz/7Z8L98IQiiNBYQ4e4bdewF06EH7ZrCdvKXpPw==");
            storageAccount = new CloudStorageAccount(credentials, true);
            InitSessionIDCounter();
            

        }

        async private  void InitSessionIDCounter()
        {
            CloudTableClient tableClient = App.storageAccount.CreateCloudTableClient();

            CloudTable table = tableClient.GetTableReference("sessionidcounter");
            await table.CreateIfNotExistsAsync();
            // Create a retrieve operation that takes a customer entity.
            TableOperation retrieveOperation = TableOperation.Retrieve<SessionId>("default", "0");

            // Execute the operation.
            TableResult retrievedResult = await table.ExecuteAsync(retrieveOperation);

            // Assign the result to a CustomerEntity object.
            SessionId sessionid = (SessionId)retrievedResult.Result;
            App.sessionIdCounter = new SessionId(1);
            if (sessionid != null)
            {
                App.sessionIdCounter.id = sessionid.id;
            }
            else
            {
                TableOperation insertOrReplaceOperation = TableOperation.InsertOrReplace(App.sessionIdCounter);
                await table.ExecuteAsync(insertOrReplaceOperation);
            }

        }
        async public static void UpdateSessionIDCounter()
        {
            CloudTableClient tableClient = App.storageAccount.CreateCloudTableClient();

            CloudTable table = tableClient.GetTableReference("sessionidcounter");
            await table.CreateIfNotExistsAsync();
            // Assign the result to a CustomerEntity object.
            TableOperation insertOrReplaceOperation = TableOperation.InsertOrReplace(App.sessionIdCounter);
            await table.ExecuteAsync(insertOrReplaceOperation);
        }

        /// <summary>
        /// Invoked when the application is launched normally by the end user.  Other entry points
        /// will be used such as when the application is launched to open a specific file.
        /// </summary>
        /// <param name="e">Details about the launch request and process.</param>
        protected override void OnLaunched(LaunchActivatedEventArgs e)
        {
#if DEBUG
            if (System.Diagnostics.Debugger.IsAttached)
            {
                this.DebugSettings.EnableFrameRateCounter = true;
            }
#endif
            Frame rootFrame = Window.Current.Content as Frame;

            // Do not repeat app initialization when the Window already has content,
            // just ensure that the window is active
            if (rootFrame == null)
            {
                // Create a Frame to act as the navigation context and navigate to the first page
                rootFrame = new Frame();

                rootFrame.NavigationFailed += OnNavigationFailed;

                if (e.PreviousExecutionState == ApplicationExecutionState.Terminated)
                {
                    //TODO: Load state from previously suspended application
                }

                // Place the frame in the current Window
                Window.Current.Content = rootFrame;
            }

            if (e.PrelaunchActivated == false)
            {
                if (rootFrame.Content == null)
                {
                    // When the navigation stack isn't restored navigate to the first page,
                    // configuring the new page by passing required information as a navigation
                    // parameter
                    rootFrame.Navigate(typeof(MainPage), e.Arguments);
                }
                // Ensure the current window is active
                Window.Current.Activate();
            }
        }


        /// <summary>
        /// Invoked when Navigation to a certain page fails
        /// </summary>
        /// <param name="sender">The Frame which failed navigation</param>
        /// <param name="e">Details about the navigation failure</param>
        void OnNavigationFailed(object sender, NavigationFailedEventArgs e)
        {
            throw new Exception("Failed to load Page " + e.SourcePageType.FullName);
        }

        /// <summary>
        /// Invoked when application execution is being suspended.  Application state is saved
        /// without knowing whether the application will be terminated or resumed with the contents
        /// of memory still intact.
        /// </summary>
        /// <param name="sender">The source of the suspend request.</param>
        /// <param name="e">Details about the suspend request.</param>
        private void OnSuspending(object sender, SuspendingEventArgs e)
        {
            var deferral = e.SuspendingOperation.GetDeferral();
            //TODO: Save application state and stop any background activity
            deferral.Complete();
        }
    }
}
