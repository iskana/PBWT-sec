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

char *addr;
char *infile;
int aflg;
int port;
int max_con;
int core;
int pbwt_n; // positions
int pbwt_m; //samples
int epsilon;
std::string tmp_dir_path;
std::string key_dir_path;

int setParam(int argc, char **argv)
{
	int opt;
	aflg=0;
	max_con = 1;
	int nopt = 0;
	core = 1;
	epsilon = 0;// longest match greater than epsilon
	tmp_dir_path="";
	key_dir_path="";
    while((opt = getopt(argc, argv, "p:a:n:c:r:f:m:d:k:e:")) != -1){
        switch(opt){
        case 'a':
			aflg=1;
			addr = optarg;
            break;
        case 'd':
			tmp_dir_path = optarg;
            break;
        case 'e':
			epsilon = atoi(optarg);
            break;
        case 'k':
			key_dir_path = optarg;
            break;
		case 'n':
			max_con = atoi(optarg);
			break;
        case 'p':
			port = atoi(optarg);
            break;
        case 'c':
			nopt++;
			pbwt_n = atoi(optarg);
            break;
        case 'r':
			nopt++;
			pbwt_m = atoi(optarg);
            break;
        case 'f':
			nopt++;
			infile = optarg;
            break;
		case 'm':			
			core = atoi(optarg);
			break;
        default:
			fprintf(stderr, "Usage: %s [-a address] [-d tmpfile_dir_path] [-e epsilon ][-m threads] [-n max_connections] [-p port] -f pbwt_file -r row -c column\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
	if(nopt==3){
		return(0);
	}else{
		fprintf(stderr, "Usage: %s [-a address] [-d tmpfile_dir_path] [-e epsilon ][-m threads] [-n max_connections] [-p port] -f pbwt_file -r row -c column\n", argv[0]);
		exit(1);
	}	   
}

int main(int argc,char **argv)
{
	double wts, wte, all_s, all_e, calc_total=0, calc_head;;
	wts = get_wall_time();

	ROT::SysInit();

	setParam(argc, argv);
	wte = get_wall_time();
	calc_head = wte - wts;;

	int sock0 = prepSSock();

	while(1){
		CPBWT::Server s;
		std::ifstream ifs;
		std::ofstream ofs;
				
		calc_total = calc_head;
		//		all_s = get_wall_time();
		int sock = acceptSSock(sock0);
		
		std::string file = tmp_dir_path + "c2s_pos";
		recvFile(sock, (char *)file.c_str()); //receive positions from client
		all_s = get_wall_time();
		
		wts = get_wall_time();
		ifs.open(file.c_str(), std::ios::binary);
		int qlen, tmp;
		ifs >> qlen;
		while(1){
			ifs >> tmp;
			if(tmp == -1){
				break;
			}
			s.pos.push_back(tmp);
		}
		ifs.close();
		
		std::cerr<<"read db\n";
		int n=pbwt_n, m=pbwt_m; // DB size 
		s.readPBWT(m,n,infile);
		s.makeLUTable();
		
		file = tmp_dir_path + "param";
		ofs.open(file.c_str(), std::ios::binary);
		//	std::cerr<<s.v_length<<"\n";
		ofs << s.samples  <<"\n";
		ofs << s.v_length <<"\n";
		ofs << s.B0 <<"\n";
		ofs << s.L0 <<"\n";
		ofs << s.L1 <<"\n";
		ofs.close();
		wte = get_wall_time();
		calc_total += wte - wts;
		sendFile(sock, (char *)file.c_str()); //send db parameters to client
		
		std::cerr<<"set db param, pubkey\n";
		file = key_dir_path + "c2s_pubkey";
		recvFile(sock, (char *)file.c_str()); //receive pubkey from client
		s.core = core;
		s.setParam(file);
		
		std::string queryf = tmp_dir_path + "squeryf";
		std::string queryg = tmp_dir_path + "squeryg";
		std::string resultf = tmp_dir_path + "sresultf";
		std::string resultg = tmp_dir_path + "sresultg";
		std::string ismatch = tmp_dir_path + "sismatch";
		
#ifdef DEBUG
		//	std::string q = "1010101010";
		std::string q = "1010101010111111111111111";
		int select = 0;
		int f=select*(s.samples+1),g=(select+1)*(s.samples+1)-1;
		int blk = s.B0*s.L1;
		int zero = '0';
		f += ('1'-zero)*blk; // fix query for debug
		g += ('1'-zero)*blk;
		std::cerr<<"f="<<f<<", g="<<g<<" ,ln="<<g-f<<"\n";
#endif
		for(int i=0;i<qlen;i++){
			wts = get_wall_time();
			std::cerr<<"round: "<<i<<"\n";
			DBG_PRT("update table\n");
			s.updtLUTable();
			
#ifdef DEBUG		
			f=s.retV(f);
			g=s.retV(g);
#endif

			int ranf0, ranf1, rang0, rang1;
#pragma omp parallel
#pragma omp sections
			{
#pragma omp section
				{
					DBG_PRT("get resultf\n");
					ranf0 = rand();
					ranf1 = rand();
					//		ranf0=0; ranf1=0; //for debug
					if(i+1 == qlen){
						ranf0=0; ranf1=0;
					}
				}
#pragma omp section
				{
					DBG_PRT("get resultg\n");
					rang0 = rand();
					rang1 = rand();
					//		rang0=0; rang1=0; //for debug
					if(i+1 == qlen){
						rang0=0; rang1=0;
					}
				}
			}
			wte = get_wall_time();
			calc_total += wte - wts;
			
			DBG_PRT("recv queries\n");
			recvFile(sock, (char *)queryf.c_str()); //receive query f from client
			recvFile(sock, (char *)queryg.c_str()); //receive query g from client
			
			wts = get_wall_time();
			DBG_PRT("f:main server\n");
			s.setPrevFr();
			s.getOrgQuery(queryf, 0);
			s.getResult(queryf, ranf0, ranf1);
			s.storePrevFr();
			s.makeResFile(resultf);
			
			DBG_PRT("g:main server\n");
			s.setPrevGr();
			s.getOrgQuery(queryg, 1);
			s.getResult(queryg, rang0, rang1);
			s.storePrevGr();
			s.makeResFile(resultg);

			if(epsilon==0){
				s.makeIsLongest(ismatch);
			}else{
				s.makeIsELongest(ismatch, epsilon);
			}
			wte = get_wall_time();
			calc_total += wte - wts;
			
			DBG_PRT("send results\n");
			sendFile(sock, (char *)resultf.c_str()); //send result f to client
			sendFile(sock, (char *)resultg.c_str()); //send result g to client
			sendFile(sock, (char *)ismatch.c_str()); //send ismatch to client
			
			for(int j=0;j<s.pos.size();j++){
				s.pos[j]++;
			}
#ifdef DEBUG
			if(i+1 != qlen){
				f += (q[i+1]-zero)*blk;
				g += (q[i+1]-zero)*blk;
				std::cerr<<"k="<<i+1<<", f="<<f<<", g="<<g<<", ln="<<g-f<<"\n";
			}
#endif
		}
		
		closeSock(sock);
		
		all_e = get_wall_time();	 
		//		std::cerr<<"server(Wall): "<<calc_total<<", "<<all_e-all_s<<"\n";
		std::cerr<<"server(Wall): "<<all_e-all_s<<"\n";
	}
	
	return(0);
}  

