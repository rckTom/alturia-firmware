import sympy as sp

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
