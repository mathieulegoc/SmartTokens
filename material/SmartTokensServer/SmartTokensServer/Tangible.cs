using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SmartTokensServer
{
    class Tangible
    {
        private UInt16 id;
        private byte touchState;
        private ImuRawData imuData;

       public Tangible(UInt16 _id)
        {
            id = _id;
            touchState = 0;
        }
    }
}
