# Practial Evaluation of our Attacks

We provide the ChipWhisperer attack scripts that we used. 
If you have a [CW1173](https://wiki.newae.com/CW1173_ChipWhisperer-Lite) at hand you should be able to reproduce the results reported in the paper. 
Note that we observed that the results are not very stable accross different boards, so you might get a different success rate of injecting faults. Nonetheless, the correlation with the secret key should be obvious. 

## Installation
- Follow the instructions on https://wiki.newae.com/Installing_ChipWhisperer
- If you are lucky, `pip install chipwhisperer` and installing `arm-none-eabi-gcc` is sufficient. 


## Attack 1 
- Select a secret key in [speed.c](speed.c). The default is `sk=[0, 0]` 
- Run `python3 CSIDH_glitch_attack1.py NTRIAL` where `NTRIAL` is the number of experiments you want to run.
- After completing the experiment, the script will write a `.csv` file containing the results of the experiments 
  - `bad`: the shared secret was corrupted
  - `good`: the shared secret did not change, which could either indicate that your glitching did not work, or your glitched a dummy-operation. 
  - `crash`: no shraed secret was returned 
  
 - We've provided the raw experiment results in [attack1_results/](attack1_results/). 
   - [attack1_results/0/](attack1_results/0/) contains results for `sk=[0, 0]`
   - [attack1_results/1/](attack1_results/1/) contains results for `sk=[0, 1]`
   - [attack1_results/2/](attack1_results/2/) contains results for `sk=[-1, 1]`
   - `cd attack1_results && python3 eval.py` prints the aggregated results over all `.csv` files. 

The results are

| key   | # of trials | faulty shared secret | 
| ----- | ----------- | -------------------- | 
| {0,0} | 5000        | 19.8%                |
| {0,1} | 5000        | 27.3%                |
| {-1,1}| 5000        | 32.8%                |

## Attack 2
- Select a secret key in [speed.c](speed.c). The default is `sk=[0, 1]` 
- Run `python3 CSIDH_glitch_attack2.py NTRIAL` where `NTRIAL` is the number of experiments you want to run.
- After completing the experiment, the script will write a `.csv` file containing the results of the experiments 
  - `bad`: the shared secret was corrupted
  - `good`: the shared secret did not change, which could either indicate that your glitching did not work, or your glitched a dummy-operation. 
  - `crash`: no shraed secret was returned 
  
 - We've provided the raw experiment results in [attack2_results/](attack2_results/). 
   - [attack2_results/1/](attack2_results/1/) contains results for `sk=[0, 1]`
   - [attack2_results/2/](attack2_results/2/) contains results for `sk=[-1, 1]`
   - `cd attack2_results && python3 eval.py` prints the aggregated results over all `.csv` files. 
   
The results are 

| key   | # of trials | faulty shared secret | 
| ----- | ----------- | -------------------- | 
| {0,1} | 5000        | 2.1%                 |
| {-1,1}| 5000        | 16.4%                |
