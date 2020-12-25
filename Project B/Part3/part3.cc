/* 
 * DT057A - Example Project B
 */
 
 /*
 Run code terminal commands
 ./waf --run scratch/<file name>
 Animation
 cd netanim/
 ./NetAnim
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
#include "ns3/packet-sink.h"
#include <random>
#include <fstream>
#include <iomanip>
#include <map>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("part3");

static void received_sensor_msg (Ptr<Socket> socket1, Ptr<Socket> socket2, Ptr<const Packet> p)
{
	std::cout << "::::: A packet from A received at the server in GW! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
	
	Ptr<UniformRandomVariable> rand=CreateObject<UniformRandomVariable>();
	
	if(rand->GetValue(0.0,1.0)<=0.1){
		std::cout << "::::: Transmitting from GW to K   "  << std::endl;
		socket1->Send (Create<Packet> (p->GetSize ()));
	}
	else{
		std::cout << "::::: Transmitting from GW to Controller   "  << std::endl;
		socket2->Send (Create<Packet> (p->GetSize ()));
	}
}

static void received_gateway_msg  (Ptr<Socket> socket, Ptr<const Packet> p)
{
	std::cout << "::::: A packet from GW received at the server in K! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
	socket->Send (Create<Packet> (p->GetSize ()));
}

static void received_k_msg  (Ptr<Socket> socket, Ptr<const Packet> p)
{
	std::cout << "::::: A packet received  at the server in Controller! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
	socket->Send (Create<Packet> (p->GetSize ()));
}

static void received_Con_msg  (Ptr<Socket> socket, Ptr<const Packet> p)
{
	std::cout << "::::: A packet from Controller received  at the server in GW! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
	socket->Send (Create<Packet> (p->GetSize ()));
}

int main (int argc, char *argv[]){
  
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
  
  NodeContainer A_GW = NodeContainer(nodes.Get(0),nodes.Get(4));
  NodeContainer B_GW = NodeContainer(nodes.Get(1),nodes.Get(4));
  NodeContainer C_GW = NodeContainer(nodes.Get(2),nodes.Get(4));
  NodeContainer D_GW = NodeContainer(nodes.Get(3),nodes.Get(4));
  NodeContainer GW_k = NodeContainer(nodes.Get(4),nodes.Get(5));
  NodeContainer k_Con = NodeContainer(nodes.Get(5),nodes.Get(6));
  NodeContainer GW_Con = NodeContainer(nodes.Get(4),nodes.Get(6));
  NodeContainer GW_Act = NodeContainer(nodes.Get(4),nodes.Get(7));
  
  // Setting global routing.
  NS_LOG_INFO ("Enabling Global Routing.");
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();  

  // Add Internet stack helper to make use of IP addresses.
  InternetStackHelper internet;
  internet.Install (nodes);

  // Create point to point channels
  //// Do i need to create p2p channels for each node? Luca uses the same channel to all devices.
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  // Sensors A, B, C, D
  NetDeviceContainer sensor_a;
  sensor_a = p2p.Install(A_GW);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  NetDeviceContainer sensor_b;
  sensor_b = p2p.Install(B_GW);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  NetDeviceContainer sensor_c;
  sensor_c = p2p.Install(C_GW);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  NetDeviceContainer sensor_d;
  sensor_d = p2p.Install(D_GW);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
 
  
   // k1
  NetDeviceContainer k1;
  k1 = p2p.Install(GW_k);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("50kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
 
  NetDeviceContainer k2;
  k2= p2p.Install(k_Con);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("50kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
 
  // Controller
  NetDeviceContainer controller;
  controller = p2p.Install(GW_Con);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  // Actuator
  NetDeviceContainer actuator;
  actuator = p2p.Install(GW_Act);
  p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  
  
  // Assign IP addresses to the channels and implement them correspondingly.
  NS_LOG_INFO ("Assign IP Addresses.");
  
  // IP v4 helper
  Ipv4AddressHelper address;
  
  // Sensor a
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_sensor_a = address.Assign(sensor_a);
  
  // Sensor b
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_sensor_b = address.Assign(sensor_b);
  
  // Sensor c
  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_sensor_c = address.Assign(sensor_c);
  
  // Sensor d
  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_sensor_d = address.Assign(sensor_d);     
  
  // k1
  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_k1 = address.Assign(k1);   
  
  //k2
  address.SetBase ("10.1.6.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_k2= address.Assign(k2);   
  
  // controller
  address.SetBase ("10.1.7.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_controller = address.Assign(controller);
  
  // actuator
  address.SetBase ("10.1.8.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_actuator = address.Assign(actuator);
  
  // Set servers on GW x 2, k x 1, controller x 1 and actuator x 1
  uint16_t port_number = 9;
  UdpServerHelper server_gateway(port_number);
  UdpServerHelper server_gateway_2(port_number+1);
  UdpServerHelper server_k(port_number);
  UdpServerHelper server_controller(port_number);
  UdpServerHelper server_actuator(port_number);

  
  //Setup servers
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  
  // Server GW1 as S1
  ApplicationContainer server_apps;
  server_apps.Add(server_gateway.Install(nodes.Get(4)));
  Ptr<UdpServer> S1 = server_gateway.GetServer();
  
  // Server k as S2
  server_apps.Add(server_k.Install(nodes.Get(5)));
  Ptr<UdpServer> S2 = server_k.GetServer();
  
  // Server controller as S3
  server_apps.Add(server_controller.Install(nodes.Get(6)));
  Ptr<UdpServer> S3 = server_controller.GetServer();
  
  // Server GW2 as S4
  server_apps.Add(server_gateway_2.Install(nodes.Get(4)));
  Ptr<UdpServer> S4 = server_gateway_2.GetServer();
  
  // Server actuator as S5   
  server_apps.Add(server_actuator.Install(nodes.Get(7)));
  Ptr<UdpServer> S5 = server_actuator.GetServer();
  
  ////Setup senders for callback action
  //// I dont know the function of tid!
  ////GW			->	k
  ////GW			->	Controller
  ////k				->	Controller
  ////Controller	->	GW
  
  //Connection Gateway -> k 
  Ptr<Socket> source1 = Socket::CreateSocket (nodes.Get (4), tid);
  InetSocketAddress remote1 = InetSocketAddress (interfaces_k1.GetAddress (1), port_number);
  source1->Connect (remote1);
  
  //Connection Gateway -> Controller
  Ptr<Socket> source1b = Socket::CreateSocket (nodes.Get (4), tid);
  InetSocketAddress remote1b= InetSocketAddress (interfaces_controller.GetAddress (1), port_number);
  source1b->Connect (remote1b);
  
  //Connection k -> Controller
  Ptr<Socket> source2 = Socket::CreateSocket (nodes.Get (5), tid);
  InetSocketAddress remote2 = InetSocketAddress (interfaces_k2.GetAddress (1), port_number);
  source2->Connect (remote2);
  
  //Connection Controller -> GW
  Ptr<Socket> source3 = Socket::CreateSocket (nodes.Get (6), tid);
  InetSocketAddress remote3 = InetSocketAddress (interfaces_controller.GetAddress (0), port_number+1);
  source3->Connect (remote3);
  
  //Connection GW -> Actuator
  Ptr<Socket> source4 = Socket::CreateSocket (nodes.Get (4), tid);
  InetSocketAddress remote4 = InetSocketAddress (interfaces_actuator.GetAddress (1), port_number+1); // Not sure, maybe we need one more server installed on (port +2)
  source4->Connect (remote4);

  // Create an application container for the servers
  //// GW -> k and GW -> Con
  S1->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&received_sensor_msg, source1, source1b ));
  
  //// k -> Con
  S2->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&received_gateway_msg, source2));
  
  //// Con -> GW
  S3->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&received_k_msg,  source3));
  
  //// GW2 -> Act
  S4->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&received_Con_msg, source4));
  
  //// Act
  S5->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&received_gateway_msg, source4)); // Vi kanske inte behöver det som server //////////////////////////
 
  server_apps.Start(Seconds(1.0));
  server_apps.Stop(Seconds(10.0));

  uint32_t max_packet_count = 10;
  Time inter_packet_interval = Seconds (0.05);
  uint32_t max_packet_size = 50;
  
  // sensor a -> GW
  UdpClientHelper client_sensor_a_to_gateway(interfaces_sensor_a.GetAddress(1), port_number); //(server address, server port)
  client_sensor_a_to_gateway.SetAttribute("MaxPackets", UintegerValue(max_packet_count)); 
  client_sensor_a_to_gateway.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client_sensor_a_to_gateway.SetAttribute("PacketSize", UintegerValue(max_packet_size));
  
  // sensor b -> GW
  UdpClientHelper client_sensor_b_to_gateway(interfaces_sensor_b.GetAddress(1), port_number); //(server address, server port)
  client_sensor_b_to_gateway.SetAttribute("MaxPackets", UintegerValue(max_packet_count)); 
  client_sensor_b_to_gateway.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client_sensor_b_to_gateway.SetAttribute("PacketSize", UintegerValue(max_packet_size));
  
  // sensor c -> GW
  UdpClientHelper client_sensor_c_to_gateway(interfaces_sensor_c.GetAddress(1), port_number); //(server address, server port)
  client_sensor_c_to_gateway.SetAttribute("MaxPackets", UintegerValue(max_packet_count)); 
  client_sensor_c_to_gateway.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client_sensor_c_to_gateway.SetAttribute("PacketSize", UintegerValue(max_packet_size));
  
  // sensor d -> GW
  UdpClientHelper client_sensor_d_to_gateway(interfaces_sensor_d.GetAddress(1), port_number); //(server address, server port)
  client_sensor_d_to_gateway.SetAttribute("MaxPackets", UintegerValue(max_packet_count)); 
  client_sensor_d_to_gateway.SetAttribute("Interval", TimeValue(inter_packet_interval));
  client_sensor_d_to_gateway.SetAttribute("PacketSize", UintegerValue(max_packet_size));
  
  //Install clients that will send UDP packets
  ApplicationContainer client_apps;
  client_apps.Add(client_sensor_a_to_gateway.Install(nodes.Get(0))); // a
  client_apps.Add(client_sensor_b_to_gateway.Install(nodes.Get(1))); // b
  client_apps.Add(client_sensor_c_to_gateway.Install(nodes.Get(2))); // c
  client_apps.Add(client_sensor_d_to_gateway.Install(nodes.Get(3))); // d
		
  client_apps.Start(Seconds(2.0));
  client_apps.Stop(Seconds(10.0));
  
  
  // Setup tracing files
  if(tracing){
    //ascii tracer
    AsciiTraceHelper ascii;
    p2p.EnableAsciiAll (ascii.CreateFileStream ("part3_ascii.tr"));
    //P2P_GW_K.EnableAsciiAll (ascii.CreateFileStream ("P2P_GW_K.tr"));
    //P2P_K_Con.EnableAsciiAll (ascii.CreateFileStream ("P2P_K_Con.tr"));
    //P2P_GW_Con.EnableAsciiAll (ascii.CreateFileStream ("P2P_GW_Con.tr"));
    // pcap
    p2p.EnablePcapAll ("part3_pcap");
    //P2P_GW_K.EnablePcapAll ("P2P_GW_K");
    //P2P_K_Con.EnablePcapAll ("P2P_K_Con");
    //P2P_GW_Con.EnablePcapAll ("P2P_GW_Con");
  }
  
  // Animate the network.
  AnimationInterface anim ("part3_animation.xml");
  anim.EnablePacketMetadata (true);
  anim.SetConstantPosition (nodes.Get(0), 10, 60);
  anim.SetConstantPosition (nodes.Get(1), 20, 60);
  anim.SetConstantPosition (nodes.Get(2), 30, 60);
  anim.SetConstantPosition (nodes.Get(3), 40, 60);
  
  anim.SetConstantPosition (nodes.Get(4), 30, 30);
  anim.SetConstantPosition (nodes.Get(5), 50, 10);
  anim.SetConstantPosition (nodes.Get(6), 70, 30);
  
  anim.SetConstantPosition (nodes.Get(7), 50, 60);

  
  // Run Simulation.
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  
  std::cout << "::::::::::::::::::::::::::::::::::::::::::" << std::endl;
  
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  return 0;
}
