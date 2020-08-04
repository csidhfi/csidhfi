#!/bin/python3
import numpy as np
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

notwistcm_mean = avgFile(["speed_countermeasures_notwistcm.log", "speed_countermeasures_notwistcm.log.1", "speed_countermeasures_notwistcm.log.2",])
print("notwistcm_mean", notwistcm_mean)
print("overhead_notwist", (notwistcm_mean-nocm_mean))
print("overhead_notwist", (notwistcm_mean-nocm_mean)/nocm_mean)

twistcm_mean = avgFile(["speed_countermeasures_twistcm.log", "speed_countermeasures_twistcm.log.1", "speed_countermeasures_twistcm.log.2",])
print("twistcm_mean", twistcm_mean)
print("overhead_twist", (twistcm_mean-nocm_mean))
print("overhead_twist", (twistcm_mean-nocm_mean)/nocm_mean)
