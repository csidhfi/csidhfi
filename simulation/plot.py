import matplotlib.pyplot as plt
import csv
import numpy as np
from matplotlib.ticker import FuncFormatter

def plot_it(fIn, fOut):
  with open(fIn, mode='r') as csv_file:
    csv_reader = csv.reader(csv_file)

    x = []
    y = []
    ls = [359, 353, 349, 347, 337, 331, 317, 313, 311,
          307, 293, 283, 281, 277, 271, 269, 263, 257, 251, 241, 239, 233, 229,
          227, 223, 211, 199, 197, 193, 191, 181, 179, 173, 167, 163, 157, 151,
          149, 139, 137, 131, 127, 113, 109, 107, 103, 101, 97, 89, 83, 79, 73,
          71, 67, 61, 59, 53, 47, 43, 41, 37, 31, 29, 23, 19, 17, 13, 11, 7, 5, 3,
          587, 373, 367]


    for entry in csv_reader:
        wsum = 0
        #for e, l in zip(entry[0:-2], ls):
        #    wsum += abs(int(e))*l

        for i in range(len(ls)):
            wsum = wsum + abs(int(entry[i]))*ls[i]

        x.append(wsum)
        y.append(float(entry[-1]))

  def thousands(x, pos):
    return '%1.0fk' % (x * 1e-3)


  formatter = FuncFormatter(thousands)


  z = np.polyfit(x, y, 1)
  p = np.poly1d(z)

  scale = 0.009
  plt.figure(figsize=(scale*460.8,scale*345.6))

  plt.gca().xaxis.set_major_formatter(formatter)
  plt.plot(x, y, "+",color='grey')
  plt.plot(x,p(x),"black")
  plt.xlabel('weighted sum of $e_i$')
  plt.ylabel('% of ineffective faults')
  plt.xlim(22000, 35000)
  plt.ylim(0.15, 0.33)
  plt.tight_layout()
  plt.savefig(fOut)
  plt.clf()

plot_it("data_1a_100_500000_ei.csv", "plot_1a_100_500000.pdf")
