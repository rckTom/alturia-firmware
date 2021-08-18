import os
import pickle
from argparse import ArgumentParser
import sympy as sp
import sympy_helpers as sph
from sympy.utilities import codegen
from sympy.printing.c import C99CodePrinter, float32, real, ccode
from sympy.matrices.expressions.matexpr import MatrixElement

def subs_single_elements(routine, element_type):
	single_elements = list()

	localvar = routine.local_vars.copy()
	resvar = routine.result_variables

	for res in routine.local_vars:
		if type(res.expr) == element_type:
			single_elements.append(res)
			localvar.remove(res)

	for e in single_elements:
		for i, res in enumerate(localvar):
			localvar[i].expr = localvar[i].expr.subs(e.result_var, e.expr)
		for i, res in enumerate(resvar):
			resvar[i].expr = resvar[i].expr.subs(e.result_var, e.expr)

	routine.local_vars = localvar

	return routine

def subs_single_mat_symbols(routine):
	return subs_single_elements(routine, sp.MatrixSymbol)


def subs_single_mat_elements(routine):
	return subs_single_elements(routine, MatrixElement)

def doit(expr):
    if isinstance(expr, sp.Equality):
        rhs = expr.rhs
    else:
        rhs = expr
    rhs, formats = sph.subsMatrixSymbols(rhs)
    rhs = rhs.doit()
    rhs =  sph.subsMatrixElements(rhs, formats)

    if isinstance(expr, sp.Equality):
        return sp.Eq(expr.lhs, rhs)
    return rhs

def gen_code(obj, file_prefix, prefix):
    for k, v in obj.items():
        if not isinstance(v, list):
            obj[k] = doit(v)
            break

        for i, e in enumerate(v):
            v[i] = doit(e)

    codegen.default_datatypes['float'].cname = 'float'
    gen = codegen.C99CodeGen(project = "alturia", cse = True)
    gen.printer.type_aliases[real] = float32

    routines = dict()
    for k, v in obj.items():
        routines[k] = gen.routine("{}_{}".format(prefix, k), v)
        routines[k] = subs_single_mat_symbols(routines[k])
        routines[k] = subs_single_mat_elements(routines[k])

    return gen.write(routines.values(), prefix=file_prefix)

def main(args):
    with open(args.input, "rb") as f:
        obj = pickle.load(f)

    in_name = os.path.basename(args.input)
    in_name = os.path.splitext(in_name)[0]

    code = gen_code(obj, file_prefix = in_name, prefix=args.prefix)

    os.makedirs(args.out_dir, exist_ok = True)

    for element in code:
        out_name = os.path.join(args.out_dir, element[0])
        with open(out_name, "w") as f:
            f.write(element[1])

if __name__ == "__main__":
    parser = ArgumentParser(description="Generate code for sympy objects")
    parser.add_argument("--input", dest='input', required=True, type=str,
                        help="input file")
    parser.add_argument("--prefix", dest='prefix', required=True, type=str,
                        help="prefix for the generated functions")
    parser.add_argument("--outdir", dest="out_dir", type=str,
                        help="directory where source files are placed",
                        default="./generated")

    args = parser.parse_args()
    main(args)
