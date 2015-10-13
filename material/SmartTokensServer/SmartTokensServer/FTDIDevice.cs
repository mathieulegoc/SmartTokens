using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using FTD2XX_NET;
using System.Windows;

namespace SmartTokensServer
{

    public sealed class MessageTypes
    { 
        public static byte StatusData{ get{return 9;}}

        public static string Name(int id)
        {
            switch(id)
            {
                case 9:
                    return "StatusData";
                default:
                    return null;
            }
        }
    }

    class FTDIDevice 
    {
        private uint minimumBytesToRead = 5;
        private bool isValid = true;
        private const byte CBUSBitbang = (byte)FTDI.FT_BIT_MODES.FT_BIT_MODE_CBUS_BITBANG;
        private const uint NB_MAX_VALUES = 200;
        private FTDI.FT_STATUS ftStatus = FTDI.FT_STATUS.FT_OK;
        private FTDI FtdiDevice;
        private Mutex COMPortMutex, serialDataMutex, valuesMutex;
        private List<byte> bufferIn;
        private List<Message> incomingMessages;
        private EventWaitHandle waitForNewSerialData;
        private Thread processingThread;
        private Thread receivingThread;
        private System.IO.StreamWriter logFile;

        public bool IsValid
        {
            get { return isValid; }
        }

        public uint MinimumBytesToRead     
        {
            get { return minimumBytesToRead; }
            set { minimumBytesToRead = (byte)value; }
        }

        public enum CBUS : byte
        {
            CBUS0 = 0,
            CBUS1 = 1,
            CBUS2 = 2,
            CBUS3 = 3,
        }

        public FTDIDevice()
        {
            bufferIn = new List<byte>();
            incomingMessages = new List<Message>();
            COMPortMutex = new Mutex();
            serialDataMutex = new Mutex();
            valuesMutex = new Mutex();
            FtdiDevice = new FTDI();
        }

        public FTDIDevice(string s_description, uint u32_baudrate, bool flowCtrl)
        {
            bufferIn = new List<byte>();
            incomingMessages = new List<Message>();
            COMPortMutex = new Mutex();
            serialDataMutex = new Mutex();
            valuesMutex = new Mutex();
            FtdiDevice = new FTDI();

            connect(s_description, u32_baudrate, flowCtrl);
        }

        public bool connect(string s_description, uint u32_baudrate, bool flowCtrl)
        {
            logFile = new System.IO.StreamWriter(@".\log.txt", false);
            ftStatus = FtdiDevice.OpenByDescription(s_description);
            if (ftStatus == FTDI.FT_STATUS.FT_DEVICE_NOT_FOUND || ftStatus == FTDI.FT_STATUS.FT_INVALID_HANDLE || ftStatus == FTDI.FT_STATUS.FT_DEVICE_NOT_OPENED)
            {
                isValid = false;
                logFile.Close();
                return false;
            }

            ftStatus = FtdiDevice.SetBaudRate(u32_baudrate);
            ftStatus = FtdiDevice.SetDataCharacteristics(FTDI.FT_DATA_BITS.FT_BITS_8, FTDI.FT_STOP_BITS.FT_STOP_BITS_1, FTDI.FT_PARITY.FT_PARITY_NONE);
            if (flowCtrl)
                ftStatus = FtdiDevice.SetFlowControl(FTDI.FT_FLOW_CONTROL.FT_FLOW_RTS_CTS, 0, 0);
            else
                ftStatus = FtdiDevice.SetFlowControl(FTDI.FT_FLOW_CONTROL.FT_FLOW_NONE, 0, 0);

            ftStatus = FtdiDevice.SetTimeouts(5000, 5000);

            byte oldBitMode = 0;
            ftStatus = FtdiDevice.GetPinStates(ref oldBitMode);

            receivingThread = new Thread(new ThreadStart(USBReceivingRoutine));
            receivingThread.IsBackground = true;

            processingThread = new Thread(new ThreadStart(processIncomingData));
            processingThread.IsBackground = true;
            receivingThread.Start();
            processingThread.Start();

            waitForNewSerialData = new AutoResetEvent(false);

            return true;
        }

        private void USBReceivingRoutine()
        {
            uint numBytesAvailable=0;
            uint numBytesRead=0;

            while (true)
            {
                Thread.Sleep(0);
                if (FtdiDevice.IsOpen == true)
                {
                    COMPortMutex.WaitOne();
                    ftStatus = FtdiDevice.GetRxBytesAvailable(ref numBytesAvailable);
                    if (numBytesAvailable > MinimumBytesToRead && ftStatus == FTDI.FT_STATUS.FT_OK)
                    {
                        byte[] readData = new byte[numBytesAvailable];
                        ftStatus = FtdiDevice.Read(readData, numBytesAvailable, ref numBytesRead);
                        if (ftStatus == FTDI.FT_STATUS.FT_OK)
                        {
                            serialDataMutex.WaitOne();
                                bufferIn.AddRange(readData);
                                waitForNewSerialData.Set();
                            serialDataMutex.ReleaseMutex();
                        }
                    }
                    COMPortMutex.ReleaseMutex();
                }   
            }
        }

        private void processIncomingData()
        {
            while (true)
            {
                Thread.Sleep(0);
                if (bufferIn.Count() >= 4)          //leave some space to be able to read the payload size byte
                {
                    serialDataMutex.WaitOne();
                    {
                        if (bufferIn[0] == '~')  //if he finds the starting character
                        {
                            byte messageId = bufferIn[1];
                            byte sourceAddr = bufferIn[2];
                            byte payloadSize = bufferIn[3];

                            if ((payloadSize + 4) < bufferIn.Count()) //checks if the message can fit in the buffer
                            {
                                if (bufferIn[payloadSize + 4] == '!') //checks that the ending character is there
                                {
                                    byte[] payload = new byte[payloadSize];

                                    for (int j = 0; j < payloadSize; j++)
                                        payload[j] = bufferIn[j + 4];

                                    bufferIn.RemoveRange(0, payloadSize + 5);

                                    serialDataMutex.ReleaseMutex();

                                    valuesMutex.WaitOne();
                                        if (incomingMessages.Count() > NB_MAX_VALUES)
                                            incomingMessages.RemoveAt(0);

                                        incomingMessages.Add(new Message(sourceAddr, messageId, payload));
                                    valuesMutex.ReleaseMutex();
                                }
                                else
                                {
                                    bufferIn.RemoveAt(0);
                                    serialDataMutex.ReleaseMutex();
                                }
                            }
                            else
                            {
                                serialDataMutex.ReleaseMutex();
                                waitForNewSerialData.WaitOne();
                            }
                        }
                        else
                        {
                            logFile.Write(Convert.ToChar(bufferIn[0]));
                            bufferIn.RemoveAt(0);
                            serialDataMutex.ReleaseMutex();
                        }
                    }
                }
                else
                    waitForNewSerialData.WaitOne();
            }
        }

        public int getIncomingMessagesNumber()
        {
            return incomingMessages.Count;
        }

        public Message getLastMessage()
        {
            if(incomingMessages.Count()>0)
            {
                valuesMutex.WaitOne();
                Message msg = incomingMessages[incomingMessages.Count()-1];
                incomingMessages.RemoveAt(incomingMessages.Count() - 1);
                valuesMutex.ReleaseMutex();
                return msg;
            }
            else 
                return null;
        }

        public void reset()
        {
            resetDevice(CBUS.CBUS2);
            FtdiDevice.Purge(FTDI.FT_PURGE.FT_PURGE_RX | FTDI.FT_PURGE.FT_PURGE_TX);

             serialDataMutex.WaitOne();
                bufferIn.Clear();
            serialDataMutex.ReleaseMutex();

            valuesMutex.WaitOne();
                incomingMessages.Clear();
            valuesMutex.ReleaseMutex();

        }

        private void resetDevice(CBUS pinNumber)
        {
            byte mask = 0xFF;
            mask ^= (byte)((int)pinNumber<<1);
            FtdiDevice.SetBitMode(mask, CBUSBitbang);
            Thread.Sleep(100);
            FtdiDevice.SetBitMode(0xFF, CBUSBitbang);
        }
        
        public string[] getDevicesNames()
        {
            uint nbDevices = 0;
            ftStatus = FtdiDevice.GetNumberOfDevices(ref nbDevices);
            if(nbDevices == 0)
                return null;
            else
            {
                // Allocate storage for device info list
                FTDI.FT_DEVICE_INFO_NODE[] ftdiDeviceList = new FTDI.FT_DEVICE_INFO_NODE[nbDevices];
                // Populate our device list
                ftStatus = FtdiDevice.GetDeviceList(ftdiDeviceList);

                if(ftStatus==FTDI.FT_STATUS.FT_OK)
                {
                    string[] deviceList = new string[nbDevices];
                    for (int i = 0; i < nbDevices; i++)
                        deviceList[i] = ftdiDeviceList[i].Description.ToString();
                    return deviceList;
                }
                else
                    return null;
            }
        }

        public int getNumberDevices()
        {
            uint nbDevices = 0;          
            ftStatus = FtdiDevice.GetNumberOfDevices(ref nbDevices);
            return Convert.ToInt32(nbDevices);
        }

        public void disconnect()
        {
            if (logFile!= null) 
                logFile.Close();
            bufferIn.Clear();
            incomingMessages.Clear();
            FtdiDevice.SetRTS(false);
            if(receivingThread != null)
               receivingThread.Abort();
            if (processingThread != null)
                processingThread.Abort();
            FtdiDevice.Close();
        }

        public void writeData(byte id, byte dest, int length, ref byte[] data)
        {
            uint nbByteWritten = 0;
            if (length > 0 && data != null)
            {
                byte[] bufferOut;
                
                bufferOut = new byte[length + 5];
                bufferOut[0] = (byte)'~';
                bufferOut[1] = id;
                bufferOut[2] = dest;
                bufferOut[3] = (byte)length;
                for (int i = 0; i < length; i++)
                    bufferOut[i + 4] = data[i];
                bufferOut[length + 4] = (byte)'!';
               
                COMPortMutex.WaitOne();
                if (FtdiDevice.IsOpen)
                    FtdiDevice.Write(bufferOut, bufferOut.Length, ref nbByteWritten);
                COMPortMutex.ReleaseMutex();
            }
        }    
    }
}
