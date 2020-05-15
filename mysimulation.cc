/* 
 * DT057A - Performance Analysis and Simulation of Communication
 * Systems: Project
 * Date: 2020-May-13
 * Author: Amanda Ericson (amer1501) and Ayad Shaif (aysh1500)
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"
#include <random>
#include <fstream>
#include <iomanip>
#include <map>


void poissonDist();
void expDist();
template<class T, int seed, int m, int a, int c> void LCG();
double LCG2();

std::ofstream LCGfile, UVfile;

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("project_Part3");
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


  bool tracing = false;
  
  // Allow the user to override any of the defaults at run-time.
  CommandLine cmd;
  cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer n_container;
  n_container.Create (6);

  // Create sensors.
  NodeContainer n_AG = NodeContainer (n_container.Get (0), n_container.Get (1));
  NodeContainer n_BG = NodeContainer (n_container.Get (0), n_container.Get (2));
  NodeContainer n_CG = NodeContainer (n_container.Get (0), n_container.Get (3));
  NodeContainer n_DG = NodeContainer (n_container.Get (0), n_container.Get (4));

  // Create actuator.
  NodeContainer n_EG = NodeContainer (n_container.Get (0), n_container.Get (5));

  // Setting global routing.
  NS_LOG_INFO ("Enabling Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();  

  // Create channels for sensors and actuator.
  NS_LOG_INFO ("Create channels to all devices.");
  PointToPointHelper p2p;

  // Between sensors A B C D and gateway G.
  p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  NetDeviceContainer net_dev_AG = p2p.Install (n_AG);
  NetDeviceContainer net_dev_BG = p2p.Install (n_BG);
  NetDeviceContainer net_dev_CG = p2p.Install (n_CG);
  NetDeviceContainer net_dev_DG = p2p.Install (n_DG);

  // Between actuator E and gateway G.
  p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  NetDeviceContainer net_dev_EG = p2p.Install (n_EG);

  // Add Internet stack helper to make use of IP addresses.
  InternetStackHelper internet;
  internet.Install (n_container);
  
  // Assign IP addresses to the channels and implement them correspondingly.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interface_container_AG = ipv4.Assign (net_dev_AG);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interface_container_BG = ipv4.Assign (net_dev_BG);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer interface_container_CG = ipv4.Assign (net_dev_CG);
  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer interface_container_DG = ipv4.Assign (net_dev_DG);
  ipv4.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer interface_container_EG = ipv4.Assign (net_dev_EG);
  
  // Create the OnOff application to send UDP datagrams of size 210 bytes at a rate of 250kbps 
  // from A to G, B to G, C to G, D to G and E to G.
  NS_LOG_INFO ("Create UDP Applications.");
  uint16_t port = 9;
  
  // Packets from sensors
  OnOffHelper onoff_AG("ns3::UdpSocketFactory", InetSocketAddress (interface_container_AG.GetAddress(1), port));
  onoff_AG.SetConstantRate (DataRate ("250kbps"));
  OnOffHelper onoff_BG("ns3::UdpSocketFactory", InetSocketAddress (interface_container_BG.GetAddress(1), port));
  onoff_BG.SetConstantRate (DataRate ("250kbps"));
  OnOffHelper onoff_CG("ns3::UdpSocketFactory", InetSocketAddress (interface_container_CG.GetAddress(1), port));
  onoff_CG.SetConstantRate (DataRate ("250kbps"));
  OnOffHelper onoff_DG("ns3::UdpSocketFactory", InetSocketAddress (interface_container_DG.GetAddress(1), port));
  onoff_DG.SetConstantRate (DataRate ("250kbps"));
  
  ApplicationContainer onOffApp_A = onoff_AG.Install (n_container.Get(0));
  onOffApp_A.Start(Seconds (10.0));
  onOffApp_A.Stop(Seconds (20.0));
  ApplicationContainer onOffApp_B = onoff_AG.Install (n_container.Get(1));
  onOffApp_B.Start(Seconds (10.0));
  onOffApp_B.Stop(Seconds (20.0));
  ApplicationContainer onOffApp_C = onoff_AG.Install (n_container.Get(2));
  onOffApp_C.Start(Seconds (10.0));
  onOffApp_C.Stop(Seconds (20.0));
  ApplicationContainer onOffApp_D = onoff_AG.Install (n_container.Get(3));
  onOffApp_D.Start(Seconds (10.0));
  onOffApp_D.Stop(Seconds (20.0));
  
  // Create packet sinks to receive these packets
  PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port));
  NodeContainer sinks = NodeContainer(n_container.Get(5), n_container.Get(1)); // ej klar!
  ApplicationContainer sinkApps = sink.Install (sinks);
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (21.0));
  
  // Set tracing functionalities.
  if(tracing){
    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll (ascii.CreateFileStream ("project_part3.tr"));
    p2p.EnablePcapAll ("project_part3", false);
  }

  // Run Simulation.
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  
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
