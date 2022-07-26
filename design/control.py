'''
Alturia Firmware - The firmware for the alturia flight computer

Copyright (c) Thomas Schmid, 2019

Authors:
 Thomas Schmid <tom@lfence.de>

This work is licensed under the terms of the GNU GPL, version 3.  See
the COPYING file in the top-level directory.
'''

import sympy as sp
import sympy_helpers as sph
import pickle

def kalman_predict(A, B, G, Q):
	x_cor = sp.MatrixSymbol('x_cor', A.shape[0], 1)
	P_cor = sp.MatrixSymbol('P_cor', *A.shape)

	if B is None:
		x_pre = A * x_cor
	else:
		u = sp.MatrixSymbol('u', B.shape[1], 1)
		x_pre = A * x_cor+B*u

	P_pre = A * P_cor * A.T + G * Q * G.T

	return (x_pre, P_pre)

def kalman_correct(C, D, R):
	x_pre = sp.MatrixSymbol('x_pre', C.shape[1], 1)
	P_pre = sp.MatrixSymbol('P_pre', C.shape[1], C.shape[1])
	y = sp.MatrixSymbol('y', C.shape[0], 1)

	K = P_pre * C.T * (C * P_pre * C.T + R)**(-1)
	delta_y = y - C * x_pre

	if D is not None:
		u = sp.MatrixSymbol('u', D.shape[1], 1)
		delta_y = delta_y - D * u

	x_cor = x_pre + K * delta_y
	P_cor = (sp.eye(K.shape[0]) - K * C) * P_pre

	return (x_cor, P_cor)

def kalman_sys_export(x_pre, P_pre, x_cor, P_cor, path):
	exprs = {"x_pre": x_pre,
		"P_pre": P_pre,
		"x_cor": x_cor,
		"P_cor": P_cor}

	for k, expr in exprs.items():
		expr, f = sph.subsMatrixSymbols(expr)
		expr = expr.doit()
		expr = sph.subsMatrixElements(expr, f)
		exprs[k] = expr


	x_pre = sp.MatrixSymbol("x_pre", *x_pre.shape)
	P_pre = sp.MatrixSymbol("P_pre", *P_pre.shape)
	x_cor = sp.MatrixSymbol("x_cor", *x_cor.shape)
	P_cor = sp.MatrixSymbol("P_cor", *P_cor.shape)

	eqs = {"correct": [sp.Eq(x_cor, exprs["x_cor"]),
			sp.Eq(P_cor, exprs["P_cor"])],
		"predict": [sp.Eq(x_pre, exprs["x_pre"]),
			sp.Eq(P_pre, exprs["P_pre"])]}

	with open(path, "wb") as f:
		pickle.dump(eqs, f)


def extended_kalman_predict(f, g, x):
	pass