#how to run the code
###for cpu.c just run commonly
>1.      gcc -o cpu cpu.c
>
>2.      ./cpu

###for others
>you can run it in VS2013 cuda7.5 without any change, but if not just delete these statament in front of the prgram
>
>1.      #include "cuda_runtime.h"
>
>2.      #include "device_launch_parameters.h"
>
>3.      #include <cuda.h>

###then compile with nvcc normally is okay