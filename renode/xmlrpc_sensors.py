import xmlrpclib

machine = monitor.Machine
acc_dev = machine["sysbus.spi2.cs_controller.bmi088_acc"]
gyro_dev = machine["sysbus.spi2.cs_controller.bmi088_gyr"]
press_dev = machine["sysbus.spi2.cs_controller.ms5607"]
sim = None
t = 0

def set_sensor_values(pressure, temperature, acceleration, angular_rate):
    press_dev.Temperature = temperature 
    press_dev.Pressure = pressure

    acc_dev.AccelerationX = acceleration[0]
    acc_dev.AccelerationY = acceleration[1]
    acc_dev.AccelerationZ = acceleration[2]

    gyro_dev.AngularRateX = angular_rate[0]
    gyro_dev.AngularRateY = angular_rate[1]
    gyro_dev.AngularRateZ = angular_rate[2]

def run():
    global t
    if sim.is_running():
        acc = sim.get_acceleration_data()
        gyr = sim.get_angular_velocity()
        press = sim.get_pressure()

        set_sensor_values(press, 20, acc, gyr)
        t+=0.01

        #get pyro channels
        pyros = [False, False, False, False]
        pyro_dev = machine["sysbus.pyros"]
        for i in range(0, len(pyros)):
                pyros[i] = pyro_dev.IsSet(i)
        
        sim.set_pyros(pyros)
        sim.run_to(t)
    else:
        monitor.parse("pause")

def setup():
    global sim
    sim = xmlrpclib.ServerProxy("http://localhost:8080")
    sim.reset()
    t = machine.ObtainManagedThread(run, 100)
    t.Start()
