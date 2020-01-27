import sympy as sp

def get_atom(expr, name, atom_type):
    s = expr.atoms(atom_type)

    for symbol in s:
        if symbol.name == name:
            return symbol

    return None

def gen_matrix(pattern, i, j):
    return sp.Matrix(i, j, lambda i, j: sp.Symbol(pattern.format(i,j)))

def subs_mat_symbol_to_matrix(expr, symbol, pattern):
    if isinstance(symbol, str):
        symbol = get_atom(expr, symbol, sp.MatrixSymbol)

    M = gen_matrix(pattern, *symbol.shape)
    expr = expr.subs({symbol: M})
    return (expr, M)

def subs_matrix_to_mat_symbol(expr, symbol, pattern):
    for i in range(0, symbol.shape[0]):
        for j in range(0, symbol.shape[1]):
            expr = expr.subs({pattern.format(i,j): symbol[i,j]})

    return expr

def subsMatrixSymbols(expr):
    symbols = expr.atoms(sp.MatrixSymbol)
    formats = dict()

    for symbol in symbols:
        f = symbol.name + "{},{}"
        formats[f] = symbol
        expr, M = subs_mat_symbol_to_matrix(expr, symbol, f)

    return expr, formats

def subsMatrixElements(expr, symbols):
    for f, symbol in symbols.items():
        expr = subs_matrix_to_mat_symbol(expr, symbol, f)
    return expr

