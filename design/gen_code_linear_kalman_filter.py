#!/usr/bin/env python
'''
Alturia Firmware - The firmware for the alturia flight computer

Copyright (c) Thomas Schmid, 2019

Authors:
 Thomas Schmid <tom@lfence.de>

This work is licensed under the terms of the GNU GPL, version 3.  See
the COPYING file in the top-level directory.
'''

from sympy.utilities import codegen
from sympy.printing.ccode import C99CodePrinter, float32, real, ccode
import pickle
import os
from argparse import ArgumentParser
import sympy as sp
import sympy_helpers as sph

def kalman_sys_export(x_pre, P_pre, x_cor, P_cor, path):
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

    with open(path, "wb") as f:
        pickle.dump(eqs, f)

def main(args):
    with open(args.input, "rb") as f:
        obj = pickle.load(f)

    in_name = os.path.basename(args.input)
    in_name = os.path.splitext(in_name)[0]

    codegen.default_datatypes['float'].cname = 'float'
    gen = codegen.C99CodeGen(project = "alturia", cse = False)
    gen.printer.type_aliases[real] = float32

    routines = dict()

    routines["correct"] = gen.routine("{}_correct"
                                .format(args.prefix), obj['correct'])
    routines["predict"] = gen.routine("{}_predict"
                                .format(args.prefix), obj['predict'])

    os.makedirs(args.out_dir, exist_ok = True)

    out_name = os.path.join(args.out_dir, in_name)
    with open(out_name + ".c", "w") as f:
        gen.dump_code([routines['correct'], routines['predict']], f,
                      prefix=in_name)

    with open(out_name + ".h", "w") as f:
        gen.dump_h([routines['correct'], routines['predict']], f,
                   prefix=in_name)

if __name__ == "__main__":
    parser = ArgumentParser(description="Generate code for linear "
            "kalman filters. Expects a pickeld object which contains a dict of "
            "sympy equations which descripe the correct and predict step")
    parser.add_argument("--input", dest='input', required=True, type=str,
                        help="input file")
    parser.add_argument("--prefix", dest='prefix', required=True, type=str,
                        help="prefix for the generated functions")
    parser.add_argument("--outdir", dest="out_dir", type=str,
                        help="directory where source files are placed",
                        default="./generated")

    args = parser.parse_args()
    main(args)
