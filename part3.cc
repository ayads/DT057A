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
#include <iomanip>


using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("project_Part3");

int main (int argc, char *argv[]){

  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);

  bool tracing = false;
  
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
  UdpClientHelper client(interfaces.GetAddress(1), port_number);
  client.SetAttribute("MaxPackets", UintegerValue(max_packet_count));
  client.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client.SetAttribute("PacketSize", UintegerValue(max_packet_size));

  //Install Client
  ApplicationContainer clientApps;
  clientApps.Add(client.Install(nodes.Get(0)));
  clientApps.Add(client.Install(nodes.Get(1)));
  clientApps.Add(client.Install(nodes.Get(2)));
  clientApps.Add(client.Install(nodes.Get(3)));
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
