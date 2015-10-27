#include <iostream>
#include <fstream>
#include <cybozu/random_generator.hpp>
#include <cybozu/crypto.hpp>
#include <mie/fp.hpp>
#include <mie/gmp_util.hpp>
#include <mie/elgamal.hpp>
#include <mie/ecparam.hpp>
#if defined(_WIN64) || defined(__x86_64__)
#define USE_MONT_FP
#endif
#ifdef USE_MONTFP
#include <mie/mont_fp.hpp>
typedef mie::MontFpT<3> Fp;
#else
typedef mie::FpT<mie::Gmp> Fp;
#endif

#include <math.h>

#define uint unsigned int

//#include<sys/time.h>
#include<sys/timeb.h>
#include<omp.h>
struct ZnTag;

typedef mie::EcT<Fp> Ec;
typedef mie::FpT<mie::Gmp, ZnTag> Zn; // use ZnTag because Zn is different class with Fp
typedef mie::ElgamalT<Ec, Zn> Elgamal;

struct CipherTextVec : public std::vector<typename Elgamal::CipherText> {};

namespace ROT{
	void SysInit();

	template<class T>
	bool Load(T& t, const std::string& name, bool doThrow = true)
	{
		std::ifstream ifs(name.c_str(), std::ios::binary);
		if (!ifs) {
			if (doThrow) throw cybozu::Exception("Load:can't read") << name;
			return false;
		}
		if (ifs >> t) return true;
		if (doThrow) throw cybozu::Exception("Load:bad data") << name;
		return false;
	}
	
	template<class T>
	void Save(const std::string& name, const T& t)
	{
		std::ofstream ofs(name.c_str(), std::ios::binary);
		ofs << t;
	}

	class Server{
		int *v0, *v1;		
		int v_length;
		int L0, L1;
		CipherTextVec res_v0, res_v1;
	protected:
		Elgamal::PublicKey pub;
		int prev_r0, prev_r1;
	public:
		int core;
		void setV(int* input, int length, int row, int column);
		void updtV(int* input, int length, int row, int column);
		void readPubkey(std::string& pubFile);
		void getResult(std::string& query, int ran0, int ran1); //query includes ciphertext t0 and a vector of ciphertexts t1. 
		void makeResFile(std::string& result);
		Server(){
			prev_r0=0;
			prev_r1=0;
		}
	};

	class Client{
		int L0, L1;
	protected:
		Elgamal::PublicKey pub;   
		Elgamal::PrivateKey prv;
	public:
		int core;
		int t0, t1;		
		void setParam(int row, int column, std::string& prvf, std::string& pupf);
		void makeQuery(std::string& query, int t0, int t1);
		void decResult(std::string& result, int t);
	};
}

