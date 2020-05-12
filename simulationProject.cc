/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/udp-socket-factory.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Laboration_Part3");

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  /*Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);*/

  //NodeContainer nodes;
  //nodes.Create (2);
  uint32_t nSpokes = 7;

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("250kbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("20ms"));
  PointToPointStarHelper star (nSpokes, pointToPoint);

  //NetDeviceContainer devices;
  //devices = pointToPoint.Install (nodes);

  InternetStackHelper internet;
  star.InstallStack (internet);

  //InternetStackHelper stack;
  //stack.Install (nodes);

  star.AssignIpv4Addresses (Ipv4AddressHelper ("10.1.1.0", "255.255.255.0"));

  //Ipv4AddressHelper address;
  //address.SetBase ("10.1.1.0", "255.255.255.0");

  //Ipv4InterfaceContainer interfaces = address.Assign (devices);
  uint16_t port = 50000;

  Address hubLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", hubLocalAddress);
  ApplicationContainer hubApp = packetSinkHelper.Install (star.GetHub ());
  hubApp.Start (Seconds (1.0));
  hubApp.Stop (Seconds (10.0));

  OnOffHelper onOffHelper ("ns3::UdpSocketFactory", Address ());
  onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

  ApplicationContainer spokeApps;

  for (uint32_t i = 0; i < star.SpokeCount (); ++i)
  {
    AddressValue remoteAddress (InetSocketAddress (star.GetHubIpv4Address (i), port));
    onOffHelper.SetAttribute ("Remote", remoteAddress);
    spokeApps.Add (onOffHelper.Install (star.GetSpokeNode (i)));
  }
  spokeApps.Start (Seconds (1.0));
  spokeApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  //UdpEchoServerHelper echoServer (9);

  //ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  //serverApps.Start (Seconds (1.0));
  //serverApps.Stop (Seconds (10.0));

  /*UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));*/

  //ASCII Tracing
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("part3.tr"));

  //pcap tracing
  pointToPoint.EnablePcapAll ("part3");
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
