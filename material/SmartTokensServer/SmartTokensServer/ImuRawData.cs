using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SmartTokensServer
{
    class ImuRawData
    {
        private Int16 ax;
        public Int16 Ax
        {
            get { return ax; }
            set { ax = value; }
        }
        private Int16 ay;
        public Int16 Ay
        {
            get { return ay; }
            set { ay = value; }
        }
        private Int16 az;
        public Int16 Az
        {
            get { return az; }
            set { az = value; }
        }
        private Int16 gx;
        public Int16 Gx
        {
            get { return gx; }
            set { gx = value; }
        }
        private Int16 gy;
        public Int16 Gy
        {
            get { return gy; }
            set { gy = value; }
        }
        private Int16 gz;
        public Int16 Gz
        {
            get { return gz; }
            set { gz = value; }
        }
      
        public ImuRawData()
        {
            ax = ay = az = gx = gy = gz = 0;
        }

        public ImuRawData(Int16 _ax, Int16 _ay, Int16 _az, Int16 _gx, Int16 _gy, Int16 _gz)
        {
            ax = _ax;
            ay = _ay;
            az = _az;
            gx = _gx;
            gy = _gy;
            gz = _gz;
        }

        override
        public string ToString()
        {
            return "Ax=" + ax + "-Ay=" + ay + "-Az=" + az + "-Gx=" + gx + "-Gy=" + Gy + "Gz=" + gz;
        }
    }
}