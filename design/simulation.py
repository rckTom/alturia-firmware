import pandas as pd
import sympy as sp
import numpy as np
import vertical_dynamics_kalman_filter as vdk
import altitude_kalman_filter as ak
import matplotlib.pyplot as plt

df = pd.read_excel("./fligthdata/altimax/tethys.ods", sheet_name="Sheet1", engine="odf")

# lambdify symbolic expressions
(x_cor, P_cor) = ak.correct()
(x_pre, P_pre) = ak.predict()

print("x_cor_args:")
print(x_cor.atoms(sp.Symbol, sp.MatrixSymbol))
print("P_cor_args:")
print(list(P_cor.atoms(sp.Symbol, sp.MatrixSymbol)))
print("x_pre_args:")
print(x_pre.atoms(sp.Symbol, sp.MatrixSymbol))
print("P_pre_args:")
print(list(P_pre.atoms(sp.Symbol, sp.MatrixSymbol)))

x_cor_f = sp.lambdify(list(x_cor.atoms(sp.Symbol, sp.MatrixSymbol)), x_cor)
P_cor_f = sp.lambdify(list(P_cor.atoms(sp.Symbol, sp.MatrixSymbol)), P_cor)
x_pre_f = sp.lambdify(list(x_pre.atoms(sp.Symbol, sp.MatrixSymbol)), x_pre)
P_pre_f = sp.lambdify(list(P_pre.atoms(sp.Symbol, sp.MatrixSymbol)), P_pre)

#calculate variance of h and a
var_h_calc = df["HEIGHT RAW [m]"][0:300].var()
var_a_calc = df["ACCEL [m/s2]"][0:300].var()
print("Variances: h = {}, a = {}".format(var_h_calc, var_a_calc))

# initialze kalman filter
variance_acc = 0.005
variance_h = 0
variance_meas_acc = var_a_calc
variance_meas_h = var_h_calc

P_pre = np.eye(P_pre.shape[0])
x_pre = np.zeros(x_pre.shape)
P_cor = np.eye(P_cor.shape[0])
x_cor = np.zeros(x_cor.shape)
print(x_cor.shape)
last_t = df["ZEIT"][0]

df["x"] = pd.Series(data=None, index = df.index, dtype="float64")
df["v"] = pd.Series(data=None, index = df.index, dtype="float64")
df["a"] = pd.Series(data=None, index = df.index, dtype="float64")
df["P_pre1"] = pd.Series(data=None, index = df.index, dtype="object")
df["P_pre2"] = pd.Series(data=None, index = df.index, dtype="object")
df["P_pre3"] = pd.Series(data=None, index = df.index, dtype="object")
df["P_pre4"] = pd.Series(data=None, index = df.index, dtype="object")
df["P_cor1"] = pd.Series(data=None, index = df.index, dtype="object")
df["P_cor2"] = pd.Series(data=None, index = df.index, dtype="object")
df["P_cor3"] = pd.Series(data=None, index = df.index, dtype="object")
df["P_cor4"] = pd.Series(data=None, index = df.index, dtype="object")

apogee = df.loc[df["EVENT"] == 9, "ZEIT"].iloc[0]
print(apogee)


for step, t in enumerate(df["ZEIT"]):
	dt = t-last_t
	last_t = t

	y = np.array([df["HEIGHT RAW [m]"][step]]).T
	
	#correct
	x_cor = x_cor_f(P_pre = P_pre, x_pre = x_pre, y = y, var_meas_altitude = variance_meas_h)
	P_cor = P_cor_f(P_pre = P_pre, var_meas_altitude=variance_meas_h)

	#predict
	x_pre = x_pre_f(x_cor = x_cor, dt = dt)
	P_pre = P_pre_f(P_cor = P_cor, dt = dt, var_process_accel = variance_acc)
	#df.at[df.index[step], "a"] = x_pre[2]

	df.at[df.index[step], "P_pre1"] = P_pre[0][0]
	df.at[df.index[step], "P_pre2"] = P_pre[1][1]
	#if t < apogee:
	#	df.at[df.index[step], "P_pre3"] = P_pre[2][2]
	#	df.at[df.index[step], "P_cor3"] = P_cor[2][2]
	df.at[df.index[step], "P_cor1"] = P_cor[0][0]
	df.at[df.index[step], "P_cor2"] = P_cor[1][1]
	df.at[df.index[step], "x"] = x_pre[0]
	df.at[df.index[step], "v"] = x_pre[1]

print(dt)
vnaive = np.gradient(df["HEIGHT RAW [m]"], 0.002)
print(vnaive)

fig, axes = plt.subplots(nrows=1, ncols=2)
df['v_naive'] = vnaive
df['v_naive_moving_avg_20'] = np.convolve(vnaive, np.ones(20)/20, mode="same")
print(df.columns)
ax = df.set_index("ZEIT")[["x", "v_naive", "v", "HEIGHT RAW [m]", "v_naive_moving_avg_20"]].plot(ax= axes[0])
l = ax.get_children()
l[0].set_zorder(4)
l[1].set_zorder(2)
l[2].set_zorder(3)
l[3].set_zorder(1)
df.set_index("ZEIT")[["P_pre1", "P_pre2", "P_pre2", "P_pre3", "P_cor1", "P_cor2", "P_cor3"]].plot(ax= axes[1])
plt.show()
