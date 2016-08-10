using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI;
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
    public sealed partial class ClassicPage : Page
    {
        public ClassicPage()
        {
            this.InitializeComponent();
            item1.Text = "Perform 5 times rolling from right to left";
            item2.Text = "Perform closing hand training 3 times (max power)";
            item3.Text = "Perform mix of rolling to left with closing hand";
        }

        private void start_Click(object sender, RoutedEventArgs e)
        {

        }

        private void stop_Click(object sender, RoutedEventArgs e)
        {

        }

        private void Back_Click(object sender, RoutedEventArgs e)
        {
            Frame.Navigate(typeof(GetStartedPage), null);
        }

        private void item1_Holding(object sender, HoldingRoutedEventArgs e)
        {
            item1.Background = new SolidColorBrush(Colors.Green);
        }

        private void item2_Holding(object sender, HoldingRoutedEventArgs e)
        {
            item2.Background = new SolidColorBrush(Colors.Green);
        }

        private void item3_Holding(object sender, HoldingRoutedEventArgs e)
        {
            item3.Background = new SolidColorBrush(Colors.Green);
        }
    }
}
