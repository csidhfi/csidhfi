#!/usr/bin/env python3
import os
import chipwhisperer as cw
import time

'''
    This script builds the binaries targeting the 32-bit ChipWhisperer-Lite
    which has an STM32F303 core, it then flashes the binaries to the board
    using the chipwhisperer python library and print the output it produces.

    This binaries being built are the CSIDH512 implementations with and without
    FI countermeasures implemented.
'''

# define binaries with and without FI countermeasures
TARGET_NOCM = "speed_nocountermeasures"
BIN_NOCM    = f"{TARGET_NOCM}-CWLITEARM.hex"

TARGET_CM_NOTWISTCM = "speed_countermeasures_notwistcm"
BIN_CM_NOTWISTCM    = f"{TARGET_CM_NOTWISTCM}-CWLITEARM.hex"

TARGET_CM_TWISTCM = "speed_countermeasures_twistcm"
BIN_CM_TWISTCM    = f"{TARGET_CM_TWISTCM}-CWLITEARM.hex"

print("making binaries")
os.system(f"make TARGET={TARGET_NOCM}")
os.system(f"make TARGET={TARGET_CM_NOTWISTCM} TYPE=CM")
os.system(f"make TARGET={TARGET_CM_TWISTCM} TYPE=CM TWIST_ATTACK_CM=TWIST_ATTACK_CM")


# chip whisperer setup
scope = cw.scope()
target = cw.target(scope)
prog = cw.programmers.STM32FProgrammer
scope.default_setup()


def doIt(binary):
  print(f"flashing binary {binary}")
  cw.program_target(scope, prog, binary)

  # wait for all the output from the board
  print(f"output starts ({binary}):")
  while True:
    response = target.read(timeout=0)
    if response != None and response != "":
        print(response,end='', flush=True)
    if "#" in response:
        break
  print(f"{binary} done.")

doIt(BIN_NOCM)
doIt(BIN_CM_NOTWISTCM)
doIt(BIN_CM_TWISTCM)
