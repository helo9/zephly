import pyahrs, numpy as np

q = np.array([1,2,2,3])
w = np.array([1,2,3])

pyahrs.propagate_attitude(q, w, 1.0)
