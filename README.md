# Dynamic-Instruction-Scheduling-Simulator
Developed a dynamic instructing scheduling simulator modelling a 9-stage out-of-order super-scalar microarchitecture.  Analyzed the effect of increasing capacity of each pipeline stage on the IPC performance of the machine.

1. Type "make" to build.  (Type "make clean" first if you already compiled and want to recompile from scratch.)

2. Run trace reader:
    
	Your simulator must accept command-line arguments as follows:
	./sim <ROB_SIZE> <IQ_SIZE> <WIDTH> <tracefile>
	The parameters <ROB_SIZE>, <IQ_SIZE>, and <WIDTH>, are explained in Section 5. <tracefile> is the filename of the input trace.

   To run without throttling output:
   ./sim 256 32 4 gcc_trace.txt

   To run with throttling (via "less"):
   ./sim 256 32 4 gcc_trace.txt | less