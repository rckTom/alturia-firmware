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

def predict():
    dt = sp.Symbol('dt')
    A_d = (A*dt).exp()
    G_d = A_d*G

    return ctrl.kalman_predict(A_d, None, G_d, Q)

def correct():
    return ctrl.kalman_correct(C, None, R)

def main(args):
    (x_pre, P_pre) = predict()
    (x_cor, P_cor) = correct()

    ctrl.kalman_sys_export(x_pre, P_pre, x_cor, P_cor, args.outfile)



if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--outfile", dest="outfile", required=True)
    args = parser.parse_args()
    main(args)
