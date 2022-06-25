Two binaries are provided:
 - MatrixVectorMultiplication.exe, a Windows executable
 - MatrixVectorMultiplication (no extension), theorically a Linux executable. Note that this has been compiled using WSL, so it may not work on a proper Linux system.

Both software have the same base code, which is provided alongside the CMakeLists used for the compilation.
The program takes a single argument, which is the matrix/vector size. They are both initially filled with numbers from 0 to size / size^2, respectively for the vector and matrix.
In the end, the resulting vector will be displayed.

Additionally, there are two macros which defined the type of the matrix/vector. They can be freely change to match your expectation, the default type being `unsigned int`.
Finally, make sure to use an adequate number of processes. The program will only recognize 1, 4, 16, 64 and 144 processes.