# **PBWT-sec**

C++ implementation of PBWT-sec

# Summary
This library provides for the implementation of the PBWT-sec [1].

* [1] K. Shimizu, K.Nuida, G. R&auml;tsch, Efficient Privacy-Preserving String Search and an Application in Genomics [doi: http://dx.doi.org/10.1101/018267](http://biorxiv.org/content/early/2015/04/21/018267).

# List of Supported CPUs and Operating Systems
* Intel 64-bit Core 2 Duo or newer Intel 64-bit CPUs
* 64-bit Linux (tested on ubuntu 13.10)

# List of Supported Compilers and Prerequisite Tools
* gcc 4.8.1 + OpenSSL 1.0.1e + GMP 5.1.2
* clang++ 3.0 or newer

# Installation Requirements

Create a working directory (e.g., work) and clone the following repositories.

       mkdir work
       cd work
       git clone git://github.com/iskana/PBWT-sec.git
       git clone git://github.com/herumi/xbyak.git
       git clone git://github.com/herumi/mcl.git
       git clone git://github.com/herumi/cybozulib.git
       git clone git://github.com/herumi/cybozulib_ext.git

* Xbyak is a prerequisite for optimizing the operations in the finite field on Intel CPUs.
* OpenSSL and libgmp-dev are available via apt-get (or other similar commands).

# Installation

  cd PBWT-sec/src
  make

* use tcmalloc (optimal) for Linux; sudo apt-get install libgoogle-perftools-dev

# Prerequisite Files and Libraries for Running Your Application
	* OpenSSL
	* GMP (libgmp-dev)

# Running PBWT-sec with a sample data
Server:
		
		mkdir /tmp/server
		./server -m <num_of_threads> -f ../sample_dat/1000G.pbwt -r 2185 -c 10000 -d /tmp/server/ -k /tmp/server/

Client:

		mkdir /tmp/client
		./client -m <num_of_threads> -h hostname -q ../sample_dat/client_input -d /tmp/client/ -k /tmp/client/

# Query file format
 	   1st line	 : A query bit string (e.g., 0000100010101000)
	   2nd line	 : A true column(allele) index to start the search
	   3rd line  : Either a true column index or a decoy column index for each line
	   ....
	   last line : -1

# Copyright Notice

Copyright (C) 2015, Kana Shimizu
All rights reserved.

# License

PBWT-sec (files in this repository) is distributed under the [BSD 3-Clause License] (http://opensource.org/licenses/BSD-3-Clause "The BSD 3-Clause License").

# Licenses of External Libraries

Licenses of external libraries are listed as follows.

* Lifted-Elgamal: BSD-3-Clause
* cybozulib: BSD-3-Clause
* mie: BSD-3-Clause
* Xbyak: BSD-3-Clause
* MPIR: LGPL2
* OpenSSL: Apache License Version 1.0 + SSLeay License

Software including any of those libraries is allowed to be used for commercial purposes without releasing its source code as long as the regarding copyright statements described as follows are included in the software.

* This product includes software that was developed by an OpenSSL project in order to use OpenSSL toolkit.
* This product includes Lifted-Elgamal, cybozulib, mie, and Xbyak.
* This product includes MPIR.

# Contact Information

* Kana Shimizu (kana-shimizu@aist.go.jp)
* Gunnar Ratsch (Gunnar.Ratsch@ratschlab.org)

# History

2015/Oct/26; initial version
