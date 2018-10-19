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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
//#include "ns3/gtk-config-store.h"
#include "ns3/virt-5gc-module.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <thread>
#include <unistd.h>

using namespace ns3;

int additionalNodes = 6;
uint16_t numberOfNodes;
uint16_t totalNodeN;
bool firstCall = true;
Ptr<OutputStreamWrapper> rttStream;
Ptr<OutputStreamWrapper> cwndStream;
Ptr<OutputStreamWrapper> thputStream;

void
Graph()
{
	usleep(2500000);
	system("gnuplot result_graph.gp\n");
}

void
RttTracer (std::string node, Time oldval, Time newval)
{
	*rttStream->GetStream() << Simulator::Now ().GetSeconds() << ", " << node << ", " << newval.GetSeconds () << std::endl;
}

void
TraceRtt (std::string rtt_tr_file_name)
{
	std::string nodeN = std::to_string(totalNodeN + additionalNodes);
	std::string path = "/NodeList/" + nodeN + "/$ns3::TcpL4Protocol/SocketList/";
	
	AsciiTraceHelper ascii;
	rttStream = ascii.CreateFileStream (rtt_tr_file_name.c_str());
	std::string node, tmp_path;

	*rttStream->GetStream() << "# Time Node Rtt" << std::endl;
	for (uint16_t i = 0; i < numberOfNodes; i++) {
		node = std::to_string(i);
		tmp_path = path + node + "/RTT";
		Config::ConnectWithoutContext(tmp_path, MakeBoundCallback (&RttTracer, node));
	}
}

void
CwndTracer (std::string node, uint32_t oldval, uint32_t newval)
{
	*cwndStream->GetStream() << Simulator::Now().GetSeconds() << ", " << node << ", " << newval << std::endl;
}

void
TraceCwnd (std::string cwnd_tr_file_name)
{
	std::string nodeN = std::to_string(totalNodeN + additionalNodes);
	std::string path = "/NodeList/" + nodeN + "/$ns3::TcpL4Protocol/SocketList/";

	AsciiTraceHelper ascii;
	cwndStream = ascii.CreateFileStream (cwnd_tr_file_name.c_str());
	std::string node, tmp_path;

	*cwndStream->GetStream() << "# Time Node Cwnd" << std::endl;
	for (uint16_t i = 0; i < numberOfNodes; i++) {
		node = std::to_string(i);
		tmp_path = path + node + "/CongestionWindow";
		Config::ConnectWithoutContext(tmp_path, MakeBoundCallback (&CwndTracer, node));
	}
}

void
CalcThroughput (Ptr<Application> sinkApp, int lastTotalRx, int node)
{
	Ptr<PacketSink> sink = DynamicCast<PacketSink> (sinkApp);
	Time currTime = Simulator::Now();
	
	DoubleValue doubleValue;
	GlobalValue::GetValueByName ("scalingDelay", doubleValue);
	double scalingDelay = doubleValue.Get();
	double delay = 1;

	if (scalingDelay > 0) {
		Time t_delay = Time(std::to_string(scalingDelay) + "s");
		TimeValue timeValue;
		GlobalValue::GetValueByName ("scalingTime", timeValue);

		Time scalingTime = timeValue.Get();
		Time endTime = scalingTime + t_delay;

		
		if ((currTime >= scalingTime) && (currTime < endTime)) {
			Time addedDelay = (endTime - currTime);
			delay += addedDelay.GetSeconds();
		}
	}
	
	double throughput = ((sink->GetTotalRx() - lastTotalRx) / delay) * (double)(8/1e6);
	lastTotalRx = sink->GetTotalRx();

	if (firstCall) {
		*thputStream->GetStream() << "# Time Node Throughput(goodput)" << std::endl;
		firstCall = false;
	}

	*thputStream->GetStream() << currTime.GetSeconds() << ", " << node << ", " << throughput << std::endl;

	Simulator::Schedule(Seconds(1), &CalcThroughput, sinkApp, lastTotalRx, node);
}

void setFlowWeight(void)
{
  std::string filePath = "ovs_weight.txt";

  std::ofstream writeFile(filePath.data());
  if (writeFile.is_open()) {
    writeFile << "10\n";
    writeFile << "10\n";
    writeFile.close();
  }
}

int
main (int argc, char *argv[])
{

  //uint16_t numberOfNodes;
  double simTime = 25.0;
  double distance = 30.0;
  double interPacketInterval = 50;
  //std::string rttFile;

  // Command line arguments
  CommandLine cmd;
  cmd.AddValue("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodes);
  cmd.AddValue("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue("distance", "Distance between eNBs [m]", distance);
  cmd.AddValue("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
  
  //cmd.AddValue("rtt_tr_name", "Name of output trace file", rttFile);
  cmd.Parse(argc, argv);

  std::string format ("Virt5gc");
  std::string topoInput ("input/virt-5gc-toposample.input");
  std::string vmInput ("input/virt-5gc-vminput-heavy.input");

  setFlowWeight();
  
  Virt5gcHelper virt5gcHelp;
  virt5gcHelp.SetFileType (format);
  virt5gcHelp.SetTopoFile (topoInput);
  virt5gcHelp.SetInputFile (vmInput);

  Ptr<Virt5gc> virt5gcHelper = virt5gcHelp.GetVirt5gc();

  virt5gcHelper->Read();

  // set standard dev
  virt5gcHelper->DynamicLoadInit(10.0);
  // set scaling delay
  virt5gcHelper->SetAllocationDelay(0.02);
  /* SetMigrationRate (scale in, scale out)
   * scale in: 0 (even distribution), 1 (load maximum from first VM in a vmList)
   * scale out: 0, 0.5, 1.0
   */
  virt5gcHelper->SetMigrationRate(0.0, 0.5);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);
  Ptr<LteHelper> lteHelper = virt5gcHelper->GetLteHelper();
  //Ptr<PointToPointEpcHelper> epcHelper = virt5gcHelper->GetEpcHelper();
  Ptr<OvsPointToPointEpcHelper> epcHelper = virt5gcHelper->GetEpcHelper();
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
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;

  ueNodes = virt5gcHelper->GetUeNodes();
  enbNodes = virt5gcHelper->GetEnbNodes();

  numberOfNodes = ueNodes.GetN();
  //numberOfNodes = 3;
  totalNodeN = ueNodes.GetN() + enbNodes.GetN();

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = virt5gcHelper->GetEnbDevs();
  NetDeviceContainer ueLteDevs = virt5gcHelper->GetUeDevs();

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  
/*
  for (uint32_t i = 0; i < numberOfNodes; i++) {
  	  lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
  }
*/
  lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));
  lteHelper->Attach (ueLteDevs.Get(1), enbLteDevs.Get(0));
 

  uint16_t dlPort = 3000;
  int start = 1.0;
  std::list<ApplicationContainer> appList;

  for (uint32_t u = 0; u < numberOfNodes; u++)
  {
  	  Ptr<Node> ue = ueNodes.Get(u);

	  ++dlPort;

	  Ipv4Address ueipaddress = ueIpIface.GetAddress(u);
	  
	  BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (ueipaddress, dlPort));
	  source.SetAttribute ("MaxBytes", UintegerValue (0));
	  source.SetAttribute("SendSize", UintegerValue (1024));
	 
	  ApplicationContainer sourceApps = source.Install (remoteHost);
	  sourceApps.Start (Seconds (start + u*0.2));
	  sourceApps.Stop (Seconds (simTime));
	  
	  PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));	  	  
	 
	  ApplicationContainer sinkApps = sink.Install (ue);
	  sinkApps.Start (Seconds (start));
	  sinkApps.Stop (Seconds (simTime));
	  appList.push_back(sinkApps);
	  
	 
	  Ptr<EpcTft> tft = Create<EpcTft> ();
	  EpcTft::PacketFilter dlpf;
	  dlpf.localPortStart = dlPort;
	  dlpf.remotePortEnd = dlPort;
	  tft->Add(dlpf);

	  EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_OPERATOR);
	  lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get(u), bearer, tft);
   }

 
  Simulator::Stop(Seconds(simTime));
  //lteHelper->EnableTraces (); 

  std::string thput_tr_file_name = "Virt5gc-throughput.data";
  AsciiTraceHelper ascii;
  thputStream = ascii.CreateFileStream (thput_tr_file_name.c_str());
  std::list<ApplicationContainer>::iterator itor = appList.begin();

  //LogComponentEnable ("Config", LOG_LEVEL_ALL);

  Simulator::Schedule (Seconds (1 + numberOfNodes*0.2), &TraceRtt, "Virt5gc-rtt.data");
  Simulator::Schedule (Seconds (1 + numberOfNodes*0.2), &TraceCwnd, "Virt5gc-cwnd.data"); 
  for (uint32_t i = 0; i < numberOfNodes; i++, itor++) {
	  Simulator::Schedule (Seconds (1 + numberOfNodes*0.2), &CalcThroughput, (*itor).Get(0), 0, i);
  }

  std::thread t(&Graph);

  Simulator::Run();

  t.join();

  Simulator::Destroy();

  return 0;

}

