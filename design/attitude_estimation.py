from pyparsing import quoted_string
import control
import os
import sympy_helpers as sph
import sympy as sp
from sympy.utilities import codegen
from sympy.printing.c import C99CodePrinter, float32, real, ccode
import codegen
import pickle
from argparse import ArgumentParser
import dill


def L(q:sp.Quaternion):
    return sp.Matrix([[-q.b, q.a, q.d, -q.c],
                      [-q.c, -q.d, q.a, q.b],
                      [-q.d, q.c, -q.b, q.a]])

def G(q:sp.Quaternion):
    return sp.Matrix([[-q.b, q.a, -q.d, q.c],
                      [-q.c, q.d, q.a, -q.b],
                      [-q.d, -q.c, q.b, q.a]])

def qdot(omega, q):
    q = sp.Rational(1,2) * L(q).T * omega
    return sp.Quaternion(q[0], q[1], q[2], q[3])

def qnext():
    q = sp.MatrixSymbol("q", 4, 1)
    dt = sp.Symbol("dt")
    omega = sp.MatrixSymbol("omega_WM_M", 3, 1)
    bias = sp.MatrixSymbol("gyro_bias", 3, 1)
    q_in = sp.Quaternion(a = q[0], b = q[1], c = q[2], d = q[3])
    q_next = q_in + qdot((omega-bias), q_in) * dt
    q_next = q_next.normalize()
    q_next = sp.Matrix([q_next.a, q_next.b, q_next.c, q_next.d])

    return q_next

def main(args):
    q_next = qnext()
    q_next_sym = sp.MatrixSymbol("q_next", *q_next.shape)


    out_dict = {"q_next" : [sp.Eq(q_next_sym, q_next)]}
    code = codegen.gen_code(out_dict, args.prefix, "attitude")

    os.makedirs(args.out_dir, exist_ok=True)

    for fid in code:
        path = os.path.join(args.out_dir, fid[0])
        with open(path, "w") as f:
            f.write(fid[1])


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--outdir", dest="out_dir", type=str,
                        help="directory where source files are placed",
                        default="./generated")
    parser.add_argument("--prefix", dest="prefix", type=str,
                        help="fileprefix")
    args = parser.parse_args()
    main(args)
