/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 */

#include "original_emu-ngc-helper.h"
#include <ns3/log.h>
#include <ns3/inet-socket-address.h>
#include <ns3/mac48-address.h>
#include <ns3/eps-bearer.h>
#include <ns3/ipv4-address.h>
#include <ns3/internet-stack-helper.h>
#include <ns3/packet-socket-helper.h>
#include <ns3/packet-socket-address.h>
#include <ns3/ngc-enb-application.h>
#include <ns3/original_ngc-enb-application.h> //sjkang_x
#include <ns3/ngc-smf-upf-application.h>
#include <ns3/emu-fd-net-device-helper.h>

#include <ns3/nr-enb-rrc.h>
#include <ns3/ngc-x2.h>
#include <ns3/nr-enb-net-device.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/ngc-amf.h>
#include <ns3/ngc-ue-nas.h>
#include <ns3/string.h>
#include <ns3/abort.h>

#include <iomanip>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EmuNgcHelper1");

NS_OBJECT_ENSURE_REGISTERED (EmuNgcHelper1);


EmuNgcHelper1::EmuNgcHelper1 () 
  : m_gtpuUdpPort (2152)  // fixed by the standard
{
  NS_LOG_FUNCTION (this);

}

EmuNgcHelper1::~EmuNgcHelper1 ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
EmuNgcHelper1::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EmuNgcHelper1")
    .SetParent<NgcHelper1> ()
    .SetGroupName("Nr")
    .AddConstructor<EmuNgcHelper1> ()
    .AddAttribute ("smfDeviceName", 
                   "The name of the device used for the N2-U interface of the SMF",
                   StringValue ("veth0"),
                   MakeStringAccessor (&EmuNgcHelper1::m_smfDeviceName),
                   MakeStringChecker ())
    .AddAttribute ("enbDeviceName", 
                   "The name of the device used for the N2-U interface of the eNB",
                   StringValue ("veth1"),
                   MakeStringAccessor (&EmuNgcHelper1::m_enbDeviceName),
                   MakeStringChecker ())
    .AddAttribute ("SmfMacAddress", 
                   "MAC address used for the SMF ",
                   StringValue ("00:00:00:59:00:aa"),
                   MakeStringAccessor (&EmuNgcHelper1::m_smfMacAddress),
                   MakeStringChecker ())
    .AddAttribute ("EnbMacAddressBase", 
                   "First 5 bytes of the Enb MAC address base",
                   StringValue ("00:00:00:eb:00"),
                   MakeStringAccessor (&EmuNgcHelper1::m_enbMacAddressBase),
                   MakeStringChecker ())
    ;
  return tid;
}

void
EmuNgcHelper1::DoInitialize ()
{
  NS_LOG_LOGIC (this);   


  // we use a /8 net for all UEs
  m_ueAddressHelper.SetBase ("7.0.0.0", "255.0.0.0");

  
 
  // create SmfUpfNode
  m_smfUpf = CreateObject<Node> ();
  InternetStackHelper internet;
  internet.SetIpv4StackInstall (true);
  internet.Install (m_smfUpf);
  
  // create N2-U socket
  Ptr<Socket> smfUpfN2uSocket = Socket::CreateSocket (m_smfUpf, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  int retval = smfUpfN2uSocket->Bind (InetSocketAddress (Ipv4Address::GetAny (), m_gtpuUdpPort));
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

  // Create AMF and connect with SMF via N11 interface
  m_amf = CreateObject<NgcAmf> ();
  m_amf->SetN11SapSmf (m_smfUpfApp->GetN11SapSmf ());
  m_smfUpfApp->SetN11SapAmf (m_amf->GetN11SapAmf ());

  // Create EmuFdNetDevice for SMF
  EmuFdNetDeviceHelper emu;
  NS_LOG_LOGIC ("SMF device: " << m_smfDeviceName);
  emu.SetDeviceName (m_smfDeviceName);
  NetDeviceContainer smfDevices = emu.Install (m_smfUpf);
  Ptr<NetDevice> smfDevice = smfDevices.Get (0);
  NS_LOG_LOGIC ("MAC address of SMF: " << m_smfMacAddress);
  smfDevice->SetAttribute ("Address", Mac48AddressValue (m_smfMacAddress.c_str ()));

  // we use a /8 subnet so the SMF and the eNBs can talk directly to each other
  m_ngcIpv4AddressHelper.SetBase ("10.0.0.0", "255.255.255.0", "0.0.0.1");  
  m_smfIpIfaces = m_ngcIpv4AddressHelper.Assign (smfDevices);
  m_ngcIpv4AddressHelper.SetBase ("10.0.0.0", "255.0.0.0", "0.0.0.101");  
  
  
  NgcHelper1::DoInitialize ();
}

void
EmuNgcHelper1::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_tunDevice->SetSendCallback (MakeNullCallback<bool, Ptr<Packet>, const Address&, const Address&, uint16_t> ());
  m_tunDevice = 0;
  m_smfUpfApp = 0;  
  m_smfUpf->Dispose ();
}


void
EmuNgcHelper1::AddEnb (Ptr<Node> enb, Ptr<NetDevice> nrEnbNetDevice, uint16_t cellId)
{
  NS_LOG_FUNCTION (this << enb << nrEnbNetDevice << cellId);

  Initialize ();

  NS_ASSERT (enb == nrEnbNetDevice->GetNode ());  

  // add an IPv4 stack to the previously created eNB
  InternetStackHelper internet;
  internet.Install (enb);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after node creation: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());



  // Create an EmuFdNetDevice for the eNB to connect with the SMF and other eNBs
  EmuFdNetDeviceHelper emu;
  NS_LOG_LOGIC ("eNB device: " << m_enbDeviceName);
  emu.SetDeviceName (m_enbDeviceName);  
  NetDeviceContainer enbDevices = emu.Install (enb);

  NS_ABORT_IF ((cellId == 0) || (cellId > 255));
  std::ostringstream enbMacAddress;
  enbMacAddress << m_enbMacAddressBase << ":" << std::hex << std::setfill ('0') << std::setw (2) << cellId;
  NS_LOG_LOGIC ("MAC address of enB with cellId " << cellId << " : " << enbMacAddress.str ());
  Ptr<NetDevice> enbDev = enbDevices.Get (0);
  enbDev->SetAttribute ("Address", Mac48AddressValue (enbMacAddress.str ().c_str ()));

  //emu.EnablePcap ("enbDevice", enbDev);

  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after installing emu dev: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());  
  Ipv4InterfaceContainer enbIpIfaces = m_ngcIpv4AddressHelper.Assign (enbDevices);
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB after assigning Ipv4 addr to N2 dev: " << enb->GetObject<Ipv4> ()->GetNInterfaces ());
  
  Ipv4Address enbAddress = enbIpIfaces.GetAddress (0);
  Ipv4Address smfAddress = m_smfIpIfaces.GetAddress (0);

  // create N2-U socket for the ENB
  Ptr<Socket> enbN2uSocket = Socket::CreateSocket (enb, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  int retval = enbN2uSocket->Bind (InetSocketAddress (enbAddress, m_gtpuUdpPort));
  NS_ASSERT (retval == 0);
    
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
 //<NgcEnbApplication> enbApp = CreateObject<NgcEnbApplication> (enbNrSocket, enbN2uSocket, enbAddress, smfAddress, cellId);
  Ptr<NgcEnbApplication1> enbApp = CreateObject<NgcEnbApplication1> (enbNrSocket, enbN2uSocket, enbAddress, smfAddress, cellId); //sjkang_x
  enb->AddApplication (enbApp);
  NS_ASSERT (enb->GetNApplications () == 1);
  NS_ASSERT_MSG (enb->GetApplication (0)->GetObject<NgcEnbApplication> () != 0, "cannot retrieve NgcEnbApplication");
  NS_LOG_LOGIC ("enb: " << enb << ", enb->GetApplication (0): " << enb->GetApplication (0));

  
  NS_LOG_INFO ("Create NgcX2 entity");
  Ptr<NgcX2> x2 = CreateObject<NgcX2> ();
  enb->AggregateObject (x2);

  NS_LOG_INFO ("connect N2-AP interface");
  m_amf->AddEnb (cellId, enbAddress, enbApp->GetN2apSapEnb ());
  m_smfUpfApp->AddEnb (cellId, enbAddress, smfAddress);
  enbApp->SetN2apSapAmf (m_amf->GetN2apSapAmf ());
}


void
EmuNgcHelper1::AddX2Interface (Ptr<Node> enb1, Ptr<Node> enb2)
{
  NS_LOG_FUNCTION (this << enb1 << enb2);

  NS_LOG_WARN ("X2 support still untested");


  // for X2, we reuse the same device and IP address of the N2-U interface
  Ptr<Ipv4> enb1Ipv4 = enb1->GetObject<Ipv4> ();
  Ptr<Ipv4> enb2Ipv4 = enb2->GetObject<Ipv4> ();
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB #1: " << enb1Ipv4->GetNInterfaces ());
  NS_LOG_LOGIC ("number of Ipv4 ifaces of the eNB #2: " << enb2Ipv4->GetNInterfaces ());
  NS_LOG_LOGIC ("number of NetDevices of the eNB #1: " << enb1->GetNDevices ());
  NS_LOG_LOGIC ("number of NetDevices of the eNB #2: " << enb2->GetNDevices ());

  // 0 is the NR device, 1 is localhost, 2 is the NGC NetDevice
  Ptr<NetDevice> enb1NgcDev = enb1->GetDevice (2);
  Ptr<NetDevice> enb2NgcDev = enb2->GetDevice (2);

  int32_t enb1Interface =  enb1Ipv4->GetInterfaceForDevice (enb1NgcDev);
  int32_t enb2Interface =  enb2Ipv4->GetInterfaceForDevice (enb2NgcDev);
  NS_ASSERT (enb1Interface >= 0);
  NS_ASSERT (enb2Interface >= 0);
  NS_ASSERT (enb1Ipv4->GetNAddresses (enb1Interface) == 1);
  NS_ASSERT (enb2Ipv4->GetNAddresses (enb2Interface) == 1);
  Ipv4Address enb1Addr = enb1Ipv4->GetAddress (enb1Interface, 0).GetLocal (); 
  Ipv4Address enb2Addr = enb2Ipv4->GetAddress (enb2Interface, 0).GetLocal (); 
  NS_LOG_LOGIC (" eNB 1 IP address: " << enb1Addr); 
  NS_LOG_LOGIC (" eNB 2 IP address: " << enb2Addr);
  
  // Add X2 interface to both eNBs' X2 entities
  Ptr<NgcX2> enb1X2 = enb1->GetObject<NgcX2> ();
  Ptr<NrEnbNetDevice> enb1NrDev = enb1->GetDevice (0)->GetObject<NrEnbNetDevice> ();
  uint16_t enb1CellId = enb1NrDev->GetCellId ();
  NS_LOG_LOGIC ("NrEnbNetDevice #1 = " << enb1NrDev << " - CellId = " << enb1CellId);

  Ptr<NgcX2> enb2X2 = enb2->GetObject<NgcX2> ();
  Ptr<NrEnbNetDevice> enb2NrDev = enb2->GetDevice (0)->GetObject<NrEnbNetDevice> ();
  uint16_t enb2CellId = enb2NrDev->GetCellId ();
  NS_LOG_LOGIC ("NrEnbNetDevice #2 = " << enb2NrDev << " - CellId = " << enb2CellId);

  enb1X2->AddX2Interface (enb1CellId, enb1Addr, enb2CellId, enb2Addr);
  enb2X2->AddX2Interface (enb2CellId, enb2Addr, enb1CellId, enb1Addr);

  enb1NrDev->GetRrc ()->AddX2Neighbour (enb2NrDev->GetCellId ());
  enb2NrDev->GetRrc ()->AddX2Neighbour (enb1NrDev->GetCellId ());
}


void 
EmuNgcHelper1::AddUe (Ptr<NetDevice> ueDevice, uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi << ueDevice );
  
  m_amf->AddUe (imsi);
  m_smfUpfApp->AddUe (imsi);
  
}

uint8_t
EmuNgcHelper1::ActivateEpsBearer (Ptr<NetDevice> ueDevice, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer)
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
  
  uint8_t bearerId = m_amf->AddBearer (imsi, tft, bearer);
  Ptr<NrUeNetDevice> ueNrDevice = ueDevice->GetObject<NrUeNetDevice> ();
  if (ueNrDevice)
    {
      Simulator::ScheduleNow (&NgcUeNas::ActivateEpsBearer, ueNrDevice->GetNas (), bearer, tft);
    }
  return bearerId;
}


Ptr<Node>
EmuNgcHelper1::GetUpfNode ()
{
  return m_smfUpf;
}


Ipv4InterfaceContainer 
EmuNgcHelper1::AssignUeIpv4Address (NetDeviceContainer ueDevices)
{
  return m_ueAddressHelper.Assign (ueDevices);
}



Ipv4Address
EmuNgcHelper1::GetUeDefaultGatewayAddress ()
{
  // return the address of the tun device
  return m_smfUpf->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
}


} // namespace ns3
