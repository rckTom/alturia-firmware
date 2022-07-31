using Antmicro.Renode.Peripherals.SPI;
using Antmicro.Renode.Peripherals.Miscellaneous;
using System;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Utilities;

namespace Antmicro.Renode.Peripherals.SPI
{
    public class SpiCsController : SimpleContainer<ISPIPeripheral>, IGPIOReceiver, ISPIPeripheral
    {
        public SpiCsController(Machine machine) : base(machine)
        {
            chipSelected = false;
            cs_line = -1;
        }
        public byte Transmit(byte data)
        {
            if (selectedSlave != null)
            {
                return selectedSlave.Transmit(data);
            }
            
            this.Log(LogLevel.Warning, "trasnmission with no selected device");
            return 0;
        }
        public void FinishTransmission()
        {
            if (selectedSlave != null)
            {
                selectedSlave.FinishTransmission();
                return;
            }

            this.Log(LogLevel.Warning, "finish transmission with no selected device");
        }

        public override void Reset(){
            selectedSlave = null;
            cs_line = -1;
        }
        public void OnGPIO(int number, bool value)
        {
            this.Log(LogLevel.Noisy, "on gpio {0}, {1}", number, value);
            if (cs_line == -1 && !value)
            {
                //device selected
                chipSelected = true;
                cs_line = number;

                if(!TryGetByAddress(cs_line, out selectedSlave)) {
                    this.Log(LogLevel.Warning, "No device at cs_line {0}.", cs_line);
                }

                if(selectedSlave is IGPIOReceiver) {
                    ((IGPIOReceiver)selectedSlave).OnGPIO(0, value);
                }

                return;
            }
            else if ((number == cs_line) && value)
            {
                FinishTransmission();
                chipSelected = false;
                cs_line = -1;
                if(selectedSlave is IGPIOReceiver) {
                    ((IGPIOReceiver)selectedSlave).OnGPIO(0, value);
                }
                selectedSlave = null;
            } else if ((number != cs_line) && chipSelected && !value)
            {
                this.Log(LogLevel.Warning, "Trying to select spi device {0} while device {1} is already selected", number, cs_line);
            }

            return;
        }

        private ISPIPeripheral selectedSlave;
        private int cs_line;
        private bool chipSelected;
    }
}