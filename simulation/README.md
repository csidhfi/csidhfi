# Overview 

We provide the scripts that have been used to produce the simulation results for attack 1 presented in the paper.

## Installation

Our simulation scripts require [Python3](https://www.python.org/about/gettingstarted/) to be installed. 
If you want to create plots, you will also need [matplotlib](https://matplotlib.org/users/installing.html).


## Running Simulations
- To run the simulations for attack 1, you execute
   - `python3 experiment_attack1.py NTRIAL` where NTRIAL is the number of fault injections that you want to simulate 
   - For example to reproduce the plot in the paper you can run `python3 experiment_attack1.py 500000` which will run for a couple of days.  
   - By default it will sample 100 random CSIDH512 secret keys. In case you want to change this number, you can change `nsample` in [experiment_attack1.py](experiment_attack1.py)
   
  The script will simulate injecting `NTRIAL` faults at uniformely random location during the CSIDH512 group action.
  It will then output if the fault was effective (corrupt shared secret) or ineffective (no change in the shared secret). 
  At the end of the simulation it will write a `.csv` containing the probability for an ineffective fault for each secret key that was sampled. 
   
  We have provided the results for the run with `NTRIAL=500000` in [data_1a_100_500000.csv](data_1a_100_500000.csv) (containing only the weighted sum) and [data_1a_100_500000_ei.csv](data_1a_100_500000_ei.csv) (containing the entire secret keys.)
  
## Ploting Results
- To create the plot in the paper, run `python3 plot.py`. The plot is also available at [plot_1a_100_500000.pdf](plot_1a_100_500000.pdf).
