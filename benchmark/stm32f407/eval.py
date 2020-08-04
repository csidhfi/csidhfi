#!/bin/python3
import matplotlib.pyplot as plt
import numpy as np

x = []
y = []

with open("speed_nocountermeasures.log", "r") as f:
    lines = f.readlines()
    for i in range(len(lines)):
        line = lines[i]
        line = line.replace("\n","")
        if "xISOG" in line:
            degree = int(line.split("=")[1].split(")")[0])
            cost   = int(lines[i+1])

            x.append(degree)
            y.append(cost)



def parseFile(fName):
    with open(fName, "r") as f:
        lines = f.readlines()
        b = lines[1+lines.index("public bob cycles:\n")].strip()
        a = lines[1+lines.index("public alice cycles:\n")].strip()
        ab = lines[1+lines.index("shared alice cycles:\n")].strip()
        ba = lines[1+lines.index("shared bob cycles:\n")].strip()
    return [int(b), int(a), int(ab), int(ba)]


def avgFile(fList):
    l = []
    for f in fList:
        l +=parseFile(f)
    return int(np.mean(l))



nocm_mean = avgFile(["speed_nocountermeasures.log", "speed_nocountermeasures.log.1", "speed_nocountermeasures.log.2",])
print("nocm_mean", nocm_mean)


m = [2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5,
	5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8,
	9, 9, 9, 10, 10, 10, 10, 9, 8, 8, 8, 7, 7, 7, 7, 7, 6, 5,
	1, 2, 2]

totalIsogCost = 0
for (cost, max) in zip(y, m):
    totalIsogCost += cost * max

print("totalIsogCost", totalIsogCost)
print(totalIsogCost/nocm_mean)

# plot
plt.plot(x, y, "+",color='grey')
plt.ylabel('xISOG cost')
plt.xlabel('isog degree')
plt.show()







notwistcm_mean = avgFile(["speed_countermeasures_notwistcm.log", "speed_countermeasures_notwistcm.log.1", "speed_countermeasures_notwistcm.log.2",])
print("notwistcm_mean", notwistcm_mean)
print("overhead_notwist", (notwistcm_mean-nocm_mean))
print("overhead_notwist", (notwistcm_mean-nocm_mean)/nocm_mean)

twistcm_mean = avgFile(["speed_countermeasures_twistcm.log", "speed_countermeasures_twistcm.log.1", "speed_countermeasures_twistcm.log.2",])
print("twistcm_mean", twistcm_mean)
print("overhead_twist", (twistcm_mean-nocm_mean))
print("overhead_twist", (twistcm_mean-nocm_mean)/nocm_mean)
