import os
import sympy_helpers as sph
import sympy as sp
from sympy.utilities import codegen
from sympy.printing.c import C99CodePrinter, float32, real, ccode
import codegen
from argparse import ArgumentParser
from sympy import Quaternion
from attitude_estimation import L, G

def q_to_rot(q):
    return G(q)*L(q).T

def cross(a, b):
    ax = sp.Matrix([[0, -a[2], a[1]],[a[2], 0, -a[0]], [-a[1], a[0], 0]])
    return ax*b

def outputs():
    dt = sp.Symbol("dt", real=True, positive=True)
    ki = sp.Symbol("ki", real=True)
    kp = sp.Symbol("kp", real=True)
    b = sp.MatrixSymbol("gyro_bias", 3, 1)
    a = sp.MatrixSymbol("a_m", 3, 1)
    g = sp.MatrixSymbol("g_m", 3, 1)
    q_in = sp.MatrixSymbol("q", 4, 1)
    q = sp.Quaternion(q_in[0], q_in[1], q_in[2], q_in[3])
    a_norm = sp.sqrt(a[0]**2 + a[1]**2 + a[2]**2)
    R = q_to_rot(q)

    v_a = R.T * sp.Matrix([[0, 0, 1]]).T
    print("v_a:")
    sp.pprint(v_a.doit())

    a_normalized = a/a_norm
    omega_mes = cross(a_normalized, v_a)
    b = b + (-ki * omega_mes) * dt
    g_biased = g - b + kp * omega_mes
    p = sp.Quaternion(0, g_biased[0], g_biased[1], g_biased[2])
    qDot = 1/2 * q.mul(p)
    q = q + qDot * dt
    q = q.normalize()
    q = sp.Matrix([q.a, q.b, q.c, q.d])

    return (q, b)

def main(args):
    q_next_sym = sp.MatrixSymbol("q_next", 4, 1)
    q, b = outputs()

    os.makedirs(args.out_dir, exist_ok=True)
    out_dict = {"q_next" : [sp.Eq(q_next_sym, q), sp.Eq(sp.MatrixSymbol("g_bias_new", 3, 1),b)]}
    code = codegen.gen_code(out_dict, args.prefix, "mahony")

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
