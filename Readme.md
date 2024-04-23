# Game of Life

This is the MPI and OpenMp implementation of the sequential game of life program in `extra_seq.c` file.

### MPI Implementation
To compile the program just run 
```bash
make
```

The program needs to run in bash terminal. To run the program run:
```bash
mpirun -np <number_of_processes> ./main <width> <height>
```

This will run the program with set number of processes and simulate the game of life. At the end it will output something like this:
```bash
....
iter = 198, Global population count = 368
iter = 199, Global population count = 370
MPI Time taken: 0.047932 seconds
```

### OpenMP
To compile the OpenMP version do the following:
```bash
make open
```

Then to run do the following:
```bash
./open <width> <height>
```

The output will be something like this:
```bash
...
iter = 197, Global population count = 368
iter = 198, Global population count = 370
OpenMP Time taken: 0.024197 seconds
```

