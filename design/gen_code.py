'''
Alturia Firmware - The firmware for the alturia flight computer

Copyright (c) Thomas Schmid, 2019

Authors:
 Thomas Schmid <tom@lfence.de>

This work is licensed under the terms of the GNU GPL, version 3.  See
the COPYING file in the top-level directory.
'''

import sympy as sp
from sympy.utilities import codegen
from sympy.printing.ccode import C99CodePrinter, float32, real, ccode
import pickle
import os
sp.init_printing()

def main():
	with open("vertical_dynamics_kalman_filter.pkl", "rb") as f:
		obj = pickle.load(f)

	codegen.default_datatypes['float'].cname = 'float'
	gen = codegen.C99CodeGen(project = "alturia", cse = False)
	printer = gen.printer
	printer.type_aliases[real] = float32
	routines = dict()

	x_cor = sp.MatrixSymbol("x_cor", *obj["x_cor"].shape)
	P_cor = sp.MatrixSymbol("P_cor", *obj["P_cor"].shape)
	x_pre = sp.MatrixSymbol("x_pre", *obj["x_pre"].shape)
	P_pre = sp.MatrixSymbol("P_pre", *obj["P_pre"].shape)

	correct = [sp.Eq(x_cor, obj["x_cor"]), sp.Eq(P_cor, obj["P_cor"])]
	predict = [sp.Eq(x_pre, obj["x_pre"]), sp.Eq(P_pre, obj["P_pre"])]

	routines["correct"] = gen.routine("kalman_vertical_dyn_correct", correct)
	routines["predict"] = gen.routine("kalman_vertical_dyn_predict", predict)

	os.makedirs("../src/generated", exist_ok = True)
	with open("../src/generated/kalman_vertical_dynamics.c", "w") as f:
		gen.dump_code([routines['correct'], routines['predict']], f, prefix="kalman_vertical_dynamics")

	with open("../src/generated/kalman_vertical_dynamics.h", "w") as f:
		gen.dump_h([routines['correct'], routines['predict']], f, prefix="kalman_vertical_dynamics")

if __name__ == "__main__":
	main()
