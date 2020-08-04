# Introduction 

This repository contains auxiliary material for the paper "Trouble at the CSIDH: Protecting CSIDH with
Dummy-Operations against Fault Injection Attacks" which was accepted to [FDTC 2020](https://fdtc.deib.polimi.it/FDTC20/index.html).

Authors
 - [Fabio Campos](https://www.sopmac.de/) `<campos@sopmac.de>` 
 - [Matthias J. Kannwischer](https://kannwischer.eu) `<matthias@kannwischer.eu>`
 - Michael Meyer `<michael.meyer@hs-rm.de>`
 - Hiroshi Onuki `<onuki@mist.i.u-tokyo.ac.jp>`
 - Marc Stöttinger `<marc.stoettinger@continental-corporation.com>` 

The paper is available at <TODO>

```
git clone --recurse-submodules --depth 1 https://github.com/csidhfi/cisdhfi
```

#Overview

This archive contains the following 
- `simulation` contains the simulation scripts used in Section 4 of the paper.
  If you have Python3 installed, then these should allow you to reproduce the 
  results in the paper. 
  For more details see [simulation/README.md](./simulation/README.md) 

- `attack` contains the attack scripts used in Section 5 of the paper.
  If you have a ChipWhisperer-Lite (CW1173) 32-bit basic board which features 
  a STM32F303 (and a little bit of time), you can reproduce the results 
  presented in the paper by simply running our attack script.  
  For more details see [attack/README.md](./attack/README.md) 
  
- `benchmark` contains our implementation with and without countermeasures
  together with a benchmarking framework which allows to reproduce the results
  of Section 7 of the paper.
  In addition to the STM32F303, it also supports the popular STM32F407.  
  For more details see [benchmark/README.md](./benchmark/README.md)


# Licenses

Code in this repository that does not indicate otherwise is placed in the public domain. 

For the third party code see their licenses:
- [libopencm3](https://github.com/libopencm3/libopencm3): [https://github.com/libopencm3/libopencm3/blob/master/COPYING.LGPL3](https://github.com/newaetech/chipwhisperer/blob/develop/LICENSE.txt)
- [chipwhisperer](https://github.com/newaetech/chipwhisperer): [https://github.com/newaetech/chipwhisperer/blob/develop/LICENSE.txt](https://github.com/newaetech/chipwhisperer/blob/develop/LICENSE.txt)
