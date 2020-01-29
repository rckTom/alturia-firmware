import control as ctrl
import sympy as sp
import sympy_helpers as sph
import pickle
from gen_code_linear_kalman_filter import kalman_sys_export

from argparse import ArgumentParser


var_sys_v = sp.Symbol('var_sys_v')
var_meas_h = sp.Symbol('var_meas_h')

A = sp.Matrix([0])
B = None
C = sp.Matrix([1])
G = sp.Matrix([1])
Q = sp.Matrix([var_sys_v])
R = sp.Matrix([var_meas_h])

def main(args):
    dt = sp.Symbol('dt')
    A_d = (A*dt).exp()
    G_d = A_d*G

    x_pre, P_pre = ctrl.kalman_predict(A_d, None, G_d, Q)
    x_cor, P_cor = ctrl.kalman_correct(C, None, R)

    kalman_sys_export(x_pre, P_pre, x_cor, P_cor, args.outfile)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--outfile", dest="outfile", required=True,
                        help="Destination file to write result to")

    args = parser.parse_args()
    main(args)