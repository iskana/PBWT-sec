#include "cpbwt.h"
#include "comm.h"

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

char *qfile;
char *host;
int port;
int core;
std::string tmp_dir_path;
std::string key_dir_path;

int setParam(int argc, char **argv)
{
	int opt;
	int nopt=0;
	int core=1;
	tmp_dir_path="";
	key_dir_path="";
    while((opt = getopt(argc, argv, "h:p:q:m:d:k:")) != -1){
        switch(opt){ 
        case 'h':
			nopt++;
			host = optarg;
            break;
        case 'p':
			port = atoi(optarg);
            break;
        case 'q':
			nopt++;
			qfile = optarg;
            break;
		case 'm':			
			core = atoi(optarg);
			break;
        case 'd':
			tmp_dir_path = optarg;
            break;
        case 'k':
			key_dir_path = optarg;
            break;
        default:
			fprintf(stderr, "Usage: %s [-d tmpfile_dir_path] [-k key_dir_path] [-m threads] [-p port] -h host -q queryfile\n", argv[0]);
			//            fprintf(stderr, "setup: %s query\n true_column\n dummy_column1\n dummy_column2\n ...\n -1\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
	if(nopt==2){
		return(0);
	}else{
		fprintf(stderr, "Usage: %s [-d tmpfile_dir_path] [-k key_dir_path] [-m threads] [-p port] -h host -q queryfile\n", argv[0]);
		exit(1);
	}	   
}

int main(int argc, char** argv)
{
	int recvSize=0; int sentSize=0;
	double wts, wte, all_s, all_e, calc_total=0;
	all_s = get_wall_time();
	wts = get_wall_time();

	ROT::SysInit();
	CPBWT::Client c;

	std::ifstream ifs;
	std::ofstream ofs;

	setParam(argc, argv);
	wte = get_wall_time();
	calc_total += wte - wts;

	int sock = prepCSock(host);	

	wts = get_wall_time();
	std::string q;
	int select = -1; //secret info
	std::vector<int> sp;

	ifs.open(qfile, std::ios::binary);
	int tmp, stmp;
	ifs >> q;
	q += "00";
	if(ifs.eof()){
		std::cerr<<"invalid param file:"<<qfile<<"\n";
		exit(1);
	}	
	ifs >> stmp;
	if(ifs.eof()){
		std::cerr<<"invalid param file:"<<qfile<<"\n";
		exit(1);
	}
	while(1){
		ifs >> tmp;
		if(tmp == -1)
			break;
		sp.push_back(tmp);
		if(stmp == tmp){
			select = sp.size()-1;
		}
		if(ifs.eof()){
			std::cerr<<"invalid param file:"<<qfile<<"\n";
			exit(1);
		}
	}
	ifs.close();
	//	std::string q = "1010101010111111111111111";

	if(select == -1){
		std::cerr<<"the true column was not found in the list of the true column and decoys:"<<qfile<<"\n";
		exit(1);
	}

	std::string file = tmp_dir_path + "pos"; // column1\n column2\n ...	
	ofs.open(file.c_str(), std::ios::binary);
	ofs << q.size() <<"\n";
	for(int i=0;i<sp.size();i++){
		ofs << sp[i] <<"\n";
	}
	ofs<<"-1\n";
	ofs.close();
	wte = get_wall_time();
	calc_total += wte - wts;

	sentSize += sendFile(sock, (char *)file.c_str()); //send round and positions to server
	
	std::cerr<<"set param\n";
	file = tmp_dir_path + "s2c_param";
	recvSize += recvFile(sock, (char *)file.c_str()); //receive db parameters from server

	wts = get_wall_time();

	ifs.open(file.c_str(), std::ios::binary);
	int vlen, B0, L0, L1;
	ifs >> c.samples; ifs >> vlen; ifs >> B0; ifs >> L0; ifs >>L1;
	ifs.close();

	file = key_dir_path + "pubkey";
	std::string prvkf = key_dir_path + "prvkey";
	c.setParam(vlen, B0, L0, L1, (char *)file.c_str(), (char *)prvkf.c_str());
	c.core = core;

	wte = get_wall_time();
	calc_total += wte - wts;	

	sentSize += sendFile(sock, (char *)file.c_str()); //send pubkey to server

	std::string queryf = tmp_dir_path + "cqueryf";
	std::string queryg = tmp_dir_path + "cqueryg";
	std::string resultf = tmp_dir_path + "cresultf";
	std::string resultg = tmp_dir_path + "cresultg";
	std::string ismatch = tmp_dir_path + "cismatch";

	//	std::cerr<<c.samples<<", "<<c.v_length<<", "<<c.B0<<", "<<c.L0<<", "<<c.L1<<"\n";
	int f=select*(c.samples+1),g=(select+1)*(c.samples+1)-1;
	int ff=0, gg=0;
	int blk = c.B0*c.L1;
	int zero = '0';
	f += (q[0]-zero)*blk;
	g += (q[0]-zero)*blk;
	std::cerr<<"f="<<f<<", g="<<g<<" ,ln="<<g-f<<"\n";
	ff=f;
	gg=g;
	int isL=-1, maxmatch=0;

	for(int i=0;i<q.size();i++){
	wts = get_wall_time();
	omp_set_num_threads(2);
#pragma omp parallel
#pragma omp sections
{
    #pragma omp section
    {
		if(isL==0){
			DBG_PRT("use dummy query\n");
		}else{
			DBG_PRT("mk queryf\n");
			c.makeQuery(queryf, (ff/c.L1)%c.L0, ff%c.L1);
		}
	}
    #pragma omp section
    {
		if(isL==0){
			DBG_PRT("use dummy query\n");
		}else{
			DBG_PRT("mk queryg\n");
			c.makeQuery(queryg, (gg/c.L1)%c.L0, gg%c.L1);
		}
	}
}
        wte = get_wall_time();
	    calc_total += wte - wts;	

     	DBG_PRT("send queries\n");
		sentSize += sendFile(sock, (char *)queryf.c_str()); //send query f to server
		sentSize += sendFile(sock, (char *)queryg.c_str()); //send query g to server

        DBG_PRT("receive results\n");
		recvSize += recvFile(sock, (char *)resultf.c_str()); //receive query f from server
		recvSize += recvFile(sock, (char *)resultg.c_str()); //receive query g from server
		recvSize += recvFile(sock, (char *)ismatch.c_str()); //receive ismatch from server
		
		wts = get_wall_time();
		if(isL!=0){
			DBG_PRT("dec result f\n");
			c.decResult(resultf, (ff/c.L1)%c.L0);
			ff = c.L1*c.t0 + c.t1;
			
			DBG_PRT("dec result g\n");	
			c.decResult(resultg, (gg/c.L1)%c.L0);
			gg = c.L1*c.t0 + c.t1;

			isL=c.chkIsLongest(ismatch);
			std::cerr<<"is longest? (yes:0, no:-1)"<<isL<<"\n";
			if(isL==0)
				maxmatch = i - 1;
		
			if(i+1 != q.size()){
				ff += (q[i+1]-zero)*blk;
				gg += (q[i+1]-zero)*blk;			
				DBG_PRT("randomized f=%d, randomized g=%d\n", ff, gg);
			}
		}else{
			std::cerr<<"receive dummy\n";
		}
		wte = get_wall_time();
	    calc_total += wte - wts;
	}

	if(isL!=0 || maxmatch > q.size() -2){ maxmatch = q.size() - 2;}
	std::cerr<<maxmatch<<"-mer match\n";
	for(int i=0;i<maxmatch;i++)
		std::cerr<<q[i];
	std::cerr<<"\n";
	
	closeSock(sock);

	all_e = get_wall_time();	 
	//	std::cerr<<"client(Wall): "<<calc_total<<", "<<all_e-all_s<<"\n";
	std::cerr<<"client(Wall): "<<all_e-all_s<<"\n";
	std::cerr.precision(3);
	std::cerr<<"c2s, s2c(Mbyte): " <<(double)sentSize/(1024*1024)<<", "<<(double)recvSize/(1024*1024)<<"\n";
	return (0);
}
