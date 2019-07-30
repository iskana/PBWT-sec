# **cvSNPSeq2pbwt.cpp**

# Summary
This program converts a SNPs matrix to the pbwt matrix that is input to PBWT-sec.
Each row of the SNPs matrix is a collection of SNPs for the same sample.
Each column of the SNPs matrix is a collection of SNPs for the same allele.

The program outputs pbwt of m by n upper-left submatrix of the input SNPs matrix to STDOUT, where m is the number of samples and n is the number of alleles.
The original m by n SNPs matrix is output to STDERR.

# Compile
		g++ cvSNPSeq2pbwt.cpp -o cvSNPSeq2pbwt

# Example for generating & testing pbwt of 20 by 100 SNPs matrix
		cvSNPSeq2pbwt 20 100 smpl.snps >1.pbwt  2>1.snps

Server:
		
		mkdir /tmp/server
		./server -m <num_of_threads> -f ../sample_dat/gensmpl/1.pbwt -r 20 -c 100 -d /tmp/server/ -k /tmp/server/

Client:

		mkdir /tmp/client
		./client -m <num_of_threads> -h hostname -q ../sample_dat/gensmpl/client_input -d /tmp/client/ -k /tmp/client/
