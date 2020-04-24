import control
import os
import sympy_helpers as sph
import sympy as sp
from sympy.utilities import codegen
from sympy.printing.ccode import C99CodePrinter, float32, real, ccode
import codegen
import pickle
from argparse import ArgumentParser
import dill


def skewrep(vec):
    return sp.Matrix([[0, 		 vec[2], -vec[1], vec[0]],
                      [-vec[2], 	  0,  vec[0], vec[1]],
                      [ vec[1], -vec[0],       0, vec[2]],
                      [-vec[0], -vec[1], -vec[2],      0]])

def main(args):
    # Attitude is represented as a quaternion
    omega_WM_M = sp.MatrixSymbol('omega_WM_M', 3, 1)
    q = sp.MatrixSymbol('q',4,1)

    # Solution to the quaternion equation
    dt = sp.Symbol('dt')
    S = skewrep(omega_WM_M) * dt
    theta = sp.sqrt(S[0,3]**2 + S[1,3]**2 + S[2,3]**2)
    q_next = q + 0.5 * (2 * (sp.cos(theta/2)-1) * sp.eye(S.shape[0]) + 2 / theta * sp.sin(theta/2) * S) * q

    q_next, formats = sph.subsMatrixSymbols(q_next)
    q_next = q_next.doit()
    q_next = sph.subsMatrixElements(q_next, formats)

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
