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
#include <cassert>
#include <cmath>


double factorial(int n);
void poissonDist(std::vector<long double> rand, int lambda, int size);
void inverseTransform(double lambda);
void expDist();
template<class T, int seed, int m, int a, int c> void LCG();

std::ofstream LCGfile, UVfile;
std::vector<long double> lcgvalues;

int main (int argc, char *argv[]){
  /*****Linear congurental engine implementation *****/
  LCGfile.open("outputLCG.txt");
  LCG<std::uint_fast32_t, 1, 500, 3, 0>();      //seed,m,a,c ::::: seed,a,c < m
  LCGfile.close();

  /*****Exponential distribution*****/
  expDist();

  /*****inverse transform method*****/
  inverseTransform(1.0);

  /*****Poisson distribution*****/
  poissonDist(lcgvalues, 4, 10);

  /*****ExponentialRandomVariable using ns3*****/
  ns3::Ptr<ns3::ExponentialRandomVariable> ERV = ns3::CreateObject<ns3::ExponentialRandomVariable> ();
  
  // Print values
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

/*****Generate Poisson Distribution RV*****/
void poissonDist(std::vector<long double> rand, int lambda, int size){
  std::vector<double> poisson, tr;
  std::vector<double> poissonEmp(size-1, 0.0);

  for(int i=0; i<size-1; i++){
    poisson.push_back(pow(lambda,i+1)*exp(-lambda)*(1/factorial(i+1)));
  }
  for(int i=size-2; i>=0; i--){
    tr.push_back(std::accumulate(poisson.begin(), poisson.end()-i, 0.0));
  }
  for(int i=0; i<rand.size(); i++){
    for(int j=0; j<size-1; j++){
      if(rand[i]<tr[j]){
        poissonEmp[j]++;
        break;
      }
    }
  }

  for (int i=0; i<size-2; i++){
    poissonEmp[i]= poissonEmp[i]/rand.size();
  }

  // Print values
  std::cout << "POISSON:\n";
  for (auto i = poissonEmp.begin(); i != poissonEmp.end(); ++i) std::cout << *i << ' ';
  std::cout << "\n\n";
}

/*****Inverse transform method to Generate Exponentially Distributed RV using LCG function values*****/
void inverseTransform(double lambda){
  std::vector<double> exponentialvalues;

  for(int j= 0; j < 10; j++){
    double uniformValue = 1-lcgvalues[j];
    exponentialvalues.push_back(-(log(uniformValue))/lambda);
  }

  // Print values
  std::cout << "\nInverse transformed values: \n";
  for(int i=0; i< exponentialvalues.size(); i++){
    std::cout << exponentialvalues[i] << "\n";
  }
}

/*****Generate Exponentially Distributed RV*****/
void expDist(){
  std::linear_congruential_engine<std::uint_fast32_t, 500, 3, 0> gen(1);
  std::exponential_distribution<> distr(1.0);

  // generates the distribution as a histogram
  std::map<double, int> histogram;
  for (int i = 0; i < 10; ++i) {
      ++histogram[distr(gen)];
  }

  // Print values
  std::cout << "Distribution for " << 10 << " samples:" << std::endl;
  int counter = 0;
  for (const auto& elem : histogram) {
      std::cout << std::fixed << std::setw(11) << ++counter << ": "
          << std::setw(14) << std::setprecision(10) << elem.first << std::endl;
  }
  std::cout << std::endl;
}

/*****Generate Uniform Distributed RV*****/
template<class T, int seed, int m, int a, int c>
void LCG(){
    long double X = ((seed*a) + c)%m;
    lcgvalues.push_back(X);

    for(int i=0; i<100; i++){
      X = (((int)lcgvalues[i]*a) + c)%m;
      lcgvalues.push_back(X);

    }
    std::transform(lcgvalues.begin(), lcgvalues.end(), lcgvalues.begin(), [](long double& y){return y/m;});
}