using System;
using System.Drawing;
using System.Collections.Generic;
using System.Linq;
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
using MahApps.Metro.Controls;

namespace SmartTokensServer
{
    public partial class MainWindow : MetroWindow
    {

        private int[] outputDataRates = { 10, 15, 25, 30, 60, 100 };
        private SocketServer mySocketServer = null;
        private Polygon dongleIncomingTrafficArrow;
        private Polygon dongleOutgoingTrafficArrow;

        public MainWindow()
        {
            InitializeComponent();
            ODRList.ItemsSource = outputDataRates;
            ODRList.SelectedIndex = 3;

            mySocketServer = new SocketServer();

            Point[] pts = {new Point(20.0f, 10.0f),
                           new Point(10.0f, 20.0f),
                           new Point(15.0f, 20.0f),
                           new Point(15.0f, 50.0f),
                           new Point(25.0f, 50.0f),
                           new Point(25.0f, 20.0f),
                           new Point(30.0f, 20.0f),
                           new Point(20.0f, 10.0f)};

            List<Point> curvePoints = new List<Point>();
            curvePoints.AddRange(pts);
            dongleIncomingTrafficArrow = new Polygon();
            dongleIncomingTrafficArrow.Points = new PointCollection(curvePoints);
            dongleIncomingTrafficArrow.StrokeThickness = 2;
            dongleIncomingTrafficArrow.HorizontalAlignment = HorizontalAlignment.Left;
            dongleIncomingTrafficArrow.VerticalAlignment = VerticalAlignment.Center;
            DongleTrafficCanvas.Children.Add(dongleIncomingTrafficArrow);
            curvePoints.Clear();


            Point[] pts2 = {new Point(35.0f, 10.0f),
                           new Point(35.0f, 40.0f),
                           new Point(30.0f, 40.0f),
                           new Point(40.0f, 50.0f),
                           new Point(50.0f, 40.0f),
                           new Point(45.0f, 40.0f),
                           new Point(45.0f, 10.0f),
                           new Point(35.0f, 10.0f)};
            curvePoints.AddRange(pts2);
            dongleOutgoingTrafficArrow = new Polygon();
            dongleOutgoingTrafficArrow.Points = new PointCollection(curvePoints);
            dongleOutgoingTrafficArrow.StrokeThickness = 2;
            dongleOutgoingTrafficArrow.HorizontalAlignment = HorizontalAlignment.Right;
            dongleOutgoingTrafficArrow.VerticalAlignment = VerticalAlignment.Center;
            DongleTrafficCanvas.Children.Add(dongleOutgoingTrafficArrow);

            CompositionTarget.Rendering += CompositionTarget_Rendering;
        }
        void CompositionTarget_Rendering(object sender, EventArgs e)
        {
            if (mySocketServer.ClientConnected)
                ClientConnectedLabel.Content = "Client connected";
            else
                ClientConnectedLabel.Content = "No client connected";

            if (mySocketServer.DongleConnected)
                DongleConnectedLabel.Content = "MicroRobots USB Dongle \nconnected";
            else
                DongleConnectedLabel.Content = "Dongle not connected";

            Message dongleData = mySocketServer.getDongleMessage();
            if (dongleData != null)
            {
                IncomingTypeLabel.Content = MessageTypes.Name(dongleData.MessageType);
                IncomingIdLabel.Content = "Tangible #"+dongleData.SenderId;
            }

            if (mySocketServer.getDongleIncomingNumber() > 0)
            {
                dongleIncomingTrafficArrow.Stroke = Brushes.Black;
                dongleIncomingTrafficArrow.Fill = Brushes.Green;
            }
            else
            {
                dongleIncomingTrafficArrow.Stroke = Brushes.DarkGray;
                dongleIncomingTrafficArrow.Fill = Brushes.LightGray;
                dongleOutgoingTrafficArrow.Stroke = Brushes.DarkGray;
                dongleOutgoingTrafficArrow.Fill = Brushes.LightGray;
            }
        }

        protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            mySocketServer.Close();
            base.OnClosing(e);
        }

        private void ChangeODR(object sender, SelectionChangedEventArgs e)
        {
            mySocketServer.OutputDataRate = outputDataRates[ODRList.SelectedIndex];
        }

        private void Window_KeyDown(object sender, KeyEventArgs e)
        {
            byte id =(byte)DateTime.Now.Second;
            switch (e.Key)
            {
                case Key.F:
                    mySocketServer.setMessage(new Message(id, MessageTypes.StatusData, new byte[] { 0x14, 0 }));
                    break;
                case Key.T:
                    mySocketServer.setMessage(new Message(id, MessageTypes.StatusData, new byte[] { 0x12, 0 }));
                    break;
                case Key.R:
                    mySocketServer.setMessage(new Message(id, MessageTypes.StatusData, new byte[] { 0x21, 0 }));
                    break;
                case Key.X:
                    mySocketServer.setMessage(new Message(id, MessageTypes.StatusData, new byte[] { 0x34, 0 }));
                    break;
                default:
                    break;
            }
        }
    }
}
