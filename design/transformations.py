import sympy as sp
import sympy_helpers as sph
from argparse import ArgumentParser
import os
import codegen

R_W_IMU = sp.rot_axis1(sp.rad(-90)) * sp.rot_axis3(sp.rad(-90))
R_W_ACC = sp.rot_axis1(sp.rad(90)) * sp.rot_axis3(sp.rad(-90))

v_IMU = sp.MatrixSymbol("v_IMU", 3, 1)
v_ACC = sp.MatrixSymbol("v_ACC", 3, 1)
v_W = sp.MatrixSymbol("v_W", 3, 1)

def main(args):
    exprs = {"IMU_to_W": [sp.Eq(v_W, R_W_IMU * v_IMU)],
             "ACC_to_W": [sp.Eq(v_W, R_W_ACC * v_ACC)],
             "W_to_IMU": [sp.Eq(v_IMU, R_W_IMU.T * v_W)],
             "W_to_ACC": [sp.Eq(v_ACC, R_W_ACC.T * v_W)]}

    out = codegen.gen_code(exprs, args.prefix, "trans")

    os.makedirs(args.out_dir, exist_ok=True)

    for fid in out:
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
