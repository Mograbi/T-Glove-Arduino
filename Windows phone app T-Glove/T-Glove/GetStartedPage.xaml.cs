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

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

namespace T_Glove
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class GetStartedPage : Page
    {
        public enum Choosing
        {
            CLASSIC, FREESTYLE, NONE
        }
        public Choosing choose = Choosing.NONE;
        public GetStartedPage()
        {
            this.InitializeComponent();
        }

        private void proceed_Click(object sender, RoutedEventArgs e)
        {
            if (choose == Choosing.NONE)
            {
                return;
            } else if (choose == Choosing.CLASSIC)
            {
                Frame.Navigate(typeof(ClassicPage), null);
            } else
            {
                Frame.Navigate(typeof(FreeStylePage), null);
            }
        }

        private void Classic_Checked(object sender, RoutedEventArgs e)
        {
            choose = Choosing.CLASSIC;
        }

        private void cancel_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(MainPage), null);
        }

        private void FreeStyle_Checked(object sender, RoutedEventArgs e)
        {
            choose = Choosing.FREESTYLE;
        }
    }
}
