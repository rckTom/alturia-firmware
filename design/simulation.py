import pandas as pd
import sympy as sp
import numpy as np
import vertical_dynamics_kalman_filter as vdk
import altitude_kalman_filter as ak
import matplotlib.pyplot as plt

df = pd.read_excel("/home/thomas/Projects/altimeter/fligthdata/altimax/tethys.ods", sheet_name="Sheet1", engine="odf")

# lambdify symbolic expressions
(x_cor, P_cor) = vdk.correct()
(x_pre, P_pre) = vdk.predict()

(x_cor_2, P_cor_2) = ak.correct()
(x_pre_2, P_pre_2) = ak.predict()

x_cor_f = sp.lambdify(list(x_cor.atoms(sp.Symbol)), x_cor)
P_cor_f = sp.lambdify(list(P_cor.atoms(sp.Symbol)), P_cor)
x_pre_f = sp.lambdify(list(x_pre.atoms(sp.Symbol)), x_pre)
P_pre_f = sp.lambdify(list(P_pre.atoms(sp.Symbol)), P_pre)

x_cor2_f = sp.lambdify(list(x_cor_2.atoms(sp.Symbol)), x_cor_2)
P_cor2_f = sp.lambdify(list(P_cor_2.atoms(sp.Symbol)), P_cor_2)
x_pre2_f = sp.lambdify(list(x_pre_2.atoms(sp.Symbol)), x_pre_2)
P_pre2_f = sp.lambdify(list(P_pre_2.atoms(sp.Symbol)), P_pre_2)

#calculate variance of h and a
var_h_calc = df["HEIGHT RAW [m]"][0:300].var()
var_a_calc = df["ACCEL [m/s2]"][0:300].var()
print("Variances: h = {}, a = {}".format(var_h_calc, var_a_calc))


# initialze kalman filter
variance_acc = var_h_calc
variance_h = 0
variance_meas_acc = var_a_calc
variance_meas_h = 1

P_pre = np.eye(P_pre.shape[0])
x_pre = np.zeros(x_pre.shape)
P_cor = np.eye(P_cor.shape[0])
x_cor = np.zeros(x_cor.shape)

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

x_pre = np.matrix([x_pre[0], x_pre[1]])
P_pre = np.ones(P_pre_2.shape)


for step, t in enumerate(df["ZEIT"]):
	dt = t-last_t
	last_t = t

	if t >= 0:
		y = df["HEIGHT RAW [m]"][step]

		#if t == apogee:
		#	x_pre = np.matrix([x_pre[0], x_pre[1]])
		#	P_pre = np.ones(P_pre_2.shape)

		if t >= apogee and t <= apogee + 0.01:
			y = -200
			df.at[df.index[step], "HEIGHT RAW [m]"] = -200
		x_cor = x_cor2_f(P_pre = P_pre, x_pre = x_pre, y = y, var_meas_altitude = variance_meas_h)
		P_cor = P_cor2_f(P_pre = P_pre, var_meas_altitude = variance_meas_h)

		x_pre = x_pre2_f(x_cor = x_cor, dt = dt)
		P_pre = P_pre2_f(P_cor = P_cor, dt = dt, var_process_accel=0.001)

	else:
		if t >= 2000 and t <= 4000:
			df.at[df.index[step], "HEIGHT RAW [m]"] = 0

		y = np.array([[df["HEIGHT RAW [m]"][step], df["ACCEL [m/s2]"][step]]]).T

		#correct
		x_cor = x_cor_f(P_pre = P_pre, x_pre = x_pre, y = y, variance_meas_a = variance_meas_acc, variance_meas_h = variance_meas_h)
		P_cor = P_cor_f(P_pre = P_pre, variance_meas_a = variance_meas_acc, variance_meas_h = variance_meas_h)

		#predict
		x_pre = x_pre_f(x_cor = x_cor, dt = dt)
		P_pre = P_pre_f(P_cor = P_cor, dt = dt, variance_acc = variance_acc, variance_h = variance_h)
		df.at[df.index[step], "a"] = x_pre[2]

	df.at[df.index[step], "P_pre1"] = P_pre[0][0]
	df.at[df.index[step], "P_pre2"] = P_pre[1][1]
	#if t < apogee:
	#	df.at[df.index[step], "P_pre3"] = P_pre[2][2]
	#	df.at[df.index[step], "P_cor3"] = P_cor[2][2]
	df.at[df.index[step], "P_cor1"] = P_cor[0][0]
	df.at[df.index[step], "P_cor2"] = P_cor[1][1]
	df.at[df.index[step], "x"] = x_pre[0]
	df.at[df.index[step], "v"] = x_pre[1]

fig, axes = plt.subplots(nrows=1, ncols=2)
df.set_index("ZEIT")[["x", "v", "a", "HEIGHT RAW [m]"]].plot(ax= axes[0])
df.set_index("ZEIT")[["P_pre1", "P_pre2", "P_pre2", "P_pre3", "P_cor1", "P_cor2", "P_cor3"]].plot(ax= axes[1])
plt.show()
