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
