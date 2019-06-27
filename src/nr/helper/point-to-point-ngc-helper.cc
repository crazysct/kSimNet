/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab
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
 * Author: Jaume Nin <jnin@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 *         Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Support for real N2AP link
 */

#include <ns3/point-to-point-ngc-helper.h>
#include <ns3/log.h>
#include <ns3/inet-socket-address.h>
#include <ns3/mac48-address.h>
#include <ns3/eps-bearer.h>
#include <ns3/ipv4-address.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/packet-socket-helper.h>
#include <ns3/packet-socket-address.h>
#include <ns3/ngc-enb-application.h>
#include <ns3/ngc-smf-upf-application.h>

#include <ns3/nr-enb-rrc.h>
#include <ns3/ngc-x2.h>
#include <ns3/ngc-n2ap.h>
#include <ns3/nr-enb-net-device.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/ngc-amf-application.h>
#include <ns3/ngc-ue-nas.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PointToPointNgcHelper");

NS_OBJECT_ENSURE_REGISTERED (PointToPointNgcHelper);


PointToPointNgcHelper::PointToPointNgcHelper () 
  : m_gtpuUdpPort (2152),  // fixed by the standard
    m_n2apUdpPort (36412)
{
  NS_LOG_FUNCTION (this);

  // since we use point-to-point links for all N2-U and N2-AP links, 
  // we use a /30 subnet which can hold exactly two addresses 
  // (remember that net broadcast and null address are not valid)
  m_n2uIpv4AddressHelper.SetBase ("10.0.0.0", "255.255.255.252");
  m_n2apIpv4AddressHelper.SetBase ("11.0.0.0", "255.255.255.252");
  m_x2Ipv4AddressHelper.SetBase ("12.0.0.0", "255.255.255.252");

  // we use a /8 net for all UEs
  m_ueAddressHelper.SetBase ("7.0.0.0", "255.0.0.0");
  
  // create SmfUpfNode
  m_smfUpf = CreateObject<Node> ();
  InternetStackHelper internet;
  internet.Install (m_smfUpf);

  // create AmfNode
  m_amfNode = CreateObject<Node> ();
  internet.Install (m_amfNode);
  
  // create N2-U socket for SmfUpfNode
  Ptr<Socket> smfUpfN2uSocket = Socket::CreateSocket (m_smfUpf, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  int retval = smfUpfN2uSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), m_gtpuUdpPort));
  NS_ASSERT (retval == 0);

  // create N2-AP socket for AmfNode
  Ptr<Socket> amfN2apSocket = Socket::CreateSocket (m_amfNode, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = amfN2apSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), m_n2apUdpPort)); // it listens on any IP, port m_n2apUdpPort
  NS_ASSERT (retval == 0);

  // create TUN device implementing tunneling of user data over GTP-U/UDP/IP 
  m_tunDevice = CreateObject<VirtualNetDevice> ();
  // allow jumbo packets
  m_tunDevice->SetAttribute ("Mtu", UintegerValue (30000));

  // yes we need this
  m_tunDevice->SetAddress (Mac48Address::Allocate ()); 

  m_smfUpf->AddDevice (m_tunDevice);
  NetDeviceContainer tunDeviceContainer;
  tunDeviceContainer.Add (m_tunDevice);
  
  // the TUN device is on the same subnet as the UEs, so when a packet
  // addressed to an UE arrives at the intenet to the WAN interface of
  // the UPF it will be forwarded to the TUN device. 
  Ipv4InterfaceContainer tunDeviceIpv4IfContainer = m_ueAddressHelper.Assign (tunDeviceContainer);  

  // create NgcSmfUpfApplication
  m_smfUpfApp = CreateObject<NgcSmfUpfApplication> (m_tunDevice, smfUpfN2uSocket);
  m_smfUpf->AddApplication (m_smfUpfApp);
  
  // connect SmfUpfApplication and virtual net device for tunneling
  m_tunDevice->SetSendCallback (MakeCallback (&NgcSmfUpfApplication::RecvFromTunDevice, m_smfUpfApp));

  // create N2apAmf object and aggregate it with the m_amfNode
  Ptr<NgcN2apAmf> n2apAmf = CreateObject<NgcN2apAmf> (amfN2apSocket, 1); // for now, only one amf!
  m_amfNode->AggregateObject(n2apAmf);

  // create NgcAmfApplication and connect with SMF via N11 interface
  m_amfApp = CreateObject<NgcAmfApplication> ();
  m_amfNode->AddApplication (m_amfApp);
  m_amfApp->SetN11SapSmf (m_smfUpfApp->GetN11SapSmf ());
  m_smfUpfApp->SetN11SapAmf (m_amfApp->GetN11SapAmf ());
  // connect m_amfApp to the n2apAmf
  m_amfApp->SetN2apSapAmfProvider(n2apAmf->GetNgcN2apSapAmfProvider());
  n2apAmf->SetNgcN2apSapAmfUser(m_amfApp->GetN2apSapAmf());
}

PointToPointNgcHelper::~PointToPointNgcHelper ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
PointToPointNgcHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PointToPointNgcHelper")
    .SetParent<NgcHelper> ()
    .SetGroupName("Nr")
    .AddConstructor<PointToPointNgcHelper> ()
    .AddAttribute ("N2uLinkDataRate", 
                   "The data rate to be used for the next N2-U link to be created",
                   DataRateValue (DataRate ("10Gb/s")),
                   MakeDataRateAccessor (&PointToPointNgcHelper::m_n2uLinkDataRate),
                   MakeDataRateChecker ())
    .AddAttribute ("N2uLinkDelay", 
                   "The delay to be used for the next N2-U link to be created",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&PointToPointNgcHelper::m_n2uLinkDelay),
                   MakeTimeChecker ())
    .AddAttribute ("N2uLinkMtu", 
                   "The MTU of the next N2-U link to be created. Note that, because of the additional GTP/UDP/IP tunneling overhead, you need a MTU larger than the end-to-end MTU that you want to support.",
                   UintegerValue (2000),
                   MakeUintegerAccessor (&PointToPointNgcHelper::m_n2uLinkMtu),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("N2apLinkDataRate", 
                   "The data rate to be used for the N2-AP link to be created",
                   DataRateValue (DataRate ("10Mb/s")),
                   MakeDataRateAccessor (&PointToPointNgcHelper::m_n2apLinkDataRate),
                   MakeDataRateChecker ())
    .AddAttribute ("N2apLinkDelay", 
                   "The delay to be used for the N2-AP link to be created",
                   TimeValue (Seconds (0.2)),
                   MakeTimeAccessor (&PointToPointNgcHelper::m_n2apLinkDelay),
                   MakeTimeChecker ())
    .AddAttribute ("N2apLinkMtu", 
                   "The MTU of the next N2-AP link to be created",
                   UintegerValue (2000),
                   MakeUintegerAccessor (&PointToPointNgcHelper::m_n2apLinkMtu),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("X2LinkDataRate",
                   "The data rate to be used for the next X2 link to be created",
                   DataRateValue (DataRate ("10Gb/s")),
                   MakeDataRateAccessor (&PointToPointNgcHelper::m_x2LinkDataRate),
                   MakeDataRateChecker ())
    .AddAttribute ("X2LinkDelay",
                   "The delay to be used for the next X2 link to be created",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&PointToPointNgcHelper::m_x2LinkDelay),
                   MakeTimeChecker ())
    .AddAttribute ("X2LinkMtu",
                   "The MTU of the next X2 link to be created. Note that, because of some big X2 messages, you need a big MTU.",
                   UintegerValue (3000),
                   MakeUintegerAccessor (&PointToPointNgcHelper::m_x2LinkMtu),
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

void
PointToPointNgcHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_tunDevice->SetSendCallback (MakeNullCallback<bool, Ptr<Packet>, const Address&, const Address&, uint16_t> ());
  m_tunDevice = 0;
  m_smfUpfApp = 0;  
  m_smfUpf->Dispose ();
}


void
PointToPointNgcHelper::AddEnb (Ptr<Node> enb, Ptr<NetDevice> nrEnbNetDevice, uint16_t cellId)
{
  NS_LOG_FUNCTION (this << enb << nrEnbNetDevice << cellId);

  NS_ASSERT (enb == nrEnbNetDevice->GetNode ());

  // add an IPv4 stack to the previously created eNB
  InternetStackHelper internet;
  internet.Install (enb);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after node creation: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());

  // create a point to point link between the new eNB and the SMF with
  // the corresponding new NetDevices on each side  
  NodeContainer enbSmfNodes;
  enbSmfNodes.Add (m_smfUpf);
  enbSmfNodes.Add (enb);
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (m_n2uLinkDataRate));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (m_n2uLinkMtu));
  p2ph.SetChannelAttribute ("Delay", TimeValue (m_n2uLinkDelay));  
  NetDeviceContainer enbSmfDevices = p2ph.Install (enb, m_smfUpf);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after installing p2p dev: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());  
  Ptr<NetDevice> enbDev = enbSmfDevices.Get (0);
  Ptr<NetDevice> smfDev = enbSmfDevices.Get (1);
  m_n2uIpv4AddressHelper.NewNetwork ();
  Ipv4InterfaceContainer enbSmfIpIfaces = m_n2uIpv4AddressHelper.Assign (enbSmfDevices);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after assigning Ipv4 addr to N2 dev: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());
  
  Ipv4Address enbAddress = enbSmfIpIfaces.GetAddress (0);
  Ipv4Address smfAddress = enbSmfIpIfaces.GetAddress (1);

  // create N2-U socket for the ENB
  Ptr<Socket> enbN2uSocket = Socket::CreateSocket (enb, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  int retval = enbN2uSocket->Bind (InetSocketAddress (enbAddress, m_gtpuUdpPort));
  NS_ASSERT (retval == 0);


  // create a point to point link between the new eNB and the AMF with
  // the corresponding new NetDevices on each side
  NodeContainer enbAmfNodes;
  enbAmfNodes.Add (m_amfNode);
  enbAmfNodes.Add (enb);
  PointToPointHelper p2ph_amf;
  p2ph_amf.SetDeviceAttribute ("DataRate", DataRateValue (m_n2apLinkDataRate));
  p2ph_amf.SetDeviceAttribute ("Mtu", UintegerValue (m_n2apLinkMtu));
  p2ph_amf.SetChannelAttribute ("Delay", TimeValue (m_n2apLinkDelay));  
  NetDeviceContainer enbAmfDevices = p2ph_amf.Install (enb, m_amfNode);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after installing p2p dev: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());  

  m_n2apIpv4AddressHelper.NewNetwork ();
  Ipv4InterfaceContainer enbAmfIpIfaces = m_n2apIpv4AddressHelper.Assign (enbAmfDevices);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after assigning Ipv4 addr to N2 dev: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());
  
  Ipv4Address amf_enbAddress = enbAmfIpIfaces.GetAddress (0);
  Ipv4Address amfAddress = enbAmfIpIfaces.GetAddress (1);

  // create N2-AP socket for the ENB
  Ptr<Socket> enbN2apSocket = Socket::CreateSocket (enb, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = enbN2apSocket->Bind (InetSocketAddress (amf_enbAddress, m_n2apUdpPort));
  NS_ASSERT (retval == 0);

  // give PacketSocket powers to the eNB
  //PacketSocketHelper packetSocket;
  //packetSocket.Install (enb); 
  
  // create NR socket for the ENB 
  Ptr<Socket> enbNrSocket = Socket::CreateSocket (enb, TypeId::LookupByName ("ns3::PacketSocketFactory"));
  PacketSocketAddress enbNrSocketBindAddress;
  enbNrSocketBindAddress.SetSingleDevice (nrEnbNetDevice->GetIfIndex ());
  enbNrSocketBindAddress.SetProtocol (Ipv4L3Protocol::PROT_NUMBER);
  retval = enbNrSocket->Bind (enbNrSocketBindAddress);
  NS_ASSERT (retval == 0);  
  PacketSocketAddress enbNrSocketConnectAddress;
  enbNrSocketConnectAddress.SetPhysicalAddress (Mac48Address::GetBroadcast ());
  enbNrSocketConnectAddress.SetSingleDevice (nrEnbNetDevice->GetIfIndex ());
  enbNrSocketConnectAddress.SetProtocol (Ipv4L3Protocol::PROT_NUMBER);
  retval = enbNrSocket->Connect (enbNrSocketConnectAddress);
  NS_ASSERT (retval == 0);  
  

  NS_LOG_INFO ("create NgcEnbApplication");
  Ptr<NgcEnbApplication> enbApp = CreateObject<NgcEnbApplication> (enbNrSocket, enbN2uSocket, enbAddress, smfAddress, cellId);
  enb->AddApplication (enbApp);
  NS_ASSERT (enb->GetNApplications () == 1);
  NS_ASSERT_MSG (enb->GetApplication (0)->GetObject<NgcEnbApplication> () != 0, "cannot retrieve NgcEnbApplication");
  NS_LOG_LOGIC ("enb: " << enb << ", enb->GetApplication (0): " << enb->GetApplication (0));

  
  NS_LOG_INFO ("Create NgcX2 entity");
  Ptr<NgcX2> x2 = CreateObject<NgcX2> ();
  enb->AggregateObject (x2);

  NS_LOG_INFO ("connect N2-AP interface");

  uint16_t amfId = 1;
  Ptr<NgcN2apEnb> n2apEnb = CreateObject<NgcN2apEnb> (enbN2apSocket, amf_enbAddress, amfAddress, cellId, amfId); // only one amf!
  enb->AggregateObject(n2apEnb);
  enbApp->SetN2apSapAmf (n2apEnb->GetNgcN2apSapEnbProvider ());
  n2apEnb->SetNgcN2apSapEnbUser (enbApp->GetN2apSapEnb());
  m_amfApp->AddEnb (cellId, amf_enbAddress); // TODO consider if this can be removed
  // add the interface to the N2AP endpoint on the AMF
  Ptr<NgcN2apAmf> n2apAmf = m_amfNode->GetObject<NgcN2apAmf> ();
  n2apAmf->AddN2apInterface (cellId, amf_enbAddress);
  
  m_smfUpfApp->AddEnb (cellId, enbAddress, smfAddress);
}


void
PointToPointNgcHelper::AddX2Interface (Ptr<Node> enb1, Ptr<Node> enb2)
{
  NS_LOG_FUNCTION (this << enb1 << enb2);

  // Create a point to point link between the two eNBs with
  // the corresponding new NetDevices on each side
  NodeContainer enbNodes;
  enbNodes.Add (enb1);
  enbNodes.Add (enb2);
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (m_x2LinkDataRate));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (m_x2LinkMtu));
  p2ph.SetChannelAttribute ("Delay", TimeValue (m_x2LinkDelay));
  NetDeviceContainer enbDevices = p2ph.Install (enb1, enb2);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB #1 after installing p2p dev: " << enb1->GetObject<Ipv4> ()->GetNInterfaces ());
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB #2 after installing p2p dev: " << enb2->GetObject<Ipv4> ()->GetNInterfaces ());
  Ptr<NetDevice> enb1Dev = enbDevices.Get (0);
  Ptr<NetDevice> enb2Dev = enbDevices.Get (1);

  m_x2Ipv4AddressHelper.NewNetwork ();
  Ipv4InterfaceContainer enbIpIfaces = m_x2Ipv4AddressHelper.Assign (enbDevices);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB #1 after assigning Ipv4 addr to X2 dev: " << enb1->GetObject<Ipv4> ()->GetNInterfaces ());
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB #2 after assigning Ipv4 addr to X2 dev: " << enb2->GetObject<Ipv4> ()->GetNInterfaces ());

  Ipv4Address enb1X2Address = enbIpIfaces.GetAddress (0);
  Ipv4Address enb2X2Address = enbIpIfaces.GetAddress (1);

  // Add X2 interface to both eNBs' X2 entities
  Ptr<NgcX2> enb1X2 = enb1->GetObject<NgcX2> ();
  Ptr<NrEnbNetDevice> enb1NrDev = enb1->GetDevice (0)->GetObject<NrEnbNetDevice> ();
  uint16_t enb1CellId = enb1NrDev->GetCellId ();
  NS_LOG_LOGIC ("NrEnbNetDevice #1 = " << enb1NrDev << " - CellId = " << enb1CellId);

  Ptr<NgcX2> enb2X2 = enb2->GetObject<NgcX2> ();
  Ptr<NrEnbNetDevice> enb2NrDev = enb2->GetDevice (0)->GetObject<NrEnbNetDevice> ();
  uint16_t enb2CellId = enb2NrDev->GetCellId ();
  NS_LOG_LOGIC ("NrEnbNetDevice #2 = " << enb2NrDev << " - CellId = " << enb2CellId);

  enb1X2->AddX2Interface (enb1CellId, enb1X2Address, enb2CellId, enb2X2Address);
  enb2X2->AddX2Interface (enb2CellId, enb2X2Address, enb1CellId, enb1X2Address);

  enb1NrDev->GetRrc ()->AddX2Neighbour (enb2NrDev->GetCellId ());
  enb2NrDev->GetRrc ()->AddX2Neighbour (enb1NrDev->GetCellId ());
}


void 
PointToPointNgcHelper::AddUe (Ptr<NetDevice> ueDevice, uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi << ueDevice );
  
  m_amfApp->AddUe (imsi);
  m_smfUpfApp->AddUe (imsi);
  

}

uint8_t
PointToPointNgcHelper::ActivateEpsBearer (Ptr<NetDevice> ueDevice, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << ueDevice << imsi);

  // we now retrieve the IPv4 address of the UE and notify it to the SMF;
  // we couldn't do it before since address assignment is triggered by
  // the user simulation program, rather than done by the NGC   
  Ptr<Node> ueNode = ueDevice->GetNode (); 
  Ptr<Ipv4> ueIpv4 = ueNode->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ueIpv4 != 0, "UEs need to have IPv4 installed before EPS bearers can be activated");
  int32_t interface =  ueIpv4->GetInterfaceForDevice (ueDevice);
  NS_ASSERT (interface >= 0);
  NS_ASSERT (ueIpv4->GetNAddresses (interface) == 1);
  Ipv4Address ueAddr = ueIpv4->GetAddress (interface, 0).GetLocal ();
  NS_LOG_LOGIC (" UE IP address: " << ueAddr);  
  m_smfUpfApp->SetUeAddress (imsi, ueAddr);
  
  uint8_t bearerId = m_amfApp->AddBearer (imsi, tft, bearer);
  Ptr<NrUeNetDevice> ueNrDevice = ueDevice->GetObject<NrUeNetDevice> ();
  if (ueNrDevice)
    {
      ueNrDevice->GetNas ()->ActivateEpsBearer (bearer, tft);
    }
  NS_LOG_LOGIC("Bearer Id added in amfApp " << bearerId);
  return bearerId;
}

uint8_t
PointToPointNgcHelper::ActivateEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NgcUeNas> ueNas, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << ueDevice << imsi);

  // we now retrieve the IPv4 address of the UE and notify it to the SMF;
  // we couldn't do it before since address assignment is triggered by
  // the user simulation program, rather than done by the NGC   
  Ptr<Node> ueNode = ueDevice->GetNode (); 
  Ptr<Ipv4> ueIpv4 = ueNode->GetObject<Ipv4> ();
  NS_ASSERT_MSG (ueIpv4 != 0, "UEs need to have IPv4 installed before EPS bearers can be activated");
  int32_t interface =  ueIpv4->GetInterfaceForDevice (ueDevice);
  NS_ASSERT (interface >= 0);
  NS_ASSERT (ueIpv4->GetNAddresses (interface) == 1);
  Ipv4Address ueAddr = ueIpv4->GetAddress (interface, 0).GetLocal ();
  NS_LOG_LOGIC (" UE IP address: " << ueAddr);  m_smfUpfApp->SetUeAddress (imsi, ueAddr);
  
  uint8_t bearerId = m_amfApp->AddBearer (imsi, tft, bearer);
  ueNas->ActivateEpsBearer (bearer, tft);
  return bearerId;
}


Ptr<Node>
PointToPointNgcHelper::GetUpfNode ()
{
  return m_smfUpf;
}

Ptr<Node>
PointToPointNgcHelper::GetAmfNode ()
{
  return m_amfNode;
}



Ipv4InterfaceContainer 
PointToPointNgcHelper::AssignUeIpv4Address (NetDeviceContainer ueDevices)
{
  return m_ueAddressHelper.Assign (ueDevices);
}



Ipv4Address
PointToPointNgcHelper::GetUeDefaultGatewayAddress ()
{
  // return the address of the tun device
  return m_smfUpf->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
}


} // namespace ns3
