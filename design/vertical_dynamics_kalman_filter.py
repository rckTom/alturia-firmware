'''
Alturia Firmware - The firmware for the alturia flight computer

Copyright (c) Thomas Schmid, 2019

Authors:
 Thomas Schmid <tom@lfence.de>

This work is licensed under the terms of the GNU GPL, version 3.  See
the COPYING file in the top-level directory.
'''

import control as ctrl
import sympy as sp
import sympy_helpers as sph
import pickle


A = sp.Matrix([[0, 1, 0],
               [0, 0, 1],
               [0, 0, 0]])

C = sp.Matrix([[1, 0, 0],
               [0, 0, 1]])

G = sp.Matrix([0, 0, 1])

Q = sp.Symbol('variance_acc')

R = sp.Matrix([[sp.Symbol('variance_meas_h'), 0],
               [0, sp.Symbol('variance_meas_a')]])

def subsMatrixSymbols(expr):
    symbols = expr.atoms(sp.MatrixSymbol)
    formats = dict()

    for symbol in symbols:
        f = symbol.name + "{},{}"
        formats[f] = symbol
        expr, M = sph.subs_mat_symbol_to_matrix(expr, symbol, f)

    return expr, formats

def subsMatrixElements(expr, symbols):
    for f, symbol in symbols.items():
        expr = sph.subs_matrix_to_mat_symbol(expr, symbol, f)
    return expr

def main():
    dt = sp.Symbol('dt')
    A_d = (A*dt).exp()
    G_d = A_d*G

    (x_cor, P_cor) = ctrl.kalman_correct(C, None, R)
    (x_pre, P_pre) = ctrl.kalman_predict(A_d, None, G_d, Q)

    exprs = {"x_cor": x_cor, "P_cor" : P_cor, "x_pre": x_pre, "P_pre": P_pre}

    for k, expr in exprs.items():
        expr, expr_formats = subsMatrixSymbols(expr)
        expr = expr.doit()
        expr = subsMatrixElements(expr, expr_formats)
        exprs[k] = expr

    with open("vertical_dynamics_kalman_filter.pkl", "wb") as f:
        pickle.dump(exprs, f)

if __name__ == "__main__":
    main()
