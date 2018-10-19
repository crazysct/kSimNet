/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
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
 *
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */

#include "ns3/lte-helper.h" //jhlim
#include "ns3/epc-helper.h" //jhlim
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"

#include "ns3/lte-enb-net-device.h"
#include "ns3/lte-ue-net-device.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
//#include "ns3/ovs-point-to-point-epc-helper.h"
//#include "ns3/point-to-point-epc-helper.h"
#include "ns3/ovs-point-to-point-epc-helper.h"
#include <iostream>
#include "ns3/mmwave-helper.h"
#include "ns3/mmwave-point-to-point-epc-helper.h"
#include <ns3/buildings-helper.h>
#include <ns3/buildings-module.h>
#include "ns3/net-device.h" //jhlim
//#include "ns3/original_net-device-container.h"

using namespace std;

//#include "ns3/gtk-config-store.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It also  starts yet another flow between each UE pair.
 */

NS_LOG_COMPONENT_DEFINE ("EpcFirstExample");

int
main (int argc, char *argv[])
{

  uint16_t numberOfNodes = 1;
  double simTime = 5;
  double distance = 60.0;
  double interPacketInterval = 1;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodes);
  cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue("distance", "Distance between eNBs [m]", distance);
  cmd.AddValue("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
  cmd.Parse(argc, argv);

// Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<MmWaveHelper> mmwaveHelper = CreateObject<MmWaveHelper> (); //sjakang

//  Ptr<OvsPointToPointEpcHelper>  epcHelper = CreateObject<OvsPointToPointEpcHelper> ();
 // lteHelper->SetEpcHelper (epcHelper);
	Ptr<MmWavePointToPointEpcHelper> epcHelper = CreateObject<MmWavePointToPointEpcHelper> ();
	mmwaveHelper->SetEpcHelper (epcHelper);
	mmwaveHelper->SetHarqEnabled (true);
	// mmwaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::BuildingsObstaclePropagationLossModel"));
	 mmwaveHelper->SetAttribute ("PathlossModel", StringValue ("ns3::BuildingsObstaclePropagationLossModel"));
	mmwaveHelper->Initialize();

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();


   // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // NodeContainer ueNodes;
  //NodeContainer enbNodes;

  //enbNodes.Create(numberOfNodes);
 ///ueNodes.Create(numberOfNodes);
 // ueNodes.Create(2);
    NodeContainer ueNodes;
	NodeContainer mmWaveEnbNodes_28G;
	NodeContainer mmWaveEnbNodes_73G;
	NodeContainer lteEnbNodes;
	NodeContainer allEnbNodes;
	mmWaveEnbNodes_28G.Create(1);
	mmWaveEnbNodes_73G.Create(1);
	lteEnbNodes.Create(1);
	ueNodes.Create(1);
	allEnbNodes.Add(lteEnbNodes);
    allEnbNodes.Add(mmWaveEnbNodes_73G);
	allEnbNodes.Add(mmWaveEnbNodes_28G);


  // Install Mobility Model
  Ptr<ListPositionAllocator> UE_positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      UE_positionAlloc->Add (Vector(distance * i, 0, 0));
    }
  MobilityHelper uemobility;
  uemobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  uemobility.SetPositionAllocator(UE_positionAlloc);
  uemobility.Install(ueNodes);
//  mobility.Install(ueNodes);


  	Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
	enbPositionAlloc->Add (Vector ((double)30.0, -5, 12)); //lte
	enbPositionAlloc->Add (Vector(0,0,10)); //mmWave 1
	enbPositionAlloc->Add (Vector(100,0,10)); //mmWave 2
	
	MobilityHelper enbmobility;
	enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	enbmobility.SetPositionAllocator(enbPositionAlloc);
	enbmobility.Install (allEnbNodes);


  // Install LTE Devices to the nodes
  //NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
 // NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);
	NetDeviceContainer lteEnbDevs = mmwaveHelper->InstallLteEnbDevice (lteEnbNodes);
	NetDeviceContainer mmWaveEnbDevs_73GHZ = mmwaveHelper->InstallEnbDevice_73GHZ(mmWaveEnbNodes_73G);
	NetDeviceContainer mmWaveEnbDevs_28GHZ = mmwaveHelper->InstallEnbDevice_28GHZ(mmWaveEnbNodes_28G);
	
	NetDeviceContainer mcUeDevs;
	mcUeDevs = mmwaveHelper->InstallMcUeDevice (ueNodes);

	 BuildingsHelper::Install (allEnbNodes);
      BuildingsHelper::Install (ueNodes);
  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (mcUeDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Attach one UE per eNodeB

  /*for (uint16_t i = 0; i < 2; i++)
      {
        lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(0));
        // side effect: the default EPS bearer will be activated
      }*/
	mmwaveHelper->AddX2Interface(lteEnbNodes, mmWaveEnbNodes_73G, mmWaveEnbNodes_28G);
	mmwaveHelper->AttachToClosestEnb (mcUeDevs, mmWaveEnbDevs_28GHZ, mmWaveEnbDevs_73GHZ, lteEnbDevs);

  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
      ++ulPort;
      ++otherPort;
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
      serverApps.Add (packetSinkHelper.Install (ueNodes.Get(u)));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      cout<<"ue.GetAddress("<<u<<"): "<<ueIpIface.GetAddress(u)<<endl;
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));

      UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
      client.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
      client.SetAttribute ("MaxPackets", UintegerValue(1000000));

      clientApps.Add (dlClient.Install (remoteHost));
      clientApps.Add (ulClient.Install (ueNodes.Get(u)));
      if (u+1 < ueNodes.GetN ())
        {
          clientApps.Add (client.Install (ueNodes.Get(u+1)));
        }
      else
        {
          clientApps.Add (client.Install (ueNodes.Get(0)));
        }
  }
  serverApps.Start (Seconds (0.01));
  clientApps.Start (Seconds (0.01));
  
  mmwaveHelper->EnableTraces ();
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-epc-first");

  cout<<"pgw getNdevices:"<<pgw->GetNDevices()<<endl;

  Simulator::Stop(Seconds(simTime));
  Simulator::Run();

  /*GtkConfigStore config;
  config.ConfigureAttributes();*/

  Simulator::Destroy();


  //ApplicationContainer sinkApps = serverApps;

//  Ptr<PacketSink> sink0 = DynamicCast<PacketSink> (sinkApps.Get (0)); 
 /// Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (1));
 // /Ptr<PacketSink> sink2 = DynamicCast<PacketSink> (sinkApps.Get (2));
 // Ptr<PacketSink> sink3 = DynamicCast<PacketSink> (sinkApps.Get (3));



  return 0;

}

