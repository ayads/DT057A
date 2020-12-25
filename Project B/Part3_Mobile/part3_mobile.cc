#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/netanim-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/packet-sink.h"
#include <random>
#include <fstream>
#include <iomanip>
#include <map>
#include <vector>

/*
	Run code terminal commands
	./waf --run scratch/<file name>
	Animation
	cd netanim/
	./NetAnim
*/
 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("part3_mobile_sensors");
int seed = 1;
long double LCG(int m, int a, int c){
    long double X = ((seed*a) + c)%m;
	seed ++;
	return X;
}
/*
static void received_sensor_msg (Ptr<Socket> socket1, Ptr<Socket> socket2, Ptr<const Packet> p)
{
	std::cout << "::::: A packet from A received at the server in GW! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
	
	Ptr<UniformRandomVariable> rand=CreateObject<UniformRandomVariable>();
	
	if(rand->GetValue(0.0,1.0)<=0.5){
		std::cout << "::::: Transmitting from GW to K   "  << std::endl;
		socket1->Send (Create<Packet> (p->GetSize ()));
	}
	else{
		std::cout << "::::: Transmitting from GW to Controller   "  << std::endl;
		socket2->Send (Create<Packet> (p->GetSize ()));
	}
}
*/

static void received_sensor_msg (Ptr<Socket> socket1, Ptr<Socket> socket2, Ptr<const Packet> p)
{
	std::cout << "::::: A packet from A received at the server in GW! Time:   " << Simulator::Now ().GetSeconds () << std::endl;
	long double rand = LCG(30, 5, 6);
	std::cout << ":::::NO. of generated packets using LCD:" << rand << std::endl;
	
	if(rand <=0.5){
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

int main (int arg, char *argv[]){
	uint32_t nWifi = 5;
	bool tracing  = false;
	NodeContainer nodes;
	nodes.Create (3);
	// Wifi nodes
	NodeContainer wifiStaNodes;
	wifiStaNodes.Create (nWifi);
	// Set GW as AP node
	NodeContainer wifiApNode = nodes.Get (0);
	
	NodeContainer GW_k = NodeContainer(nodes.Get(0),nodes.Get(1));
	NodeContainer k_Con = NodeContainer(nodes.Get(1),nodes.Get(2));
	NodeContainer GW_Con = NodeContainer(nodes.Get(0),nodes.Get(2));
	
	YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
	YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
	phy.SetChannel (channel.Create ());
	
	WifiHelper wifi;
	wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
	
	WifiMacHelper mac;
	Ssid ssid = Ssid ("ns-3-ssid");
	
	// Wifi sensors and actuator
	mac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid), "ActiveProbing", BooleanValue (false));
	NetDeviceContainer staDevices;
	staDevices = wifi.Install (phy, mac, wifiStaNodes);
	
	// ApWifi GW
	mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
	NetDeviceContainer apDevices;
	apDevices = wifi.Install (phy, mac, wifiApNode);
	
	MobilityHelper mobility;
	
	mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue (0.0),
                                  "MinY", DoubleValue (0.0),
                                  "DeltaX", DoubleValue (5.0),
                                  "DeltaY", DoubleValue (10.0),
                                  "GridWidth", UintegerValue (3),
                                  "LayoutType", StringValue ("RowFirst"));
	
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (wifiApNode);
	
	mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                              "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
	mobility.Install (wifiStaNodes);
	
	NS_LOG_INFO ("Enabling Global Routing.");
	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	
	InternetStackHelper internet;
	internet.Install (nodes);
	internet.Install (wifiStaNodes);
	//internet.Install (wifiApNode);
	
	// Create point to point channels
	PointToPointHelper p2p;
	p2p.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
	p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
	
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
	
	
	
	// Assign IP addresses to the channels and implement them correspondingly.
	NS_LOG_INFO ("Assign IP Addresses.");
	
	// IP v4 helper
	Ipv4AddressHelper address;
	
	address.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces_k1 = address.Assign(k1);
	
	address.SetBase ("10.1.2.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces_k2 = address.Assign(k2);
	
	address.SetBase ("10.1.3.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces_controller = address.Assign(controller);
	
	address.SetBase ("10.1.4.0", "255.255.255.0");
	Ipv4InterfaceContainer interfaces_wireless = address.Assign (staDevices);///////////////////////////////////Put all wireless devices in the same interface.
	Ipv4InterfaceContainer interfaces_GW = address.Assign (apDevices);
	
	//Server setup
	uint16_t port_number = 9;
	UdpServerHelper server_gateway(port_number);
	UdpServerHelper server_gateway_2(port_number+1);
	UdpServerHelper server_k(port_number);
	UdpServerHelper server_controller(port_number);
	UdpServerHelper server_actuator(port_number);
	
	TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
	
	// Server GW1 as S1
	ApplicationContainer server_apps;
	server_apps.Add(server_gateway.Install(nodes.Get(0)));
	Ptr<UdpServer> S1 = server_gateway.GetServer();
  
	// Server k as S2
	server_apps.Add(server_k.Install(nodes.Get(1)));
	Ptr<UdpServer> S2 = server_k.GetServer();
  
	// Server controller as S3
	server_apps.Add(server_controller.Install(nodes.Get(2)));
	Ptr<UdpServer> S3 = server_controller.GetServer();
  
	// Server GW2 as S4
	server_apps.Add(server_gateway_2.Install(nodes.Get(0)));
	Ptr<UdpServer> S4 = server_gateway_2.GetServer();
	
	// Server actuator as S5   
	server_apps.Add(server_actuator.Install(wifiStaNodes.Get(4)));
	Ptr<UdpServer> S5 = server_actuator.GetServer();
	
	
	//Connection Gateway -> k 
	Ptr<Socket> source1 = Socket::CreateSocket (nodes.Get (0), tid);
	InetSocketAddress remote1 = InetSocketAddress (interfaces_k1.GetAddress (1), port_number);
	source1->Connect (remote1);
  
	//Connection Gateway -> Controller
	Ptr<Socket> source1b = Socket::CreateSocket (nodes.Get (0), tid);
	InetSocketAddress remote1b= InetSocketAddress (interfaces_controller.GetAddress (1), port_number);
	source1b->Connect (remote1b);
  
	//Connection k -> Controller
	Ptr<Socket> source2 = Socket::CreateSocket (nodes.Get (1), tid);
	InetSocketAddress remote2 = InetSocketAddress (interfaces_k2.GetAddress (1), port_number);
	source2->Connect (remote2);
  
	//Connection Controller -> GW
	Ptr<Socket> source3 = Socket::CreateSocket (nodes.Get (2), tid);
	InetSocketAddress remote3 = InetSocketAddress (interfaces_controller.GetAddress (0), port_number+1);
	source3->Connect (remote3);
	
	//Connection GW -> Actuator ///////////////////////////////////should be specified for actuator
	Ptr<Socket> source4 = Socket::CreateSocket (nodes.Get (0), tid);
	InetSocketAddress remote4 = InetSocketAddress (interfaces_wireless.GetAddress(4), port_number+1);
	source4->Connect (remote4);
	
	// GW -> k and GW -> Con
	S1->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&received_sensor_msg, source1, source1b ));
	
	// k -> Con
	S2->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&received_gateway_msg, source2));
	
	// Con -> GW
	S3->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&received_k_msg,  source3));
	
	// GW2 -> Act
	S4->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&received_Con_msg, source4));
	
	// Act
	S5->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&received_gateway_msg, source4));
	
	server_apps.Start(Seconds(1.0));
	server_apps.Stop(Seconds(10.0));
	
	uint32_t max_packet_count = 10;
	Time inter_packet_interval = Seconds (0.05);
	uint32_t max_packet_size = 50;
  
	// sensor a -> GW
	UdpClientHelper client_sensor_a_to_gateway(interfaces_GW.GetAddress(0), port_number); //(server address, server port)
	client_sensor_a_to_gateway.SetAttribute("MaxPackets", UintegerValue(max_packet_count)); 
	client_sensor_a_to_gateway.SetAttribute("Interval", TimeValue(inter_packet_interval));
	client_sensor_a_to_gateway.SetAttribute("PacketSize", UintegerValue(max_packet_size));
  
	// sensor b -> GW
	UdpClientHelper client_sensor_b_to_gateway(interfaces_GW.GetAddress(0), port_number); //(server address, server port)
	client_sensor_b_to_gateway.SetAttribute("MaxPackets", UintegerValue(max_packet_count)); 
	client_sensor_b_to_gateway.SetAttribute("Interval", TimeValue(inter_packet_interval));
	client_sensor_b_to_gateway.SetAttribute("PacketSize", UintegerValue(max_packet_size));
	
	// sensor c -> GW
	UdpClientHelper client_sensor_c_to_gateway(interfaces_GW.GetAddress(0), port_number); //(server address, server port)
	client_sensor_c_to_gateway.SetAttribute("MaxPackets", UintegerValue(max_packet_count)); 
	client_sensor_c_to_gateway.SetAttribute("Interval", TimeValue(inter_packet_interval));
	client_sensor_c_to_gateway.SetAttribute("PacketSize", UintegerValue(max_packet_size));
	
	// sensor d -> GW
	UdpClientHelper client_sensor_d_to_gateway(interfaces_GW.GetAddress(0), port_number); //(server address, server port)
	client_sensor_d_to_gateway.SetAttribute("MaxPackets", UintegerValue(max_packet_count)); 
	client_sensor_d_to_gateway.SetAttribute("Interval", TimeValue(inter_packet_interval));
	client_sensor_d_to_gateway.SetAttribute("PacketSize", UintegerValue(max_packet_size));
	
	//Install clients that will send UDP packets
	ApplicationContainer client_apps;
	client_apps.Add(client_sensor_a_to_gateway.Install(wifiStaNodes.Get(0))); // a
	client_apps.Add(client_sensor_b_to_gateway.Install(wifiStaNodes.Get(1))); // b
	client_apps.Add(client_sensor_c_to_gateway.Install(wifiStaNodes.Get(2))); // c
	client_apps.Add(client_sensor_d_to_gateway.Install(wifiStaNodes.Get(3))); // d
	
	client_apps.Start(Seconds(2.0));
	client_apps.Stop(Seconds(10.0));
	
	if(tracing){
		//ascii tracer
		AsciiTraceHelper ascii;
		p2p.EnableAsciiAll (ascii.CreateFileStream ("part3_mobile_ascii.tr"));
		//P2P_GW_K.EnableAsciiAll (ascii.CreateFileStream ("P2P_GW_K.tr"));
		//P2P_K_Con.EnableAsciiAll (ascii.CreateFileStream ("P2P_K_Con.tr"));
		//P2P_GW_Con.EnableAsciiAll (ascii.CreateFileStream ("P2P_GW_Con.tr"));
		// pcap
		p2p.EnablePcapAll ("part3_mobile_pcap");
		//phy.EnablePcap ("wifi");
		//P2P_GW_K.EnablePcapAll ("P2P_GW_K");
		//P2P_K_Con.EnablePcapAll ("P2P_K_Con");
		//P2P_GW_Con.EnablePcapAll ("P2P_GW_Con");
	}
	
	AnimationInterface anim ("part3_mobile_animation.xml");
	anim.EnablePacketMetadata (true);
	anim.SetConstantPosition (wifiStaNodes.Get(0), 10, 60);
	anim.SetConstantPosition (wifiStaNodes.Get(1), 20, 60);
	anim.SetConstantPosition (wifiStaNodes.Get(2), 30, 60);
	anim.SetConstantPosition (wifiStaNodes.Get(3), 40, 60);
	
	anim.SetConstantPosition (nodes.Get(0), 30, 30);
	anim.SetConstantPosition (nodes.Get(1), 50, 10);
	anim.SetConstantPosition (nodes.Get(2), 70, 30);
	
	anim.SetConstantPosition (wifiStaNodes.Get(4), 50, 60);
	
	// Run Simulation.
	NS_LOG_INFO ("Run Simulation.");
	Simulator::Stop(Seconds(10.0));
	Simulator::Run ();
	std::cout << "::::::::::::::::::::::::::::::::::::::::::" << std::endl;
	Simulator::Destroy ();
	NS_LOG_INFO ("Done.");
	return 0;
}