
import sympy as sp
import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import attitude_estimation as ae

np.seterr("raise")
qnext = ae.qnext()
qnext_f = sp.lambdify(list(qnext.atoms(sp.Symbol, sp.MatrixSymbol)), qnext)

data = pd.read_csv("./fligthdata/alturia/flickflack.csv")
data.set_index('time')

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
plt.show()


