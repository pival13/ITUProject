## Compilation instruction

```
cmake -B [buildFolder] [sourceDolder]
cmake --build [buildFolder]
```

`sourceFolder` is the folder containing the CMakeLists.txt file.
`buildFolder` is the folder where temporary compilation file will be created.

The final executable, named `a`, will should also be copied to `sourceFolder` at the end of the compilation.

## Executable

The executable `a` does not require any argument. It can be launch with any power of 2 number of processes. The behaviour on other cases in undetermined.

The number of data treated is currently 100x100x100. It can be changed by modfying the macro for X_SIZE, Y_SIZE and Z_SIZE.

The program will currently output, in order, the time spent while itering, the whole executable time until the display, the number of iteration and all the results of the equation.<br>
The second time include the parallelisation initialization and output gathering, which is not included on the first time.<br>
Each line of the result is composed of: the x,y,z value, the value calculated and the value of the B vector.