import csv
import os
def parse_csv(fname):
    d = []
    with open(fname, 'r') as csvfile:
        reader = csv.reader(csvfile, delimiter=",", quotechar='"')
        for r in reader:
            d.append(r)
    return d

def eval(key, dir):
    sumTotal = 0
    sumBad   = 0
    sumCrash = 0

    for f in os.listdir(dir):
        content = parse_csv(dir+f)
        total = content[-1][0]
        bad = content[-1][1]
        crash = content[-1][2]

        sumTotal += int(total)
        sumBad   += int(bad)
        sumCrash += int(crash)
    print("key=", key)
    print("sumAttempt=", sumTotal)
    def fmt(x):
        return f"({x:.1%})"
    print("sumBad=", sumBad, fmt(sumBad/sumTotal))
    print("sumCrash=", sumCrash, fmt(sumCrash/sumTotal))

print("=====================")
eval("{0,1}", "1/")
print("=====================")
eval("{-1,1}", "2/")
print("=====================")
