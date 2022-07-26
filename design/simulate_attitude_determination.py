
import argparse
import sympy as sp
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import attitude_estimation as ae
from alturia import Alturialog

np.seterr("raise")

def main(args):
    qnext = ae.qnext()
    qnext_f = sp.lambdify(list(qnext.atoms(sp.Symbol, sp.MatrixSymbol)), qnext)

    #data = pd.read_csv("/home/thomas/Projects/altimeter/alturia-firmware/scripts/data/log130_0.csv")
    #data.set_index('time')

    data = Alturialog(args.data).get_track(0).to_dataframe()

    print(qnext.shape)
    q = np.zeros(qnext.shape)
    q[3] = 1.0
    last_t = data['time'][0]/1000

    offsets = data[0:5000][['gx', 'gy', 'gz']].mean()
    print(offsets.T)

    data['q0'] = pd.Series(data = None, index = data.index, dtype="float64")
    data['q1'] = pd.Series(data = None, index = data.index, dtype="float64")
    data['q2'] = pd.Series(data = None, index = data.index, dtype="float64")
    data['q3'] = pd.Series(data = None, index = data.index, dtype="float64")
    print(q)
    for step, t in enumerate(data['time']):
        t = t/1000
        dt = t-last_t
        last_t = t
        if dt < 0.001:
            continue
        omega = np.array([[data['gx'][step]-offsets['gx'],
                        data['gy'][step]-offsets['gy'],
                        data['gz'][step]-offsets['gz']]]).T
        omega = omega * 3.1459/180
        q = qnext_f(q=q, dt = dt, omega_WM_M = omega)
        data.at[data.index[step], 'q0'] = q[0]
        data.at[data.index[step], 'q1'] = q[1]
        data.at[data.index[step], 'q2'] = q[2]
        data.at[data.index[step], 'q3'] = q[3]

    plt.figure(0)
    data[['q0','q1','q2','q3']].plot()
    plt.figure(1)
    data.plot()
    plt.figure(2)
    data[["v_w", "ax", "ay", "az", "gx", "gy", "gz"]].plot()
    plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("data")

    main(parser.parse_args())