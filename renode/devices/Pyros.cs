using System;
using System.Linq;
using System.Collections.Generic;
using Antmicro.Renode.Peripherals.Bus;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Utilities;

namespace Antmicro.Renode.Peripherals
{
    public class Pyros : IGPIOReceiver, IBytePeripheral, IKnownSize{
        private HashSet<int> fired_pyros;
        public long Size => 0x0;

        public Pyros() {
            fired_pyros = new HashSet<int>();
        }

        public void OnGPIO(int number, bool value)
        {
            if (value) {
                fired_pyros.Add(number);
            }
        }

        public bool IsSet(int number)
        {
            return fired_pyros.Contains(number);
        }

        public byte ReadByte(long addr) {
            return (byte)0;
        }

        public void WriteByte(long addr, byte value) {
            return;
        }

        public void Reset() {
            fired_pyros = new HashSet<int>();
        }
    }
}
