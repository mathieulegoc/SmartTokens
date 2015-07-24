using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

namespace SmartTokensServer
{
    class SocketServer
    {
        private Message dongleLastMessage;
        private Mutex dongleMessageMutex = null;
        private Socket myListener;
        private Socket myHandler;
        private bool firstClient = true;
        private bool dongleConnected = false;
        private bool clientConnected = false;
        private bool stream = true;
        private byte[] bytesBuffer = new Byte[1024];
        private int outputDataRate = 30;
        private Thread streamingThread = null;
        private Thread listeningThread = null;
        private static FTDIDevice myFtdiDevice = null;

        //Getters and setters
        #region
        public bool ClientConnected
        {
            get { return clientConnected; }
            set { clientConnected = value; }
        }
        public int OutputDataRate
        {
            get { return outputDataRate; }
            set { outputDataRate = value; }
        }
        public bool DongleConnected
        {
            get { return dongleConnected; }
            set { dongleConnected = value; }
        }
        #endregion 

        public SocketServer()
        {
            myFtdiDevice = new FTDIDevice();
            connectDongle();

            dongleMessageMutex = new Mutex();

            listeningThread = new Thread(new ThreadStart(lookForClients));
            streamingThread = new Thread(new ThreadStart(StreamData));
            streamingThread.IsBackground = true;
            listeningThread.IsBackground = true;

            streamingThread.Start();
            listeningThread.Start();
        }

        private void lookForClients()
        {
            if (firstClient)
                clientConnected = Initialise();
            else
                clientConnected = Reconnect();
        }

        private void connectDongle()
        {
            dongleConnected = myFtdiDevice.connect("MicroRobots USB Dongle", 921600, false);
        }

        private void StreamData()
        {
            while (stream)
            {
                if (dongleConnected)
                {
                    dongleMessageMutex.WaitOne();
                    dongleLastMessage = myFtdiDevice.getLastMessage();
                    dongleMessageMutex.ReleaseMutex();
                }
                else
                    connectDongle();

                if (clientConnected)
                {
                    if (dongleLastMessage!= null)
                    {
                        Send(dongleLastMessage.ToByteArray());
                        dongleLastMessage = null;
                    }
                }
                Thread.Sleep(1000 / outputDataRate);
            }
            if (clientConnected)
            {
                myHandler.Shutdown(SocketShutdown.Both);
                myHandler.Close();
            }
        }

        public void setMessage(Message m)
        {
            dongleMessageMutex.WaitOne();
            dongleLastMessage = m;
            dongleMessageMutex.ReleaseMutex();
        }

        public Message getDongleMessage()
        {
            Message tmpMsg;
            dongleMessageMutex.WaitOne();
            tmpMsg = dongleLastMessage;
            dongleMessageMutex.ReleaseMutex();
            return tmpMsg;
        }

        public void StopStreaming()
        {
            stream = false;
        }

        public void Close()
        {
            StopStreaming();
            if (myFtdiDevice.IsValid)
                myFtdiDevice.disconnect();  
        }

        private bool Initialise()
        {
            // Establish the local endpoint for the socket.
            // Dns.GetHostName returns the name of the 
            // host running the application.
            IPHostEntry ipHostInfo = Dns.Resolve(Dns.GetHostName());
            IPAddress ipAddress = ipHostInfo.AddressList[0];
            IPEndPoint localEndPoint = new IPEndPoint(ipAddress, 11000);

            // Create a TCP/IP socket.
            myListener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

            // Bind the socket to the local endpoint and 
            // listen for incoming connections.
            try
            {
                myListener.Bind(localEndPoint);
                myListener.Listen(10);

                myHandler = myListener.Accept();
                firstClient = false;
                // Program is suspended while waiting for an incoming connection.
                return true;
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
            return false;
        }

        public void Send(string data)
        {
            byte[] msg = Encoding.ASCII.GetBytes(data);
            Send(msg);
        }

        private void Send(byte[] data)
        {
            try
            {
                if(myHandler.Connected)
                  myHandler.Send(data);
            }
            catch (SocketException se)
            {
                clientConnected = false;
                listeningThread = new Thread(new ThreadStart(lookForClients));
                listeningThread.Start();
            }
            catch (Exception e)
            {
                Console.WriteLine("Unexpected exception : {0}", e.ToString());
            }
        }

        private int Receive()
        {
            int bytesRec = myHandler.Receive(bytesBuffer);
            return bytesRec;
        }

        private bool Reconnect()
        {
            try
            {
                myHandler = myListener.Accept();
                return true;
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
            return false;
        }

        public int getDongleIncomingNumber()
        {
            return myFtdiDevice.getIncomingMessagesNumber();
        }
    }
}