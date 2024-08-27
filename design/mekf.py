import sympy as sp
import sympy_helpers as sph

def skew_symmetric(x):
    return sp.Matrix([
        [0, -x[2], x[1]],
        [x[2], 0, -x[0]],
        [-x[1], x[0], 0],
    ])

class Quaternion:
    def __init__(self, real, vector):
        self.q = sp.Matrix(4,1)
        self.q[0] = real
        self.q[1] = vector[0]
        self.q[2] = vector[1]
        self.q[3] = vector[2]

    def as_vector(self):
        return self.q

    def vector_part(self):
        return self.q[1:4]

    def real_part(self):
        return self.q[0]

    def len(self):
        return sp.sqrt(self.q.T * self.q)

    def rot_mat(self):
        s = self.len()
        R = s*(self.vector_part * self.vector_part.T +
            self.real_part**2 * sp.eye(3) +
            self.real_part * 2 * skew_symmetric(self.vector_part) + 
            skew_symmetric(self.vector_part)**2)
        return R

def error_quaternion(p):
    return sp.Matrix([1, p[0]/2, p[1]/2, p[2]/2])

p = sp.MatrixSymbol("p", 3, 1) #representation of error quaternion
omega = sp.MatrixSymbol("omega", 3, 1)
beta_omega = sp.MatrixSymbol("beta_omega", 3, 1)
beta_a = sp.MatrixSymbol("beta_a", 3, 1)

x = sp.zeros(9,1)
x[0] = p[0]
x[1] = p[1]
x[2] = p[2]
x[3] = beta_omega[0]
x[4] = beta_omega[1]
x[5] = beta_omega[2]
x[6] = beta_a[0]
x[7] = beta_a[1]
x[8] = beta_a[2]

print(x)
f = sp.zeros(9,1)
adot = -skew_symmetric(omega) * p - beta_omega

g = sp.zeros(3,1)
f[0] = adot[0]
f[1] = adot[1]
f[2] = adot[2]

sp.pprint(f.jacobian(x))

#print(f.jacobian(x))