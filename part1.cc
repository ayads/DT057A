#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <random>
#include <fstream>
#include <map>
#include <vector>
#include <iostream>
#include <functional>
#include <math.h>
#include <numeric>

double factorial(int n);
void poissonDist(int lambda, int size);
void expDist(int lambda);
template<class T, int seed, int m, int a, int c> void LCG();

std::ofstream LCGfile, UVfile, EXPfile, EXP_NS3file, POISSONfile;
std::vector<long double> LCG_rand, exp_rand;

int main (int argc, char *argv[]){

  /*****Linear congurental engine implementation *****/
  LCGfile.open("outputLCG.txt");
  //seed,m,a,c
  //seed,a,c < m
  LCG<std::uint_fast32_t, 1, 500, 3, 0>();
  LCGfile.close();

  /*****Exponential distribution*****/
  EXPfile.open("outputEXP.txt");
  expDist(1);
  EXPfile.close();

  /*****Poisson distribution*****/
  POISSONfile.open("outputPOISSON.txt");
  poissonDist(4, 10);
  POISSONfile.close();

  /*****UniformRandomVariable using ns3*****/
  UVfile.open("outputUV.txt");
  ns3::Ptr<ns3::UniformRandomVariable> uv = ns3::CreateObject<ns3::UniformRandomVariable> ();
  for(int i=0; i<1000; i++){
    UVfile << std::setprecision(10) << uv->GetValue() << "\t" << i << "\n";
  }
  UVfile.close();

  /*****ExponentialRandomVariable using ns3*****/
  EXP_NS3file.open("outputNS3EXP.txt");
  ns3::Ptr<ns3::ExponentialRandomVariable> ERV = ns3::CreateObject<ns3::ExponentialRandomVariable> ();
  //std::cout << "NS3 EXP:\n";
  ERV->SetAttribute ("Bound", ns3::DoubleValue (1.0)); //[0,1.0] upper bound
  for(int i=0; i<1000; i++){
    EXP_NS3file << ERV ->GetValue() << "\n"; //<< "\t" << i
  }
  EXP_NS3file.close();
  std::cout<<"\n";
  return 0;
}

double factorial(int n)
{
    if(n > 1)
        return n * factorial(n - 1);
    else
        return 1;
}

void poissonDist(int lambda, int size){
  std::vector<double> poisson, tr;
  std::vector<double> poissonEmp(size-1, 0.0);

 //Poisson fördelning
  for(int i=0; i<size; i++){
    poisson.push_back(pow(lambda,i+1)*exp(-lambda)*(1/factorial(i+1)));
  }

  for(int i=size-1; i>=0; i--){
    tr.push_back(std::accumulate(poisson.begin(), poisson.end()-i, 0.0));
  }
  //for (auto i = tr.begin(); i != tr.end(); ++i) std::cout << *i << ' ';

  for(int i=0; i<LCG_rand.size(); i++){
    for(int j=0; j<size; j++){
      if(exp_rand[i]<tr[j]){
        poissonEmp[j]++;
        break;
      }
    }
  }

  for (int i=0; i<size-1; i++){
    poissonEmp[i]= poissonEmp[i]/exp_rand.size();
  }

  for (auto i = poissonEmp.begin(); i != poissonEmp.end(); ++i) POISSONfile << *i << '\n';
}

void expDist(int lambda){

  for (int i = 0; i < 1000; ++i) {
      exp_rand.push_back(lambda*exp((-lambda)*LCG_rand[i]));
  }

  for (auto i = exp_rand.begin(); i != exp_rand.end(); ++i) EXPfile << *i << "\n";

  //std::cout << "\n";
}

template<class T, int seed, int m, int a, int c>
void LCG(){
    long double X = ((seed*a) + c)%m;
    LCG_rand.push_back(X);

    for(int i=0; i<1000; i++){
      X = (((int)LCG_rand[i]*a) + c)%m;
      LCG_rand.push_back(X);
    }

    std::transform(LCG_rand.begin(), LCG_rand.end(), LCG_rand.begin(), [](long double& y){return y/m;});

    int index=0;
    for (auto i = LCG_rand.begin(); i != LCG_rand.end(); ++i){
        LCGfile << *i << "\t" << index << "\n";
        index++;
    }

}
