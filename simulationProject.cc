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
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"
#include <random>
#include <fstream>
#include <iomanip>

NS_LOG_COMPONENT_DEFINE ("projectPart3");


std::ofstream LCGfile, UVfile;
template<class T, int seed, int m, int a, int c>
void LCG(){
    std::linear_congruential_engine<T, m, a, c> E(seed);
    long double X = double(E())/E.max();
    for(int i=0; i<1000; i++){
      X = double(E())/E.max();
      //std::cout << std::setprecision(10); //<< X << "\n";
      LCGfile << std::setprecision(10)<< X << "\n";
    }
}


using namespace ns3;
int main (int argc, char *argv[]){
  //Linear Congruential Generator.
  LCGfile.open("outputLCG.txt");
  LCG<std::uint_fast32_t, 1, 500, 3, 0>();
  LCGfile.close();

  //Uniform Random Variable.
  UVfile.open("outputUV.txt");
  ns3::Ptr<UniformRandomVariable> uv = CreateObject<ns3::UniformRandomVariable> ();
  for(int i=0; i<1000; i++){
    UVfile << std::setprecision(10) << uv->GetValue() << "\n";
  } 
  UVfile.close();
  
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

  // Between sensors A B C D and gateway G
  p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  NetDeviceContainer net_dev_AG = p2p.Install (n_AG);
  NetDeviceContainer net_dev_BG = p2p.Install (n_BG);
  NetDeviceContainer net_dev_CG = p2p.Install (n_CG);
  NetDeviceContainer net_dev_DG = p2p.Install (n_DG);

  // Between actuator E and gateway G
  p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  NetDeviceContainer net_dev_EG = p2p.Install (n_EG);

  // Add Internet stack helper to make use of IP addresses.
  InternetStackHelper internet;
  internet.Install (n_container);
  
  // Assign IP addresses to the channels
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
  
  // Create the OnOff application to send UDP datagrams of size 210 bytes at a rate of 448 Kb/s from n0 to n4
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


  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}