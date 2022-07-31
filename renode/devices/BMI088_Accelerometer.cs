//
// Copyright (c) 2021 Bitcraze
// Copyright (c) 2010-2020 Antmicro
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//
using System;
using System.Linq;
using System.Collections.Generic;
using Antmicro.Renode.Peripherals.Sensor;
using Antmicro.Renode.Peripherals.I2C;
using Antmicro.Renode.Peripherals.SPI;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Utilities;

namespace Antmicro.Renode.Peripherals.Sensors
{
    public class BMI088_Accelerometer : ISPIPeripheral, IProvidesRegisterCollection<ByteRegisterCollection>, ISensor, IGPIOReceiver
    {
        public BMI088_Accelerometer()
        {
            RegistersCollection = new ByteRegisterCollection(this);
            DefineRegisters();
        }


        public double AccelerationX {get; set;}
        public double AccelerationY {get; set;}
        public double AccelerationZ {get; set;}

        public void OnGPIO(int number, bool value)
        {
            if(number != 0)
            {
                this.Log(LogLevel.Warning, "This model supports only CS on pin 0, but got signal on pin {0}", number);
                return;
            }

            // value is the negated CS
            if(chipSelected && value)
            {
                FinishTransmission();
            }
            chipSelected = !value;
        }
        public void Reset()
        {
            RegistersCollection.Reset();
            this.Log(LogLevel.Noisy, "Reset registers");
        }

        public byte Transmit(byte b) {
            if (!chipSelected) {
                return 0;
            }

            if (state == State.Idle) {
                this.Log(LogLevel.Noisy, "received cmd: {0:X}", b);
                addr = (byte)(b & 0X7F);
                if ((b & 0x80) > 0) {
                    state = State.StartRead;
                    this.Log(LogLevel.Noisy, "enter read mode at addr {0:X}", addr);
                } else {
                    state = State.Write;
                    this.Log(LogLevel.Noisy, "enter write mode at addr {0:X}", addr);
                    // read mode
                }

                return 0;
            }

            if (state == State.StartRead) {
                state = State.Read;
                return 0;
            }

            if (state == State.Read) {
                byte result = RegistersCollection.Read(addr);
                this.Log(LogLevel.Noisy, "Read from address {0:X}, value: {1:X}", addr, result);
                addr++;
                return result;
            }

            if (state == State.Write) {
                this.Log(LogLevel.Noisy, "Write to address {0:X}, value: {1:X}", addr, b);
                RegistersCollection.Write(addr, b);
                return 0;
            }

            throw new ArgumentException("invalid state detected");
        }

        public void FinishTransmission()
        {
            state = State.Idle;
        }

        public ByteRegisterCollection RegistersCollection { get; }

        private void Selftest(byte val) {
            if (val == (byte)0x0D) {
                // positive selftest
                AccelerationX = 1000;
                AccelerationY = 1000;
                AccelerationZ = 500;
            } else if (val == (byte)0x09) {
                AccelerationX = -1000;
                AccelerationY = -1000;
                AccelerationZ = -500;
            } else {
                AccelerationX = 0;
                AccelerationY = 0;
                AccelerationZ = 0;
            }
        }

        private void DefineRegisters()
        {
            Registers.AccChipID.Define(this, 0x1E); //RO
            Registers.AccXLSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "ACC_X_LSB", valueProviderCallback: _ => mgToByte(AccelerationX, false)); //RO
            Registers.AccXMSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "ACC_X_MSB", valueProviderCallback: _ => mgToByte(AccelerationX, true)); //RO
            Registers.AccYLSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "ACC_Y_LSB", valueProviderCallback: _ => mgToByte(AccelerationY, false)); //RO
            Registers.AccYMSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "ACC_Y_MSB", valueProviderCallback: _ => mgToByte(AccelerationY, true)); //RO
            Registers.AccZLSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "ACC_Z_LSB", valueProviderCallback: _ => mgToByte(AccelerationZ, false)); //RO
            Registers.AccZMSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "ACC_Z_MSB", valueProviderCallback: _ => mgToByte(AccelerationZ, true)); //RO
            Registers.AccConf.Define(this, 0xA8)
                .WithValueField(0, 4, name: "acc_odr")
                .WithValueField(4, 4, name: "acc_bwp"); //RW
            Registers.AccRange.Define(this, 0x01)
                .WithValueField(0, 2, out accRange, name: "acc_range")
                .WithReservedBits(2, 6); //RW
            Registers.AccPwrConf.Define(this, 0x03)
                .WithValueField(0, 8, name: "pwr_save_mode"); //RW
            Registers.AccPwrCtrl.Define(this, 0x00)
                .WithValueField(0, 8, name: "acc_enable"); //RW
            Registers.AccSoftreset.Define(this, 0x00) //WO
                .WithValueField(0, 8, FieldMode.Write, name: "acc_soft_reset")
                .WithWriteCallback((_, val) =>
                {
                    if(val == resetCommand)
                    {
                        Reset();
                    }
                });
            Registers.AccSelfTest.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Write, name: "acc_self_test", writeCallback: (_, val) => Selftest((byte)val));
        }

        private byte addr;
        private bool chipSelected;

        private IValueRegisterField accRange;

        private const byte resetCommand = 0xB6;

        private short toRawValue(double rawData) {
            rawData = rawData * 32768 / ((double)(1000 * 1.5 * (2 << (short)accRange.Value)));
            return (short)(rawData > Int16.MaxValue ? Int16.MaxValue : rawData < Int16.MinValue ? Int16.MinValue : rawData);
        }
        private byte mgToByte(double rawData, bool msb)
        {
            short converted = toRawValue(rawData);
            return (byte)(converted >> (msb ? 8 : 0));
        }

        private State state = State.Idle;
        private enum State
        {
            Idle,
            StartRead,
            Read,
            Write
        }

        private enum Registers
        {
            AccChipID = 0x00, // Read-Only
            // 0x01 reserved
            AccErrReg = 0x02, // Read-Only
            AccStatus = 0x03, // Read-Only
            // 0x04 - 0x11 reserved
            AccXLSB = 0x12, // Read-Only
            AccXMSB = 0x13, // Read-Only
            AccYLSB = 0x14, // Read-Only
            AccYMSB = 0x15, // Read-Only
            AccZLSB = 0x16, // Read-Only
            AccZMSB = 0x17, // Read-Only
            Sensortime0 = 0x18, // Read-Only
            Sensortime1 = 0x19, // Read-Only
            Sensortime2 = 0x1A,  // Read-Only
            // 0x1B - 0x1C reserved
            AccIntStat1 = 0x1D, // Read-Only
            // 0x1E - 0x21 reserved
            TempMSB = 0x22, // Read-Only
            TempLSB = 0x23, // Read-Only
            FIFOLength0 = 0x24, // Read-Only
            FIFOLength1 = 0x25, // Read-Only
            FIFOData = 0x26, // Read-Only
            // 0x27 - 0x3F reserved
            AccConf = 0x40, // Read-Write
            AccRange = 0x41, // Read-Write
            // 0x42 - 0x44 reserved
            FIFODowns = 0x45, // Read-Write
            FIFOWTM0 = 0x46, // Read-Write
            FIFOWTM1 = 0x47, // Read-Write
            FIFOConfig0 = 0x48, // Read-Write
            FIFOConfig1 = 0x49, // Read-Write
            // 0x4A - 0x52
            Int1IOCtrl = 0x53, // Read-Write
            Int2IOCtrl = 0x54, // Read-Write
            // 0x55 - 0x57 reserved
            IntMapData = 0x58, // Read-Write
            // 0x59 - 0x6C reserved
            AccSelfTest = 0x6D, // Read-Write
            // 0x6E - 0x7B reserved
            AccPwrConf = 0x7C, // Read-Write
            AccPwrCtrl = 0x7D, // Read-Write
            AccSoftreset = 0x7E // Write-Only
        }
    }
}
