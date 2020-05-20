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
void expDist();
template<class T, int seed, int m, int a, int c> void LCG();

std::ofstream LCGfile, UVfile;
std::vector<long double> LCG_rand;

int main (int argc, char *argv[]){

  /*****Exponential distribution*****/
  //expDist();

  /*****Linear congurental engine implementation *****/
  LCGfile.open("outputLCG.txt");
  //seed,m,a,c
  //seed,a,c < m
  LCG<std::uint_fast32_t, 1, 500, 3, 0>();
  LCGfile.close();

  /*****Poisson distribution*****/
  poissonDist(4, 10);

  /*****UniformRandomVariable using ns3*****/
  /*UVfile.open("outputUV.txt");
  ns3::Ptr<ns3::UniformRandomVariable> uv = ns3::CreateObject<ns3::UniformRandomVariable> ();
  for(int i=0; i<1000; i++){
    UVfile << std::setprecision(10) << uv->GetValue() << "\t" << i << "\n";
  }
  UVfile.close();*/

  /*****ExponentialRandomVariable using ns3*****/
  ns3::Ptr<ns3::ExponentialRandomVariable> ERV = ns3::CreateObject<ns3::ExponentialRandomVariable> ();
  std::cout << "NS3 EXP:\n";
  for(int i=0; i<9; i++){
    std::cout << std::setprecision(10) << ERV ->GetValue() << " "; //<< "\t" << i
  }
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
  //std::linear_congruential_engine<std::uint_fast32_t, 500, 3, 0> generator(1);
  std::vector<double> poisson, tr;
  std::vector<double> poissonEmp(size-1, 0.0);

  for(int i=0; i<size-1; i++){
    poisson.push_back(pow(lambda,i+1)*exp(-lambda)*(1/factorial(i+1)));
  }

  //for (auto i = poisson.begin(); i != poisson.end(); ++i) std::cout << *i << ' ';
  //std::cout<< "\n";

  for(int i=size-2; i>=0; i--){
    tr.push_back(std::accumulate(poisson.begin(), poisson.end()-i, 0.0));
  }
  //for (auto i = tr.begin(); i != tr.end(); ++i) std::cout << *i << ' ';

  for(int i=0; i<LCG_rand.size(); i++){
    for(int j=0; j<size-1; j++){
      if(LCG_rand[i]<tr[j]){
        poissonEmp[j]++;
        break;
      }
    }
  }

  for (int i=0; i<size-2; i++){
    poissonEmp[i]= poissonEmp[i]/LCG_rand.size();
  }
  std::cout << "POISSON:\n";

  for (auto i = poissonEmp.begin(); i != poissonEmp.end(); ++i) std::cout << *i << ' ';
  std::cout << "\n\n";
  /*std::poisson_distribution<> distribution (1.0);

  for (int i=0; i<10; ++i)
    std::cout << i << "\t" << distribution(generator) << "\n";

  std::cout << std::endl;*/
}

void expDist(){
  std::linear_congruential_engine<std::uint_fast32_t, 500, 3, 0> gen(1);

  std::exponential_distribution<> distr(1.0);
  //std::cout <<std::setprecision(5) << distribution.lambda() << "\n";
  std::cout << std::endl;
  std::cout << "min() == " << distr.min() << std::endl;
  std::cout << "max() == " << distr.max() << std::endl;
  std::cout << "lambda() == " << std::fixed << std::setw(11) << std::setprecision(10) << distr.lambda() << std::endl;

  // generate the distribution as a histogram
  std::map<double, int> histogram;
  for (int i = 0; i < 10; ++i) {
      ++histogram[distr(gen)];
  }

  // print results
  std::cout << "Distribution for " << 10 << " samples:" << std::endl;
  int counter = 0;
  for (const auto& elem : histogram) {
      std::cout << std::fixed << std::setw(11) << ++counter << ": "
          << std::setw(14) << std::setprecision(10) << elem.first << std::endl;
  }
  std::cout << std::endl;
}

template<class T, int seed, int m, int a, int c>
void LCG(){
    //value = (seed*a+c)mod m
    //std::linear_congruential_engine<T, m, a, c> E(seed);
    //long double X = double(E())/E.max();

    long double X = ((seed*a) + c)%m;
    //std::vector<long double> LCG_rand;
    LCG_rand.push_back(X);

    for(int i=0; i<100; i++){
      //X = double(E())/E.max();
      X = (((int)LCG_rand[i]*a) + c)%m;
      LCG_rand.push_back(X);
      //std::cout << std::setprecision(10); //<< X << "\n";
      //LCGfile << std::setprecision(10)<< X << "\n"; //"\t"<< i<<
    }

    std::transform(LCG_rand.begin(), LCG_rand.end(), LCG_rand.begin(), [](long double& y){return y/m;});

    for (auto i = LCG_rand.begin(); i != LCG_rand.end(); ++i){
      //std::cout << *i << ' ';
      LCGfile << *i << "\n";
    }


}
