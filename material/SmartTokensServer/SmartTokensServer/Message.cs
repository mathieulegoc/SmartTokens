using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SmartTokensServer
{
    class Message
    {
        private byte messageType;
        private byte senderId;
        private byte[] payload;

        public byte MessageType
        {
            get { return messageType; }
            set { messageType = value; }
        }
        public byte SenderId
        {
            get { return senderId; }
            set { senderId = value; }
        }
        public byte[] Payload
        {
            get { return payload; }
            set { payload = value; }
        }

        public Message(byte sender, byte type, byte[] data)
        {
            senderId = sender;
            messageType = type;
            payload = data;
        }

        public byte[] ToByteArray()
        {
            byte[] array = new byte[payload.Length+2];
            array[0] = messageType;
            array[1] = (byte)senderId;
            Array.ConstrainedCopy(payload, 0, array, 2, payload.Length);
            return array;
        }


        public string IntoString()
        {
            string s = string.Empty;

            if(senderId!='\n')
                s += (char)senderId;
            else
                s += "\\n";

            if(messageType!='\n')
                s += (char)messageType;
            else
                s += "\\n";
        
            for (int i = 0; i < payload.Length; i++)
            {
                if (payload[i] == '\n')
                    s += "\\n";
                else
                    s += (char)payload[i];
            }

            return s;
        }

        public string ToHexString()
        {
            string s = string.Empty;

            s += "0x" + senderId.ToString("X") + " ";
            s += "0x" + messageType.ToString("X") + " ";

            for (int i = 0; i < payload.Length; i++)
            {
                if (payload[i] == '\n')
                    s += "\\n";
                else
                    s += "0x" + payload[i].ToString("X") + " ";
            }
            return s;
        }
    }
}
