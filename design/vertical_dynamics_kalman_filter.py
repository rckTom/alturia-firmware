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


A = sp.Matrix([[0, 1, 0],
               [0, 0, 1],
               [0, 0, 0]])

C = sp.Matrix([[1, 0, 0],
               [0, 0, 1]])

G = sp.Matrix([0, 0, 1])

Q = sp.Symbol('variance_acc')

R = sp.Matrix([[sp.Symbol('variance_meas_h'), 0],
               [0, sp.Symbol('variance_meas_a')]])

def main(args):
    dt = sp.Symbol('dt')
    A_d = (A*dt).exp()
    G_d = A_d*G

    (x_cor, P_cor) = ctrl.kalman_correct(C, None, R)
    (x_pre, P_pre) = ctrl.kalman_predict(A_d, None, G_d, Q)

    ctrl.kalman_sys_export(x_pre, P_pre, x_cor, P_cor, args.outfile)

if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--outfile", dest="outfile", required=True)
    args = parser.parse_args()
    main(args)
