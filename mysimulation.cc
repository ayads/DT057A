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
#include "ns3/csma-module.h"
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

  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);

  bool tracing = true;
  
  // Allow the user to override any of the defaults at run-time.
  CommandLine cmd;
  cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  nodes.Create (8);

  // Add Internet stack helper to make use of IP addresses.
  InternetStackHelper stack;
  stack.Install (nodes);

  PointToPointHelper p2p1;
  p2p1.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p1.SetChannelAttribute ("Delay", StringValue ("20ms"));

  PointToPointHelper p2p2;
  p2p2.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p2.SetChannelAttribute ("Delay", StringValue ("20ms"));

  PointToPointHelper p2p3;
  p2p3.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p3.SetChannelAttribute ("Delay", StringValue ("20ms"));

  // Assign IP addresses to the channels and implement them correspondingly.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  NetDeviceContainer devices;
  // p2p1
  devices.Add(p2p1.Install(nodes.Get(0), nodes.Get(5)));
  devices.Add(p2p1.Install(nodes.Get(1), nodes.Get(5))); 
  devices.Add(p2p1.Install(nodes.Get(2), nodes.Get(5))); 
  devices.Add(p2p1.Install(nodes.Get(3), nodes.Get(5))); 
  devices.Add(p2p1.Install(nodes.Get(4), nodes.Get(5))); 
  // p2p2
  devices.Add(p2p2.Install(nodes.Get(5), nodes.Get(6)));
  devices.Add(p2p2.Install(nodes.Get(5), nodes.Get(7)));
  // p2p3
  devices.Add(p2p3.Install(nodes.Get(6), nodes.Get(7)));
  Ipv4InterfaceContainer interfaces = address.Assign(devices);


  uint16_t port_number = 9;
  UdpServerHelper server(port_number);

  // Create an application container for the servers
  ApplicationContainer serverApps;
  serverApps.Add(server.Install(nodes.Get(7)));//Controller
  serverApps.Add(server.Install(nodes.Get(4)));// Actuator
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(10.0)); 

  uint32_t max_packet_count = 320;
  Time inter_packet_interval = Seconds (0.05);
  uint32_t max_packet_size = 1024;
  UdpClientHelper client1(interfaces.GetAddress(13), port_number);
  client1.SetAttribute("MaxPackets", UintegerValue(max_packet_count));
  client1.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client1.SetAttribute("PacketSize", UintegerValue(max_packet_size));

  UdpClientHelper client2(interfaces.GetAddress(15), port_number);
  client2.SetAttribute("MaxPackets", UintegerValue(max_packet_count));
  client2.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client2.SetAttribute("PacketSize", UintegerValue(max_packet_size));
/*
  UdpEchoClientHelper echoClient(interfaces.GetAddress(15), port_number);
  echoClient.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));
*/
  //Install Client
  ApplicationContainer clientApps;
  clientApps.Add(server.Install(nodes.Get(0)));
  clientApps.Add(server.Install(nodes.Get(1)));
  clientApps.Add(server.Install(nodes.Get(2)));
  clientApps.Add(server.Install(nodes.Get(3)));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));

  // Setting global routing.
  NS_LOG_INFO ("Enabling Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();  

  if(tracing){
    AsciiTraceHelper ascii;
    p2p1.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p1.tr"));
    p2p2.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p2.tr"));
    p2p3.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p3.tr"));
    p2p1.EnablePcapAll ("project_part3_p2p1");
    p2p2.EnablePcapAll ("project_part3_p2p2");
    p2p3.EnablePcapAll ("project_part3_p2p3");
  }

  
  AnimationInterface anim ("project_part3_anim.xml");
  anim.EnablePacketMetadata (true);
  anim.EnableIpv4RouteTracking ("project_part3_route_anim.xml", Seconds(0), Seconds(5), Seconds(0.25));
  anim.SetConstantPosition (nodes.Get(0), 10, 60);
  anim.SetConstantPosition (nodes.Get(1), 20, 60);
  anim.SetConstantPosition (nodes.Get(2), 30, 60);
  anim.SetConstantPosition (nodes.Get(3), 40, 60);
  anim.SetConstantPosition (nodes.Get(4), 50, 60);
  anim.SetConstantPosition (nodes.Get(5), 30, 30);
  anim.SetConstantPosition (nodes.Get(6), 50, 10);
  anim.SetConstantPosition (nodes.Get(7), 70, 30);

  // Run Simulation.
  NS_LOG_INFO ("Run Simulation.");
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
