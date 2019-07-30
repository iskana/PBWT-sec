#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<iostream>
#include<fstream>
#include<vector>


using namespace std;

#define RATIO 10
#define SHOW_ORG

int main(int argc, char** argv){

  if(argc==1){
    cout<<"cvSNPSeq2pbwt: <num_of_samples> <num_of_alleles/positions> <file>\n";
    exit(1);
  }
  int m = atoi(argv[1]);
  int n = atoi(argv[2]);
  int t;  
  int *seq = (int*)malloc(sizeof(int)*n*m);
  int *pfx = (int*)malloc(sizeof(int)*n*m);

  string tmps;
  ifstream ifs(argv[3]);
  if(ifs==NULL){
    cerr<<"can't find "<<argv[3]<<"\n";
    exit(1);
  }

 for(int j=0; j<m; j++){
    ifs>>tmps;
  for(int i=0; i<n; i++){
      if(tmps.c_str()[i] == '0')
	seq[i*m+j] = 0;
      else
	seq[i*m+j] = 1;
    }
  }

  // make prefix arrays
  vector<int>id;
  vector<int>z;
  vector<int>o;
  for(int j=0; j<m; j++){
    id.push_back(j);
  }

  for(int j=0; j<m; j++){
    pfx[j]=seq[id[j]];
  }

  for(int i=0; i<n-1; i++){
    z.clear();
    o.clear();
    for(int j=0; j<m; j++){
      if(seq[i*m+id[j]]==0){
	z.push_back(id[j]);
      }else{
	o.push_back(id[j]);
      }
    }
    for(int k=0;k<z.size();k++){
      pfx[(i+1)*m+k]=seq[(i+1)*m+z[k]];
    }
    for(int k=0;k<o.size();k++){
      pfx[(i+1)*m+k+z.size()]=seq[(i+1)*m+o[k]];
    }
    
    id.clear();
    for(int k=0;k<z.size();k++){
      id.push_back(z[k]);
    }
    for(int k=0;k<o.size();k++){
      id.push_back(o[k]);
    }
  }

#ifdef SHOW_ORG
  for(int j=0; j<m; j++){
    for(int i=0; i<n; i++){
      cerr<<seq[i*m+j];
    }
    cerr<<"\n";
  }
  //  cerr<<"----\n";
#endif
  
  for(int i=0; i<n; i++){
    for(int j=0; j<m; j++){
      cout<<pfx[i*m+j];
    }
    cout<<"\n";
  }  
  return(0);

}
