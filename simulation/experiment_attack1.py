# Experiment for Attack 1
import matplotlib.pyplot as plt
import random
import sys
import csv
from copy import deepcopy
from attack1 import prob_valid


def usage():
    print("USAGE: python3 experiment_attack1.py NTRIAL")
    print("e.g.: python3 experiment_attack1.py 1000")
    sys.exit(-1)

if __name__ == "__main__":
    m = [2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
         4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5,
         5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8,
         9, 9, 9, 10, 10, 10, 10, 9, 8, 8, 8, 7, 7, 7, 7, 7, 6, 5,
         1, 2, 2]

    ls = [359, 353, 349, 347, 337, 331, 317, 313, 311,
          307, 293, 283, 281, 277, 271, 269, 263, 257, 251, 241, 239, 233, 229,
          227, 223, 211, 199, 197, 193, 191, 181, 179, 173, 167, 163, 157, 151,
          149, 139, 137, 131, 127, 113, 109, 107, 103, 101, 97, 89, 83, 79, 73,
          71, 67, 61, 59, 53, 47, 43, 41, 37, 31, 29, 23, 19, 17, 13, 11, 7, 5,
          3, 587, 373, 367]
    n = len(m)

    # SIMBA-3-8
    nsplit = 3
    nfirst = 8

    # to have reproducibe experiments and always use the same keys
    random.seed(7)

    # number of secret keys
    nsample = 100
    if len(sys.argv) != 2:
        usage()
    # number of trials for calculating a probability
    ntrial = int(sys.argv[1])
    mode = "1a"

    sum = 0
    x = []
    el= []
    y = []
    for _ in range(nsample):
        print('.')
        e = [random.randint(-m[i],m[i]) for i in range(n)]
        ed = [m[i] - abs(e[i]) for i in range(n)]
        for i in range(len(e)):
            sum = sum + abs(e[i]) * ls[i]
        x.append(sum)
        el.append(deepcopy(e))
        y.append(prob_valid(e, m, nsplit, nfirst, ntrial, mode))
        sum = 0


    # save raw data as csv
    with open(f"data_{mode}_{nsample}_{ntrial}.csv", mode='w') as csv_file:
        csv_writer = csv.writer(csv_file)
        for xx, yy in zip(x,y):
            csv_writer.writerow([xx,yy])

    with open(f"data_{mode}_{nsample}_{ntrial}_ei.csv", mode='w') as csv_file:
        csv_writer = csv.writer(csv_file)
        for xx, yy in zip(el,y):
            csv_writer.writerow(xx +[yy])

