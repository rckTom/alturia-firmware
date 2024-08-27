import argparse

import numpy as np
import sympy as sp
import pandas as pd
import matplotlib.pyplot as plt
from sympy.utilities.lambdify import lambdastr
from alturia import Alturialog

import ahrs

np.seterr("raise")

def main(args):
    log = Alturialog(args.flight_data)
    
    mahony = ahrs.filters.Mahony(k_I=0, k_P = 0)

    sensor_data = log.get_track(0).to_dataframe()
    gyro_bias = np.zeros((3, 1))
    print(gyro_bias)
    q = np.zeros((4,))
    q[0] = 1.0
    dt = 0.01
    ki = 0.05
    kp = 0.1

    qs = list()
    last_t = sensor_data["time"][0]

    for step, t in enumerate(sensor_data['time']):
        t = t/1000
        dt = t-last_t
        last_t = t
        if dt < 0.001:
            continue
        sample = sensor_data.iloc[step]
        a = np.array([sample.ax, sample.ay, sample.az])
        g = np.array([sample.gx, sample.gy, sample.gz])
        q = mahony.updateIMU(q, g, a)
        qs.append(q.copy())

    print(qs)
    q_sim = pd.DataFrame.from_records(qs)
    q_sim.plot()
    plt.show()

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("flight_data")

    main(parser.parse_args())
