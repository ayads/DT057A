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

/*
void ReceivePacket (Ptr<Socket> socket)
{
  NS_LOG_INFO ("Received one packet!");
  Ptr<Packet> packet = socket->Recv ();
  SocketIpTosTag tosTag;
  if (packet->RemovePacketTag (tosTag))
  {
    NS_LOG_INFO (" TOS = " << (uint32_t)tosTag.GetTos ());
  }
  SocketIpTtlTag ttlTag;
  if (packet->RemovePacketTag (ttlTag))
  {
    NS_LOG_INFO (" TTL = " << (uint32_t)ttlTag.GetTtl ());
  }
}

static void SendPacket (Ptr<Socket> socket, uint32_t pktSize, uint32_t pktCount, Time pktInterval )
{
  if (pktCount > 0){
    socket->Send (Create<Packet> (pktSize));
    Simulator::Schedule (pktInterval, &SendPacket, socket, pktSize, pktCount - 1, pktInterval);
  }
  else {
    socket->Close ();
  }
}
*/

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
  PointToPointHelper p2p1;
  p2p1.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p1.SetChannelAttribute ("Delay", StringValue ("20ms"));
  PointToPointHelper p2p2;
  p2p2.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p2.SetChannelAttribute ("Delay", StringValue ("20ms"));
  PointToPointHelper p2p3;
  p2p3.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  p2p3.SetChannelAttribute ("Delay", StringValue ("20ms"));

  // Sensors
  NetDeviceContainer sensors;
  sensors.Add(p2p1.Install(nodes.Get(0), nodes.Get(5)));
  sensors.Add(p2p1.Install(nodes.Get(1), nodes.Get(5))); 
  sensors.Add(p2p1.Install(nodes.Get(2), nodes.Get(5))); 
  sensors.Add(p2p1.Install(nodes.Get(3), nodes.Get(5))); 
  
  // Actuator
  NetDeviceContainer actuator;
  actuator.Add(p2p1.Install(nodes.Get(4), nodes.Get(5)));
  
  // k
  NetDeviceContainer k;
  k.Add(p2p2.Install(nodes.Get(5), nodes.Get(6)));
  k.Add(p2p2.Install(nodes.Get(6), nodes.Get(7)));
  
  // Controller
  NetDeviceContainer controller;
  controller.Add(p2p3.Install(nodes.Get(5), nodes.Get(7)));
  
  // Assign IP addresses to the channels and implement them correspondingly.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_sensors = address.Assign(sensors);      // size 8
  Ipv4InterfaceContainer interfaces_actuator = address.Assign(actuator);    // size 2
  Ipv4InterfaceContainer interfaces_k = address.Assign(k);                  // size 4
  Ipv4InterfaceContainer interfaces_controller = address.Assign(controller);// size 2

  uint16_t port_number = 9;
  UdpServerHelper server(port_number);

  // Create an application container for the servers
  ApplicationContainer serverApps;
  //serverApps.Add(server.Install(nodes.Get(5)));// Gateway
  serverApps.Add(server.Install(nodes.Get(7)));//Controller
  serverApps.Add(server.Install(nodes.Get(4)));// Actuator
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(10.0));

  uint32_t max_packet_count = 10;
  Time inter_packet_interval = Seconds (0.05);
  uint32_t max_packet_size = 1024;

  UdpClientHelper client_sensors(interfaces_sensors.GetAddress(1), port_number);
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

  //Install Client
  ApplicationContainer clientApps;
  clientApps.Add(client_sensors.Install(nodes.Get(0)));
  clientApps.Add(client_sensors.Install(nodes.Get(1)));
  clientApps.Add(client_sensors.Install(nodes.Get(2)));
  clientApps.Add(client_sensors.Install(nodes.Get(3)));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));


  /*
  //Socket options for IPv4, currently TOS, TTL, RECVTOS, and RECVTTL
  uint32_t ipTos = 0; 
  bool ipRecvTos = true; 
  uint32_t ipTtl = 0; 
  bool ipRecvTtl = true;
  
  NS_LOG_INFO ("Create sockets.");
  //Receiver socket on n1
  TypeId t_id = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (nodes.Get (1), t_id);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), port_number);
  recvSink->SetIpRecvTos (ipRecvTos);
  recvSink->SetIpRecvTtl (ipRecvTtl);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  //Sender socket on n0
  Ptr<Socket> source_a = Socket::CreateSocket (nodes.Get (0), t_id);
  InetSocketAddress remote_a = InetSocketAddress (interfaces_devices.GetAddress (0), port_number);

  Ptr<Socket> source_b = Socket::CreateSocket (nodes.Get (1), t_id);
  InetSocketAddress remote_b = InetSocketAddress (interfaces_devices.GetAddress (1), port_number);

  Ptr<Socket> source_c = Socket::CreateSocket (nodes.Get (2), t_id);
  InetSocketAddress remote_c = InetSocketAddress (interfaces_devices.GetAddress (2), port_number);

  Ptr<Socket> source_d = Socket::CreateSocket (nodes.Get (3), t_id);
  InetSocketAddress remote_d = InetSocketAddress (interfaces_devices.GetAddress (3), port_number);
  
  //Set socket options, it is also possible to set the options after the socket has been created/connected.
  if (ipTos > 0) 
  {
    source_a->SetIpTos (ipTos);
    source_b->SetIpTos (ipTos);
    source_c->SetIpTos (ipTos);
    source_d->SetIpTos (ipTos);
  }
  
  if (ipTtl > 0)
  {
    source_a->SetIpTtl (ipTtl);
    source_b->SetIpTtl (ipTtl);
    source_c->SetIpTtl (ipTtl);
    source_d->SetIpTtl (ipTtl);
  }
  
  source_a->Connect (remote_a);
  source_b->Connect (remote_b);
  source_c->Connect (remote_c);
  source_d->Connect (remote_d);
  
  //Access IP addresses
  ns3::Ptr<ns3::Ipv4> ipv4_a = nodes.Get(0)->GetObject<ns3::Ipv4> ();
  ns3::Ipv4InterfaceAddress iaddr_a = ipv4_a->GetAddress (1,0); 
  ns3::Ipv4Address ipAddr_a = iaddr_a.GetLocal ();

  ns3::Ptr<ns3::Ipv4> ipv4_b = nodes.Get(1)->GetObject<ns3::Ipv4> ();
  ns3::Ipv4InterfaceAddress iaddr_b = ipv4_b->GetAddress (1,0); 
  ns3::Ipv4Address ipAddr_b = iaddr_b.GetLocal (); 

  ns3::Ptr<ns3::Ipv4> ipv4_c = nodes.Get(2)->GetObject<ns3::Ipv4> ();
  ns3::Ipv4InterfaceAddress iaddr_c = ipv4_c->GetAddress (1,0); 
  ns3::Ipv4Address ipAddr_c = iaddr_c.GetLocal (); 

  ns3::Ptr<ns3::Ipv4> ipv4_d = nodes.Get(3)->GetObject<ns3::Ipv4> ();
  ns3::Ipv4InterfaceAddress iaddr_d = ipv4_d->GetAddress (1,0); 
  ns3::Ipv4Address ipAddr_d = iaddr_d.GetLocal ();
  */

  // Setup tracing files
  if(tracing){
    AsciiTraceHelper ascii;
    p2p1.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p1.tr"));
    p2p2.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p2.tr"));
    p2p3.EnableAsciiAll (ascii.CreateFileStream ("project_part3_p2p3.tr"));
    p2p1.EnablePcapAll ("project_part3_p2p1");
    p2p2.EnablePcapAll ("project_part3_p2p2");
    p2p3.EnablePcapAll ("project_part3_p2p3");
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
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  return 0;
}