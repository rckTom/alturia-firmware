import argparse

import numpy as np
import sympy as sp
import pandas as pd
import matplotlib.pyplot as plt
from sympy.utilities.lambdify import lambdastr
from alturia import Alturialog

from mahony_filter import outputs

np.seterr("raise")

def main(args):
    log = Alturialog(args.flight_data)
    
    q_next, gyro_bias = outputs()

    q_next_func = sp.lambdify(list(q_next.atoms(sp.Symbol, sp.MatrixSymbol)), q_next)
    gyro_bias_func = sp.lambdify(list(q_next.atoms(sp.Symbol, sp.MatrixSymbol)), gyro_bias)

    sensor_data = log.get_track(0).to_dataframe()
    gyro_bias = np.zeros((3, 1))
    print(gyro_bias)
    q = np.zeros((4, 1))
    q[0] = 1.0
    dt = 0.01
    ki = 2
    kp = 50

    qs = list()
    biases = list()
    last_t = sensor_data["time"][0]

    for step, t in enumerate(sensor_data['time']):
        t = t/1000
        dt = t-last_t
        last_t = t
        if dt < 0.001:
            continue
        sample = sensor_data.iloc[step]
        a = np.array([[sample.ax, sample.ay, sample.az]]).T
        #a = np.zeros((3, 1))
        g = np.array([[sample.gx, sample.gy, sample.gz]]).T
        q_new = q_next_func(a_m = a, g_m = g, dt = dt, gyro_bias = gyro_bias, ki = ki, kp = kp, q = q)
        gyro_bias = gyro_bias_func(a_m = a, g_m = g, dt = dt, gyro_bias = gyro_bias, ki = ki, kp = kp, q = q)

        q = q_new
        qs.append(q.squeeze())
        biases.append(gyro_bias.squeeze())

    q_sim = pd.DataFrame.from_records(qs)
    offset_sim = pd.DataFrame.from_records(biases)
    q_sim.plot()
    offset_sim.plot()
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("flight_data")

    main(parser.parse_args())
