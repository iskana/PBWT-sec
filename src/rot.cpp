#include <cybozu/option.hpp>
#include "rot.h"

cybozu::RandomGenerator rg;

//#define DEBUG_MAIN

void ROT::SysInit()
{
	const mie::EcParam& para = mie::ecparam::secp192k1;
	Zn::setModulo(para.n);
	Fp::setModulo(para.p);
	Ec::setParam(para.a, para.b);
}

void ROT::Server::setV(int* input, int length, int row, int column)
{
	v_length = length;
	L0 = row;
	L1 = column;
	v0 = (int*)malloc(sizeof(int)*v_length);
	v1 = (int*)malloc(sizeof(int)*v_length);

	for(int i = 0;i < v_length;i++){
		v0[i] = input[i]/L1;
		v1[i] = input[i]%L1;
	}
}

void ROT::Server::updtV(int* input, int length, int row, int column)
{
	for(int i = 0;i < v_length;i++){
		v0[i] = input[i]/L1;
		v1[i] = input[i]%L1;
	}
}

void ROT::Server::readPubkey(std::string& pubFile)
{
	Load(pub, pubFile);
}

/**
void ROT::Server::getResult(std::string& query, int ran0, int ran1)
{
	Elgamal::CipherText et0;
	CipherTextVec et1;
	std::ifstream ifs(query.c_str(), std::ios::binary);
	ifs >> et0;

	Elgamal::CipherText c;
	Elgamal::CipherText d;
	Elgamal::CipherText e;

	for(int i=0;i<L1;i++){
		ifs >> c;
		et1.push_back(c);
	}

	res_v0.resize(L0*2);
	res_v1.resize(L0*2);

	CipherTextVec::iterator it;
	CipherTextVec::iterator start;
	int L, r, tmpr=0, ran;
	int *v;
	for(int x=0;x<2;x++){
		if(x==0){
			v=v0;
			L=L0;
			ran=ran0;
			it = res_v0.begin();
			start = res_v0.begin();
		}else{
			v=v1;
			L=L1;
			ran=ran1;
			it = res_v1.begin();
			start = res_v1.begin();
		}
		for(int i=0;i<prev_r0;i++){
			it++;
			it++;
		}
		r = ran%L;
		for(int i=0;i<L0;i++){
			e.clear();
			for(int j=0;j<L1;j++){
				int k = v[i*L1+j];
				k += r;
				k = k%L;
				c = et1[ (j+prev_r1)%L1 ];
				c.mul(k);
				e.add(c);
			}
			pub.enc(c,(i + prev_r0),rg);
			c.neg();
			c.add(et0);
			int tr = rand();
			c.mul(tr);
			c.add(e);
			
			pub.enc(d,(i + prev_r0 - L0),rg);
			d.neg();
			d.add(et0);
			tr = rand();
			d.mul(tr);
			d.add(e);
			
			if(i+prev_r0==L0){
				it = start;
			}
			int sel = rg.get32()%2;
			if(sel==0){
				*it=c;
				it++;
				*it=d;
				it++;
			}else{
				*it=d;
				it++;
				*it=c;
				it++;
			}
	    }
		if(x==0)
			tmpr = r;
		else
			prev_r1 = r;
	}
	prev_r0 = tmpr;
	v=NULL;
}
**/

void ROT::Server::getResult(std::string& query, int ran0, int ran1)
{
	Elgamal::CipherText et0;
	CipherTextVec et1;
	std::ifstream ifs(query.c_str(), std::ios::binary);
	ifs >> et0;

	Elgamal::CipherText a;
	for(int i=0;i<L1;i++){
		ifs >> a;
		et1.push_back(a);
	}

	res_v0.resize(L0*2);
	res_v1.resize(L0*2);

	int L, r, tmpr=0, ran;
	int *v;
	CipherTextVec *it;
	for(int x=0;x<2;x++){
		if(x==0){
			v=v0;
			L=L0;
			ran=ran0;
		}else{
			v=v1;
			L=L1;
			ran=ran1;
		}
		r = ran%L;
		omp_set_num_threads(core);
		omp_set_nested(1);
		#pragma omp parallel for
		for(int i=0;i<L0;i++){
			Elgamal::CipherText c;
			Elgamal::CipherText d;
			Elgamal::CipherText e;
			for(int j=0;j<L1;j++){
				int k = v[i*L1+j];
				k += r;
				k = k%L;
				c = et1[ (j+prev_r1)%L1 ];
				c.mul(k);
				e.add(c);
			}

			Zn rn;
			pub.enc(c,(i + prev_r0),rg);
			c.neg();
			c.add(et0);
			rn.setRand(rg);
			c.mul(rn);
			c.add(e);

			pub.enc(d,(i + prev_r0 - L0),rg);
			d.neg();
			d.add(et0);
			rn.setRand(rg);
			d.mul(rn);
			d.add(e);

			int pos = prev_r0+i;
			if(L0<=pos){
				pos = pos%L0;
			}
			int sel = rg.get32()%2;

			if(sel==0){
				switch(x){
				case 0:
					res_v0[2*pos] = c;
					res_v0[2*pos+1] = d;
					break;
				case 1:
					res_v1[2*pos] = c;
					res_v1[2*pos+1] = d;
					break;
				}
			}else{
				switch(x){
				case 0:
					res_v0[2*pos] = d;
					res_v0[2*pos+1] = c;
					break;
				case 1:
					res_v1[2*pos] = d;
					res_v1[2*pos+1] = c;
					break;
				}
			}
	    }
		if(x==0)
			tmpr = r;
		else
			prev_r1 = r;
	}
	prev_r0 = tmpr;
	v=NULL;
}

void ROT::Server::makeResFile(std::string& result)
{
	std::ofstream ofs(result.c_str(), std::ios::binary);
	for(int i=0;i<2*L0;i+=2){
		ofs << res_v0[i];
		ofs << "\n";
		ofs << res_v0[i+1];
		ofs << "\n";

		ofs << res_v1[i];
		ofs << "\n";
		ofs << res_v1[i+1];
		ofs << "\n";
	}	
}

 void ROT::Client::setParam(int row, int column, std::string& prvFile, std::string& pubFile)
 {		
	L0 = row;
	L1 = column;
	
	const mie::EcParam& para = mie::ecparam::secp192k1;
	const Fp x0(para.gx);
	const Fp y0(para.gy);
	const Ec P(x0, y0);
	const size_t bitLen = Zn(-1).getBitLen();

	Elgamal::PrivateKey prvt;
	prvt.init(P, bitLen, rg);
	const Elgamal::PublicKey& pubt = prvt.getPublicKey();

	fprintf(stderr,"make privateKey=%s, publicKey=%s\n", prvFile.c_str(), pubFile.c_str());
	Save(prvFile, prvt);
	Save(pubFile, pubt);

	Load(pub, pubFile);
	Load(prv, prvFile);

	if(L0<L1)
		prv.setCache(0, L1+1); // set cache for prv
	else
		prv.setCache(0, L0+1);
}

void ROT::Client::makeQuery(std::string& query,int t0, int t1)
{
	std::ofstream ofs(query.c_str(), std::ios::binary);
	Elgamal::CipherText c;
	CipherTextVec tmp;
	tmp.resize(L1);
	pub.enc(c, t0, rg);
	ofs << c;
	ofs << "\n";
	int m=0;
	omp_set_num_threads(core);
	omp_set_nested(1);
#pragma omp parallel for
	for(int i=0;i<L1;i++){
		Elgamal::CipherText d;
		if(i==t1){
			m=1;
		}else{
			m=0;
		}
		pub.enc(d, m, rg);
		tmp[i]=d;
		//		ofs << c;
		//		ofs << "\n";
	}

	for(int i=0;i<L1;i++){
		ofs << tmp[i];
		ofs << "\n";
	}
}

void ROT::Client::decResult(std::string& result, int t)
{
	Elgamal::CipherText c, d;
	std::ifstream ifs(result.c_str(), std::ios::binary);

	int x, y;
	bool xc, yc;
	for(int i=0;i<=t;i++){
		ifs >> c;
		ifs >> d;
		if(i == t){
			x = prv.dec(c, &xc);
			y = prv.dec(d, &yc);
			if(x<L0 && xc){
				t0=x;
				if(y<L0 && yc){					
					std::cerr<<"error(t0): ";
					std::cerr<<x<<","<<y<<"\n";
				}				
			}else{
				t0=y;
			}		
		}

		ifs >> c;
		ifs >> d;
		if(i == t){
			x = prv.dec(c, &xc);
			y = prv.dec(d, &yc);
			if(x<L1 && xc){
				t1=x;
				if(y<L1 && yc){					
					std::cerr<<"error(t1): ";
					std::cerr<<x<<","<<y<<"\n";
				}				
			}else{
				t1=y;
			}		
		}
	}
}

#ifdef DEBUG_MAIN
int main(int argc, char** argv)
{
	ROT::SysInit();

	ROT::Client c;
	std::string prvk = "prvkey";
	std::string pubk = "pubkey";

	int row=30, column=100;
	c.setParam(row, column, prvk, pubk);

	ROT::Server s;
	int len = row*column;
	int* input = (int*)malloc(sizeof(int)*len);
	for(int i=0;i<len;i++){
		input[i] = rand()%len;
		std::cerr<<input[i]<<",";
	}

	int itr=10;
	int out=0;
	std::cerr<<"\n";
	for(int i=0;i<itr+1;i++){
		out = input[out];
		std::cerr<<out<<"\n";
	}

	s.setV(input, len, row, column);
	s.readPubkey(pubk);

	std::string query = "query";
	c.makeQuery(query, 0, 0);
	s.getResult(query, 999,1234);
	std::string result = "result";
	s.makeResFile(result);
	c.decResult(result, 0);
	std::cerr<<"\n"<<c.t0<<","<<c.t1<<"\n";

	for(int i=0;i<itr-1;i++){
		c.makeQuery(query, c.t0, c.t1);
		s.getResult(query, rand(), rand());
		s.makeResFile(result);
		c.decResult(result, c.t0);
		std::cerr<<c.t0<<","<<c.t1<<"\n";
	}

	c.makeQuery(query, c.t0, c.t1);
	s.getResult(query, 0, 0);
	s.makeResFile(result);
	c.decResult(result, c.t0);
	std::cerr<<c.t0<<","<<c.t1<<"\n";
	std::cerr<<c.t0*column+c.t1<<"\n";
	return(0);
}
#endif
