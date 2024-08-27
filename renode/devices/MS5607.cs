///
// Copyright (c) 2022 Thomas Schmid
//
// This file is licensed under the MIT License.
// Full license text is available in 'licenses/MIT.txt'.
//

// uncomment the following line to get warnings about
// accessing unhandled registers; it's disabled by default
// as it generates a lot of noise in the log and might
// hide some important messages
// 
// #define WARN_UNHANDLED_REGISTERS

using System;
using Antmicro.Renode.Core;
using Antmicro.Renode.Core.Structure.Registers;
using Antmicro.Renode.Logging;
using Antmicro.Renode.Peripherals.SPI;
using Antmicro.Renode.Peripherals.Sensor;
using Antmicro.Renode.Utilities;
using Antmicro.Renode.Peripherals.I2C;

namespace Antmicro.Renode.Peripherals.Sensors
{
    public class MS5607 : ISPIPeripheral, IProvidesRegisterCollection<ByteRegisterCollection>, IGPIOReceiver, ISensor
    {
        public ByteRegisterCollection RegistersCollection { get; }
        public MS5607()
        {
            RegistersCollection = new ByteRegisterCollection(this);
            DefineRegisters();
            InitProm();
            Temperature = 20;
            Pressure = 1000;
        }

        private void InitProm()
        {
            SENSE_T1 = 46372;
            OFF_T1 = 43981;
            TCS = 29059;
            TCO = 27842;
            T_REF = 31553;
            TEMPSENSE = 28165;
        }

        public void OnGPIO(int number, bool value)
        {
            if (number != 0)
            {
                this.Log(LogLevel.Warning, "This model supports only CS on pin 0, but got signal on pin {0}", number);
                return;
            }

            // value is the negated CS
            if (chipSelected && value)
            {
                FinishTransmission();
            }
            chipSelected = !value;
        }

        public byte Transmit(byte b)
        {
            if (!chipSelected)
            {
                this.Log(LogLevel.Warning, "Received transmission, but CS pin is not selected");
                return 0;
            }

            byte result = 0;
            switch (state)
            {
                case State.Idle:
                    result = HandleIdle(b);
                    break;

                case State.Reading_ADC:
                    this.NoisyLog("Reading adc value {0} (0x{0:X})", (Registers)address);
                    result = RegistersCollection.Read(address);

                    if (address == 2)
                    {
                        state = State.Idle;
                    }

                    address++;
                    break;
                case State.Reading_PROM:
                    this.NoisyLog("Reading prom value");
                    result = RegistersCollection.Read(address);
                    address++;
                    return result;

                default:
                    this.Log(LogLevel.Error, "Received byte in an unexpected state!");
                    break;
            }

            this.Log(LogLevel.Noisy, "Transmitting - received 0x{0:X}, sending 0x{1:X} back", b, result);
            return result;
        }

        public void FinishTransmission()
        {
            this.NoisyLog("Finishing transmission, going to the Idle state");
            state = State.Idle;
        }

        public void Reset()
        {
            RegistersCollection.Reset();
            state = State.Idle;
            address = 0;
            chipSelected = false;
            address = 0;

            InitProm();
        }

        private double temperature;
        private double pressure;
        public double Pressure
        {
            get
            {
                return pressure;
            }
            set
            {
                pressure = value;
                adc_temp = TemperatureToAdc(temperature);
                adc_pressure = PressureToAdc(value, temperature, adc_temp);
                this.Log(LogLevel.Debug, "temp_adc {0}", adc_temp);
                this.Log(LogLevel.Debug, "pressure_adc {0}", adc_temp);
            }
        }
        public double Temperature
        {
            get
            {
                return temperature;
            }
            set
            {
                temperature = value;
                adc_temp = TemperatureToAdc(value);
                adc_pressure = PressureToAdc(pressure, temperature, adc_temp);
                this.Log(LogLevel.Debug, "temp_adc {0}", adc_temp);
                this.Log(LogLevel.Debug, "pressure_adc {0}", adc_temp);
            }
        }
        public UInt16 OFF_T1 { get; set; }
        public UInt16 SENSE_T1 { get; set; }
        public UInt16 T_REF { get; set; }
        public UInt16 TCO { get; set; }
        public UInt16 TCS { get; set; }

        public UInt16 TEMPSENSE { get; set; }

        private UInt32 adc_value { get; set; }
        private UInt32 adc_temp { get; set; }
        private UInt32 adc_pressure { get; set; }

        private byte HandleIdle(byte b)
        {
            command = b;
            address = 0;

            if (command == 0x1E)
            {
                //Reset
                Reset();
            }
            else if (command == 0x00)
            {
                //ADC Read
                state = State.Reading_ADC;
                address = 0;
            }
            else if (command >= 0xA0)
            {
                // PROM Read
                address = command;
                state = State.Reading_PROM;
            }
            else if (command >= 0x40 & command <= 0x48)
            {
                // D1 Conversion (Pressure)
                adc_value = adc_pressure;
            }
            else if (command >= 0x50 & command <= 0x58)
            {
                // D2 Conversion (Temperature)
                adc_value = adc_temp;
            }
            else
            {
                throw new NotSupportedException();
            }

            return 0;
        }

        private void DefineRegisters()
        {
            Registers.adc_value_0.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "ADC[23:16]", valueProviderCallback: _ => Convert(adc_value, bytenum: 2));
            Registers.adc_value_1.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "ADC[15:8]", valueProviderCallback: _ => Convert(adc_value, bytenum: 1));
            Registers.adc_value_2.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "ADC[7:0]", valueProviderCallback: _ => Convert(adc_value, bytenum: 0));
            Registers.prom_0.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "Reservered[15:8]", valueProviderCallback: _ => 0x0);
            Registers.prom_1.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "Reservered[7:0]", valueProviderCallback: _ => 0x0);
            Registers.prom_2.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "SENSE_T1[15:8]", valueProviderCallback: _ => Convert(SENSE_T1, 1));
            Registers.prom_3.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "SENSE_T1[7:0]", valueProviderCallback: _ => Convert(SENSE_T1, 0));
            Registers.prom_4.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "OFF_T1[15:8]", valueProviderCallback: _ => Convert(OFF_T1, 1));
            Registers.prom_5.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "OFF_T1[7:0]", valueProviderCallback: _ => Convert(OFF_T1, 0));
            Registers.prom_6.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "TCS[15:8]", valueProviderCallback: _ => Convert(TCS, 1));
            Registers.prom_7.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "TCS[7:0]", valueProviderCallback: _ => Convert(TCS, 0));
            Registers.prom_8.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "TCO[15:8]", valueProviderCallback: _ => Convert(TCO, 1));
            Registers.prom_9.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "TCO[7:0]", valueProviderCallback: _ => Convert(TCO, 0));
            Registers.prom_10.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "T_REF[15:8]", valueProviderCallback: _ => Convert(T_REF, 1));
            Registers.prom_11.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "T_REF[7:0]", valueProviderCallback: _ => Convert(T_REF, 0));
            Registers.prom_12.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "TEMPSENSE[15:8]", valueProviderCallback: _ => Convert(TEMPSENSE, 1));
            Registers.prom_13.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "TEMPSENSE[7:0]", valueProviderCallback: _ => Convert(TEMPSENSE, 0));
            Registers.prom_14.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "CRC[15:8]", valueProviderCallback: _ => 0);
            Registers.prom_15.Define(this)
                .WithValueField(0, 8, FieldMode.Read, name: "CRC[7:0]", valueProviderCallback: _ => 0);
        }

        private byte Convert(UInt32 value, byte bytenum)
        {
            return (byte)(value >> (8 * bytenum));
        }

        private UInt32 PressureToAdc(double pressure, double temp, double adc_temp)
        {
            pressure = pressure * 100;
            Double pressure_adc;

            if (temp >= 20)
            {
                pressure_adc = 4194304.0 * (8388608.0 * OFF_T1 - 256.0 * TCO * T_REF + TCO * adc_temp + 2097152.0 * pressure) / (8388608.0 * SENSE_T1 - 256.0 * TCS * T_REF + TCS * adc_temp);
            }
            else if (temp >= -15)
            {
                pressure_adc = 65536 * (147573952589676412928.0 * OFF_T1 - 4503599627370496.0 * TCO * T_REF + 17592186044416.0 * TCO * adc_temp - 61 * Math.Pow(TEMPSENSE, 2) * Math.Pow(256 * T_REF - adc_temp, 2) + 36893488147419103232.0 * pressure) / (2305843009213693952.0 * SENSE_T1 - 70368744177664.0 * TCS * T_REF + 274877906944.0 * TCS * adc_temp - Math.Pow(TEMPSENSE, 2) * Math.Pow(256 * T_REF - adc_temp, 2));
            }
            else
            {
                pressure_adc = 65536.0 * (-147573952589676412928.0 * OFF_T1 + 4503599627370496.0 * TCO * T_REF - 17592186044416.0 * TCO * adc_temp + 61 * Math.Pow(TEMPSENSE, 2) * Math.Pow(256 * T_REF - adc_temp, 2) - 36893488147419103232.0 * pressure + 240 * Math.Pow(-256 * TEMPSENSE * T_REF + TEMPSENSE * adc_temp + 29360128000.0, 2)) / (-2305843009213693952.0 * SENSE_T1 + 70368744177664.0 * TCS * T_REF - 274877906944.0 * TCS * adc_temp + Math.Pow(TEMPSENSE, 2) * Math.Pow(256 * T_REF - adc_temp, 2) + 4 * Math.Pow(-256 * TEMPSENSE * T_REF + TEMPSENSE * adc_temp + 29360128000.0, 2));
            }

            return (UInt32)pressure_adc;
        }

        private UInt32 TemperatureToAdc(double temperature)
        {
            Double temp = temperature * 100;
            Double adc;

            if (temperature < 20)
            {
                adc = 128.0 * TEMPSENSE + 256.0 * T_REF - 128.0 * Math.Sqrt(Math.Pow(TEMPSENSE, 2) - 131072.0 * temp + 262144000.0);

            }
            else
            {
                adc = 256.0 * (TEMPSENSE * T_REF + 32768.0 * temp - 65536000.0) / TEMPSENSE;
            }

            return (UInt32)adc;
        }

        private byte command;
        private byte address;

        private State state;

        private bool chipSelected;

        private enum State
        {
            Idle,
            // those two states are used in SPI mode
            Reading_ADC,
            Reading_PROM
        }

        private enum Registers
        {
            adc_value_0 = 0x0,
            adc_value_1 = 0x1,
            adc_value_2 = 0x2,
            prom_0 = 0xA0,
            prom_1 = 0xA1,
            prom_2 = 0xA2,
            prom_3 = 0xA3,
            prom_4 = 0xA4,
            prom_5 = 0xA5,
            prom_6 = 0xA6,
            prom_7 = 0xA7,
            prom_8 = 0xA8,
            prom_9 = 0xA9,
            prom_10 = 0xAA,
            prom_11 = 0xAB,
            prom_12 = 0xAC,
            prom_13 = 0xAD,
            prom_14 = 0xAE,
            prom_15 = 0xAF,
        }
    }
}
