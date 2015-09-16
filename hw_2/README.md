Hardware provides performance counters which allow us to measure micro-architectural events such as
clock ticks, floating-point multiplies, pipeline flushes to do to 4k-modulo aliasing in the load-store queue,
etc. We will use these to measure the behavior of a simple matrix-matrix multiply.
Recall the code for the ijk-ordered matrix multiply. Implement this version of matrix multiply. Instrument
the code using PAPI to measure total instructions, total cycles, total floating point instructions, data L1
misses, and total L2 misses (see note about making multiple measurements). Measure these values for a
matrix multiply as a function of problem size. Reason from your source code what the values should be as
a function of problem size.
Make the measurements with a matrix initialized with finite random numbers which will not result in
inf, nan, or denormal numbers in the result. These drastically slow down the hardware.
Make the measurements with a cold cache. There isn’t an architectural way to flush a cache from
userspace, but if you reflect on how a cache works, you should see that you can write code that will effectively
do so. You only need to flush L1 and L2.
    You cannot do more than 1 matrix matrix multiply for a given allocation or you will not get valid
    numbers. It is fine to take multiple measurements with one execution of your code, but you must reallocate
    the matrixes each time.
