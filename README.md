# D5-Code
This project is part of our ELEC2217, D5 Smart Meter project. The algorithm was written by Joseph Butterworth which I then ported onto the Il Matto AVR development board. Our final test scored 80% which was the highest in the cohort.

The algorithm and surrounding testbench can be found in the folder [evaluator](https://github.com/jamesstocktonj1/D5-Code/tree/main/evaluator). The Il Matto files can be found in [main.c](https://github.com/jamesstocktonj1/D5-Code/blob/main/main.c).

### Makefile Usage

The following will compile all the files relevant for the Il Matto.
```
make all
```

The following will test the programming connections to the Il Matto, flash the fuses and finally flash the program.
```
make test
make fuse
make flash
```

## Testbed
<p align="center"><img src="https://github.com/jamesstocktonj1/D5-Code/blob/refactor/docs/testbed.jpg"></p>

## Final Product
<p align="center"><img src="https://github.com/jamesstocktonj1/D5-Code/blob/refactor/docs/housing.jpg"></p>

