#include "cpbwt.h"

#include<sys/time.h>

//#define DEBUG

#ifdef DEBUG
#define DBG_PRT(...)  printf(__VA_ARGS__)
#else
#define DBG_PRT(...)  
#endif

double get_wall_time(){
	struct timeval time;
	if (gettimeofday(&time, NULL)){
		return 0;
	}
	return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

extern cybozu::RandomGenerator rg;

int score;
int ccore;
int dummy;

int setParam(int argc, char **argv)
{
	int opt;
	score=1;
	ccore=1;
	dummy=0;
    while((opt = getopt(argc, argv, "c:m:d:")) != -1){
        switch(opt){
		case 'm':			
			score = atoi(optarg);
			break;
        case 'c':
			ccore = atoi(optarg);			
            break;
        case 'd':
			dummy = atoi(optarg);			
            break;
        default:
			fprintf(stderr, "Usage: %s [-m s_threads] [-c c_threads] [-d dummy]\n", argv[0]);
			exit(EXIT_FAILURE);
        }
    }
}


int main(int argc, char** argv)
{
	double all_s, all_e;
	all_s = get_wall_time();

	setParam(argc, argv);

	CPBWT::Server s;
	CPBWT::Client c;

	ROT::SysInit();
	
	int n=10000, m=2185;
	s.pos.push_back(0);
	for(int i=0;i<dummy;i++){
		s.pos.push_back(i);
	}

	s.core = score;
	c.core = ccore;

	s.readPBWT(m,n,"../sample_dat/1000G.pbwt");
	s.makeLUTable();
	
	std::cerr<<"set param\n";
	c.setParam(s.v_length, s.B0, s.L0, s.L1, "pubkey", "prvkey");  // keygen
	std::string pkeyn="pubkey";
	s.setParam(pkeyn); //keyset
	
	c.samples = s.samples;

	//		std::string q = "011001011000001010101010100101010101010010110010100101010101001010101010101010";
		std::string q = "1010101010111111111111111";
	//  std::string q = "0000010001000001001001000";
	//	std::string q = "01";
	
	std::string queryf = "queryf";
	std::string queryg = "queryg";
	std::string resultf = "resultf";
	std::string resultg = "resultg";
	std::string ismatch = "ismatch";

	int t=0;
	int f=t*(c.samples+1),g=(t+1)*(c.samples+1)-1;
	int ff=0, gg=0;
	int blk = s.B0*s.L1;
	int zero = '0';
	f += (q[0]-zero)*blk;
	g += (q[0]-zero)*blk;
	std::cerr<<"f="<<f<<", g="<<g<<" ,ln="<<g-f<<"\n";
	ff=f;
	gg=g;

	for(int i=0;i<q.size();i++){
		std::cerr<<"update table\n";
		s.updtLUTable();

		f=s.retV(f);
		g=s.retV(g);

		int ranf0, ranf1, rang0, rang1;
#pragma omp parallel
#pragma omp sections
{
    #pragma omp section
    {
		std::cerr<<"f:pre------------------\n";
		std::cerr<<"mk query\n";
		c.makeQuery(queryf, (ff/c.L1)%c.L0, ff%c.L1);

		std::cerr<<"get result\n";
		ranf0 = rand();
		ranf1 = rand();
		if(i+1 == q.size()){
			ranf0=0; ranf1=0;
		}
	}
    #pragma omp section
    {
		std::cerr<<"g:pre------------------\n";
		std::cerr<<"mk query\n";
		c.makeQuery(queryg, (gg/c.L1)%c.L0, gg%c.L1);

		std::cerr<<"get result\n";
		rang0 = rand();
		rang1 = rand();
		if(i+1 == q.size()){
			rang0=0; rang1=0;
		}
	}
}
		std::cerr<<"f:main------------------\n";
		s.setPrevFr();
		s.getOrgQuery(queryf, 0);
		s.getResult(queryf, ranf0, ranf1);
		s.storePrevFr();
		s.makeResFile(resultf);

		std::cerr<<"f:dec------------------\n";
		std::cerr<<"dec result\n";
		c.decResult(resultf, (ff/c.L1)%c.L0);
		ff = c.L1*c.t0 + c.t1;

		std::cerr<<"g:main------------------\n";
		s.setPrevGr();
		s.getOrgQuery(queryg, 1);
		s.getResult(queryg, rang0, rang1);
		s.storePrevGr();
		s.makeResFile(resultg);

		std::cerr<<"g:dec------------------\n";
		std::cerr<<"dec result\n";	
		c.decResult(resultg, (gg/c.L1)%c.L0);
		gg = c.L1*c.t0 + c.t1;

		s.makeIsLongest(ismatch);
		std::cerr<<"is longest? (yes:0, no:-1)"<<c.chkIsLongest(ismatch)<<"\n";

		for(int j=0;j<s.pos.size();j++){
			s.pos[j]++;
		}
		if(i+1 != q.size()){
			f += (q[i+1]-zero)*blk;
			g += (q[i+1]-zero)*blk;

			ff += (q[i+1]-zero)*blk;
			gg += (q[i+1]-zero)*blk;
			std::cerr<<"k="<<i+1<<", f="<<f<<", g="<<g<<", ln="<<g-f<<", ";
			std::cerr<<"ff="<<ff<<", gg="<<gg<<", ln="<<gg-ff<<"\n";
		}
	}

	std::cerr<<"k="<<q.size()<<", f="<<f<<", g="<<g<<", ln="<<g-f<<", ";
	std::cerr<<"ff="<<ff<<", gg="<<gg<<", ln="<<gg-ff<<"\n";

	all_e = get_wall_time();	 
	std::cerr<<"Comp(Wall): "<<all_e-all_s<<"\n";	
	return(0);
}
