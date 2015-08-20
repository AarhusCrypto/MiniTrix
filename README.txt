************************************************************
                    MiniMac Implementation
                             for
        An Empirical Study and some Improvements of the 
            MiniMac Protocol for Secure Computation

  Author: Rasmus Winther Zakarias (Lauritsen)

  Theory by: Ivan Damgaard, Tomas Toft and Rasmus Zakarias
             (C) Aarhus University 2014

             https://eprint.iacr.org/2014/289

  Acknowledgements: Many thanks goes to Jesper Buus Nielsen for
  providing C++ implementation of TinyOT on which ours is based. Thanks
  to Tomas for several hours of debugging and rubber ducking.

************************************************************


This software package is written to document the empirical result
reported in the paper above. The timings reported in the paper were
created using fresh-horses (the student available servers) at the
Department of Computer Science Aarhus University in 2014 running this
software. The timings were measured when built in release mode.

---------- STRUCTURE ---------- 

This software is structured into several stand-alone Linux
AutoTools projects producing static libries and executables to enable
reuse of code for other (later) projects. From the bottom up, OSAL is
an Operating System Abstraction Layer. It is the only party needed to
be rewritten to support other Operating Systems/Platforms (in addition
to creating a platform specific build system if AutoTools are not
available like on Windows). DS stands for Data Structures and contains
a HashMap and BlockingQueue implementation. MATH contains our Galois
2^8 implementation and Matrix operations on top of that. ENCODING
contains byte to integer encoding as well as Distinguished Encoding
Rule encodings for the most general tags including serialization of
int, long, strings and bytes. UTILS has a command line options
parser. CARENA is a network squid managing incoming and outgoing
connections. CMINIMACS is where the actual protocol variants are
implemented. CAES is one application of CMINIMACS for computing
AES. CINTERP is an circuit interpreter based on CMINIMACS taking any
circuit in a specific format and evaluates it (e.g. every instance of
the program is a peer).

---------- HOW TO USE IT ----------


Build: 

Every project is build on the Linux platform by typing

./build.sh debug 

./build.sh release

depending on the version wanted. To reproduce our performance results 
build the release version and follow the instructions below.

Run:

If successful the process above will create a *sys* folder next to
 this file. In this folder is the well known tree structure from /usr
 with bin, lib and include subfolders. Bin contains all the executables,
 here follows a summary of a few of them:

genpre - generate preprocessing material

lsrep  - print info about preprocessing in a rep-file

mulpar2bitfft - the fastests MiniMac instantiation for 2PC AES 

manymulpar2bitfft -the fastests MiniMac instantiation for any number
                   of parties computing AES

To e.g. run MiniMac obtaining the best times reported do:

[Create fake preprocessing material]
bin/genpre 85 255 2 6800 fft sba

[Run the mulpar2bitfft peer1]
bin/mulpar2bitfft minimacs_85_255_2_0_fft.rep bdt_85_255_0of2_6800_fft.rep 1

ARGS explained:
minimacs_85_255_2_0_fft.rep - preprocessing of singles result of genpre
bdt_85_255_0of2_6800_fft.rep - special bit-decomposed triples
1 - number of instances (processes) to start in parallel

[Run the mulpar2bitfft peer2]
 bin/mulpar2bitfft minimacs_85_255_2_1_fft.rep bdt_85_255_1of2_6800_fft.rep 1 127.0.0.1

ARGS explained:
As above, the last argument is the ip address of peer1.


Please contact rwl@cs.au.dk for comments and suggestions.
