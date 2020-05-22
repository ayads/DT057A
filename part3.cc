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
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include <random>
#include <fstream>
#include <iomanip>
#include <map>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("project_Part3");

static void received_sensor_msg (Ptr<const Packet> p)
{
	std::cout << "::::: A packet correctly received_sensor_msg at the server! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
}

static void received_gateway_msg (Ptr<const Packet> p)
{
	std::cout << "::::: A packet correctly received_gateway_msg at the server! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
}

static void received_controller_msg (Ptr<const Packet> p)
{
	std::cout << "::::: A packet correctly received_controller_msg at the server! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
}

static void received_k_msg (Ptr<const Packet> p)
{
	std::cout << "::::: A packet correctly received_gateway_msg at the server! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
}

int main (int argc, char *argv[]){
  
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

  // Setting global routing.
  NS_LOG_INFO ("Enabling Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();  

  // Add Internet stack helper to make use of IP addresses.
  InternetStackHelper internet;
  internet.Install (nodes);

  // Create point to point channels
  PointToPointHelper p2p_a;
  p2p_a.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p_a.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  PointToPointHelper p2p_b;
  p2p_b.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p_b.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  PointToPointHelper p2p_c;
  p2p_c.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p_c.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  PointToPointHelper p2p_d;
  p2p_d.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p_d.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  PointToPointHelper p2p_e;
  p2p_e.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p_e.SetChannelAttribute ("Delay", StringValue ("20ms"));

  PointToPointHelper p2p_k1;
  p2p_k1.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p_k1.SetChannelAttribute ("Delay", StringValue ("20ms"));

  PointToPointHelper p2p_k2;
  p2p_k2.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p_k2.SetChannelAttribute ("Delay", StringValue ("20ms"));

  PointToPointHelper p2p_controller;
  p2p_controller.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p_controller.SetChannelAttribute ("Delay", StringValue ("20ms"));

  // Sensors
  NetDeviceContainer sensors;
  sensors.Add(p2p_a.Install(nodes.Get(0), nodes.Get(5)));
  sensors.Add(p2p_b.Install(nodes.Get(1), nodes.Get(5))); 
  sensors.Add(p2p_c.Install(nodes.Get(2), nodes.Get(5))); 
  sensors.Add(p2p_d.Install(nodes.Get(3), nodes.Get(5)));
  
  // Actuator
  NetDeviceContainer actuator;
  actuator.Add(p2p_e.Install(nodes.Get(4), nodes.Get(5)));
  
  // k
  NetDeviceContainer k;
  k.Add(p2p_k1.Install(nodes.Get(5), nodes.Get(6)));
  k.Add(p2p_k2.Install(nodes.Get(6), nodes.Get(7)));
  
  // Controller
  NetDeviceContainer controller;
  controller.Add(p2p_controller.Install(nodes.Get(5), nodes.Get(7)));
  
  // Assign IP addresses to the channels and implement them correspondingly.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_sensors = address.Assign(sensors);      // size 8
  Ipv4InterfaceContainer interfaces_actuator = address.Assign(actuator);    // size 2
  Ipv4InterfaceContainer interfaces_k = address.Assign(k);                  // size 4
  Ipv4InterfaceContainer interfaces_controller = address.Assign(controller);// size 2

  uint16_t port_number = 9;
  UdpServerHelper gateway_server_1(port_number);
  UdpServerHelper k_server(port_number);
  UdpServerHelper controller_server_1(port_number);
  UdpServerHelper controller_server_2(port_number + 1);
  UdpServerHelper gateway_server_2(port_number + 1);
  UdpServerHelper actuator_server(port_number);
  // Create an application container for the servers
  ApplicationContainer server_apps;
  server_apps.Add(gateway_server_1.Install(nodes.Get(5)));
  Ptr<UdpServer> S1 = gateway_server_1.GetServer();
  S1->TraceConnectWithoutContext ("Rx", MakeCallback (&received_sensor_msg));

  server_apps.Add(k_server.Install(nodes.Get(6)));
  Ptr<UdpServer> S2 = k_server.GetServer();
  S2->TraceConnectWithoutContext ("Rx", MakeCallback (&received_gateway_msg));

  server_apps.Add(controller_server_1.Install(nodes.Get(7)));
  Ptr<UdpServer> S3 = controller_server_1.GetServer();
  S3->TraceConnectWithoutContext ("Rx", MakeCallback (&received_k_msg));
  
  server_apps.Add(controller_server_2.Install(nodes.Get(7)));
  Ptr<UdpServer> S4 = controller_server_2.GetServer();
  S4->TraceConnectWithoutContext ("Rx", MakeCallback (&received_gateway_msg));

  server_apps.Add(gateway_server_2.Install(nodes.Get(5)));
  Ptr<UdpServer> S5 = gateway_server_2.GetServer();
  S5->TraceConnectWithoutContext ("Rx", MakeCallback (&received_controller_msg));

  server_apps.Add(actuator_server.Install(nodes.Get(4)));
  server_apps.Start(Seconds(1.0));
  server_apps.Stop(Seconds(10.0));

  uint32_t max_packet_count = 10;
  Time inter_packet_interval = Seconds (0.05);
  uint32_t max_packet_size = 1024;

  UdpClientHelper client_sensors(interfaces_sensors.GetAddress(1), port_number); //(server address, server port)
  client_sensors.SetAttribute("MaxPackets", UintegerValue(max_packet_count)); 
  client_sensors.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client_sensors.SetAttribute("PacketSize", UintegerValue(max_packet_size));

  UdpClientHelper client_actuator(interfaces_actuator.GetAddress(0), port_number);
  client_actuator.SetAttribute("MaxPackets", UintegerValue(max_packet_count));
  client_actuator.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client_actuator.SetAttribute("PacketSize", UintegerValue(max_packet_size));

  UdpClientHelper client_k(interfaces_k.GetAddress(1), port_number); 
  client_k.SetAttribute("MaxPackets", UintegerValue(max_packet_count));
  client_k.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client_k.SetAttribute("PacketSize", UintegerValue(max_packet_size));

  UdpClientHelper client_controller(interfaces_controller.GetAddress(1), port_number); // size 2
  client_controller.SetAttribute("MaxPackets", UintegerValue(max_packet_count));
  client_controller.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client_controller.SetAttribute("PacketSize", UintegerValue(max_packet_size));

  //Install clients that will send UDP packets
  ApplicationContainer client_apps;
  client_apps.Add(client_sensors.Install(nodes.Get(0))); // a
  client_apps.Add(client_sensors.Install(nodes.Get(1))); // b
  client_apps.Add(client_sensors.Install(nodes.Get(2))); // c
  client_apps.Add(client_sensors.Install(nodes.Get(3))); // d
  client_apps.Add(client_controller.Install(nodes.Get(5))); // controller
  client_apps.Add(client_k.Install(nodes.Get(6))); // k
  client_apps.Start(Seconds(2.0));
  client_apps.Stop(Seconds(10.0));
  
  // Setup tracing files
  if(tracing){
    //ascii tracer
    AsciiTraceHelper ascii;
    p2p_a.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p1.tr"));
    p2p_b.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p2.tr"));
    p2p_c.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p3.tr"));
    p2p_d.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p1.tr"));
    p2p_e.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p2.tr"));
    p2p_k1.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p3.tr"));
    p2p_k2.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p2.tr"));
    p2p_controller.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p3.tr"));
    // pcap
    p2p_a.EnablePcapAll ("project_part3_p2p1");
    p2p_b.EnablePcapAll ("project_part3_p2p2");
    p2p_c.EnablePcapAll ("project_part3_p2p3");
    p2p_d.EnablePcapAll ("project_part3_p2p1");
    p2p_e.EnablePcapAll ("project_part3_p2p2");
    p2p_k1.EnablePcapAll ("project_part3_p2p3");
    p2p_k2.EnablePcapAll ("project_part3_p2p1");
    p2p_controller.EnablePcapAll ("project_part3_p2p2");
  }

  // Animate the network.
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
  
  std::cout << ":::::::::::::::::::::::::::" << std::endl;
  std::cout << "interface size = "<< interfaces_sensors.GetN()<< std::endl;
  
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  return 0;
}