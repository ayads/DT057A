#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include <random>
#include <fstream>
#include <map>

void poissonDist();
void expDist();
template<class T, int seed, int m, int a, int c> void LCG();
double LCG2();

std::ofstream LCGfile, UVfile;

int main (int argc, char *argv[]){
  /*****Exponential distribution*****/
  //expDist();

  /*****Poisson distribution*****/
  //poissonDist();

  /*****Linear congurental engine implementation *****/
  LCGfile.open("outputLCG.txt");
  LCG<std::uint_fast32_t, 1, 500, 3, 0>();
  LCGfile.close();

  /*****UniformRandomVariable using ns3*****/
  /*UVfile.open("outputUV.txt");
  ns3::Ptr<ns3::UniformRandomVariable> uv = ns3::CreateObject<ns3::UniformRandomVariable> ();
  for(int i=0; i<1000; i++){
    UVfile << std::setprecision(10) << uv->GetValue() << "\t" << i << "\n";
  }
  UVfile.close();*/

  /*****ExponentialRandomVariable using ns3*****/
  /*ns3::Ptr<ns3::ExponentialRandomVariable> ERV = ns3::CreateObject<ns3::ExponentialRandomVariable> ();
  for(int i=0; i<9; i++){
    std::cout << std::setprecision(10) << ERV ->GetValue() << "\t" << i << "\n";
  }*/
  return 0;
}


double LCG2(){
  std::linear_congruential_engine<std::uint_fast32_t, 500, 3, 0> lce(1);
  return double(lce())/lce.max();
}

void poissonDist(){
  std::linear_congruential_engine<std::uint_fast32_t, 500, 3, 0> generator(1);

  std::poisson_distribution<> distribution (1.0);

  for (int i=0; i<10; ++i)
    std::cout << i << "\t" << distribution(generator) << "\n";

  std::cout << std::endl;
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
    std::linear_congruential_engine<T, m, a, c> E(seed);
    long double X = double(E())/E.max();

    for(int i=0; i<1000; i++){
      X = double(E())/E.max();
      //std::cout << std::setprecision(10); //<< X << "\n";
      LCGfile << std::setprecision(10)<< X << "\t"<< i<< "\n";
    }
}
