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
    public class BMI088_Gyroscope : ISPIPeripheral, IProvidesRegisterCollection<ByteRegisterCollection>, ISensor, IGPIOReceiver
    {
        public BMI088_Gyroscope()
        {
            RegistersCollection = new ByteRegisterCollection(this);
            DefineRegisters();
        }


        public double AngularRateX {get; set;}
        public double AngularRateY {get; set;}
        public double AngularRateZ {get; set;}

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
                    state = State.Read;
                    this.Log(LogLevel.Noisy, "enter read mode at addr {0:X}", addr);
                } else {
                    state = State.Write;
                    this.Log(LogLevel.Noisy, "enter write mode at addr {0:X}", addr);
                    // read mode
                }

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

        private void DefineRegisters()
        {
            Registers.GyroChipID.Define(this, 0x0F); //RO
            Registers.RateXLSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "RATE_X_LSB", valueProviderCallback: _ => DPStoByte(AngularRateX, false)); //RO
            Registers.RateXMSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "RATE_X_MSB", valueProviderCallback: _ => DPStoByte(AngularRateX, true)); //RO
            Registers.RateYLSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "RATE_Y_LSB", valueProviderCallback: _ => DPStoByte(AngularRateY, false)); //RO
            Registers.RateYMSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "RATE_Y_MSB", valueProviderCallback: _ => DPStoByte(AngularRateY, true)); //RO
            Registers.RateZLSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "RATE_Z_LSB", valueProviderCallback: _ => DPStoByte(AngularRateZ, false)); //RO
            Registers.RateZMSB.Define(this, 0x00)
                .WithValueField(0, 8, FieldMode.Read, name: "RATE_Z_MSB", valueProviderCallback: _ => DPStoByte(AngularRateZ, true)); //RO

            Registers.GyroIntStat1.Define(this, 0x00)
                .WithReservedBits(0, 4)
                .WithFlag(4, name: "fifo_int")
                .WithReservedBits(5, 2)
                .WithFlag(7, name: "gyro_drdy"); //RO

            Registers.GyroRange.Define(this, 0x00)
                .WithValueField(0, 8, out gyroRange, name: "gyro_range"); //RW
            Registers.GyroBandwidth.Define(this, 0x80)
                .WithValueField(0, 8, name: "gyro_bw"); //RW //TODO should be used to determine output data rate
            Registers.GyroLPM1.Define(this, 0x00); //RW
            Registers.GyroSoftreset.Define(this, 0x00) //WO
                .WithWriteCallback((_, val) =>
                {
                    if(val == resetCommand)
                    {
                        Reset();
                    }
                });
            Registers.GyroIntCtrl.Define(this, 0x00)
                .WithReservedBits(0, 6)
                .WithFlag(6, out fifoEn, name: "fifo_en") // Currently unused
                .WithFlag(7, out dataEn, name: "data_en");
            Registers.Int3Int4IOConf.Define(this, 0x0F)
                .WithFlag(0, name: "int3_lvl")
                .WithFlag(1, name: "int3_od")
                .WithFlag(2, name: "int4_lvl")
                .WithFlag(3, name: "int4_od")
                .WithReservedBits(4, 4); // TODO implement?
            Registers.Int3Int4IOMap.Define(this, 0x00)
                .WithFlag(0, out int3Data, name: "int3_data")
                .WithReservedBits(1, 1)
                .WithFlag(2, out int3Fifo, name: "int3_fifo")
                .WithReservedBits(3, 2)
                .WithFlag(5, out int4Fifo, name: "int4_fifo")
                .WithReservedBits(6, 1)
                .WithFlag(7, out int4Data, name: "int4_data");
            Registers.GyroSelfTest.Define(this, 0x12); // HACK: Reset value is value to be read on succesful self test and not actual reset value.
        }
        private byte addr;
        private bool chipSelected;

        private IValueRegisterField gyroRange;

        private IFlagRegisterField dataEn;
        private IFlagRegisterField fifoEn;
        private IFlagRegisterField int3Data;
        private IFlagRegisterField int3Fifo;
        private IFlagRegisterField int4Fifo;
        private IFlagRegisterField int4Data;

        private const byte resetCommand = 0xB6;

        private State state = State.Idle;
        private enum State
        {
            Idle,
            Read,
            Write
        }

        private byte DPStoByte(double rawData, bool msb)
        {
            rawData = rawData*(double)16.384*(1<<(short)gyroRange.Value);
            short converted = (short)(rawData > Int16.MaxValue ? Int16.MaxValue : rawData < Int16.MinValue ? Int16.MinValue : rawData);
            return (byte)(converted >> (msb ? 8 : 0));
        }

        private enum Registers
        {
            GyroChipID = 0x00, // Read-Only
            // 0x01 reserved
            RateXLSB = 0x02, // Read-Only
            RateXMSB = 0x03, // Read-Only
            RateYLSB = 0x04, // Read-Only
            RateYMSB = 0x05, // Read-Only
            RateZLSB = 0x06, // Read-Only
            RateZMSB = 0x07, // Read-Only
            // 0x08 - 0x09 reserved
            GyroIntStat1 = 0x0A, // Read-Only
            // 0x0B - 0x0D reserved
            FIFOStatus = 0x0E, // Read-Only
            GyroRange = 0x0F, // Read-Write
            GyroBandwidth = 0x10, // Read-Write
            GyroLPM1 = 0x11, // Read-Write
            // 0x12 - 0x13 reserved
            GyroSoftreset = 0x14, // Write-Only
            GyroIntCtrl = 0x15, // Read-Write
            Int3Int4IOConf = 0x16, // Read-Write
            // 0x17 reserved
            Int3Int4IOMap = 0x18, // Read-Write
            // 0x19 - 0x1D reserved
            FIFOWmEn = 0x1E, // Read-Write
            // 0x1F - 0x33 reseved
            FIFOExtIntS = 0x34, // Read-Write
            // 0x35 - 0x3B reserved
            GyroSelfTest = 0x3C,
            FIFOConfig0 = 0x3D, // Read-Write
            FIFOConfig1 = 0x3E, // Read-Write
            FIFOData = 0x3F // Read-Only
        }
    }
}
