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
from argparse import ArgumentParser


var_process_accel = sp.Symbol('var_process_accel')
var_meas_altitude = sp.Symbol('var_meas_altitude')

A = sp.Matrix([[0, 1],
               [0, 0]])

C = sp.Matrix([1, 0]).T

G = sp.Matrix([0, 1])
Q = var_process_accel
R = sp.Matrix([var_meas_altitude])

def main(args):
    (x_pre, P_pre) = ctrl.kalman_predict(A, None, G, Q)
    (x_cor, P_cor) = ctrl.kalman_correct(C, None, R)

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

    with open(args.outfile, "wb") as f:
        pickle.dump(eqs, f)



if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--outfile", dest="outfile", required=True)
    args = parser.parse_args()
    main(args)
