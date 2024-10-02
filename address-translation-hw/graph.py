import random
import matplotlib.pyplot as plt
import numpy as np

asize = 1024
psize = 16 * 1024
u_list = [0] * psize

for i in range(300):
    random.seed(i)
    for j in range(psize):
        limit = j
        va = int(asize * random.random())
        if va < limit:
            u_list[j] += 1

fig = plt.figure()
x = np.linspace(1, psize, psize)
plt.plot(x, [u / 300 for u in u_list], color="blue")
plt.ylim(0, 1)
plt.margins(0)
plt.xlabel("Limit")
plt.ylabel("Valid Fraction (Average)")
plt.savefig("valid-fraction.png", dpi=227)
plt.show()
