/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab. 
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
 * Author: Michele Polese <michele.polese@gmail.com> 
 *          Support for real N2AP link
 */

#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/ngc-gtpu-header.h"
#include <ns3/simulator.h>

#include "ns3/ngc-n2ap-header.h"
#include "ns3/ngc-n2ap.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NgcN2ap");

N2apIfaceInfo::N2apIfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket)
{
  m_remoteIpAddr = remoteIpAddr;
  m_localCtrlPlaneSocket = localCtrlPlaneSocket;
}

N2apIfaceInfo::~N2apIfaceInfo (void)
{
  m_localCtrlPlaneSocket = 0;
}

N2apIfaceInfo& 
N2apIfaceInfo::operator= (const N2apIfaceInfo& value)
{
  NS_LOG_FUNCTION (this);
  m_remoteIpAddr = value.m_remoteIpAddr;
  m_localCtrlPlaneSocket = value.m_localCtrlPlaneSocket;
  return *this;
}

///////////////////////////////////////////

N2apConnectionInfo::N2apConnectionInfo (uint16_t enbId, uint16_t amfId)
{
  m_enbId = enbId;
  m_amfId = amfId;
}

N2apConnectionInfo::~N2apConnectionInfo (void)
{
  m_enbId = 0;
  m_amfId = 0;
}

N2apConnectionInfo& 
N2apConnectionInfo::operator= (const N2apConnectionInfo& value)
{
  NS_LOG_FUNCTION (this);
  m_enbId = value.m_enbId;
  m_amfId = value.m_amfId;
  return *this;
}

///////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcN2apEnb);

NgcN2apEnb::NgcN2apEnb (Ptr<Socket> localSocket, Ipv4Address enbAddress, Ipv4Address amfAddress, uint16_t cellId, uint16_t amfId)
  : m_n2apUdpPort (36412) // As defined by IANA
{
  NS_LOG_FUNCTION (this);
  AddN2apInterface(cellId, enbAddress, amfId, amfAddress, localSocket);
  m_n2apSapProvider = new MemberNgcN2apSapEnbProvider<NgcN2apEnb> (this);
}

NgcN2apEnb::~NgcN2apEnb ()
{
  NS_LOG_FUNCTION (this);
}

void
NgcN2apEnb::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_n2apInterfaceSockets.clear ();
  m_n2apInterfaceCellIds.clear ();
  delete m_n2apSapProvider;
}

TypeId
NgcN2apEnb::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2apEnb")
    .SetParent<Object> ()
    .SetGroupName("Nr");
  return tid;
}

void
NgcN2apEnb::SetNgcN2apSapEnbUser (NgcN2apSapEnb * s)
{
  NS_LOG_FUNCTION (this << s);
  m_n2apSapUser = s;
}

NgcN2apSapEnbProvider*
NgcN2apEnb::GetNgcN2apSapEnbProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_n2apSapProvider;
}


void
NgcN2apEnb::AddN2apInterface (uint16_t enbId, Ipv4Address enbAddress,
                       uint16_t amfId, Ipv4Address amfAddress,
                       Ptr<Socket> localN2apSocket)
{
  NS_LOG_FUNCTION (this << enbId << enbAddress << amfId << amfAddress);

  localN2apSocket->SetRecvCallback (MakeCallback (&NgcN2apEnb::RecvFromN2apSocket, this));

  NS_ASSERT_MSG (m_n2apInterfaceSockets.find (amfId) == m_n2apInterfaceSockets.end (),
                 "Mapping for amfId = " << amfId << " is already known");
  m_n2apInterfaceSockets [amfId] = Create<N2apIfaceInfo> (amfAddress, localN2apSocket);

  // TODO m_amfId is initialized once since one amf is connected to this enb interface, consider when extending
  m_amfId = amfId;
}


void 
NgcN2apEnb::RecvFromN2apSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  NS_LOG_LOGIC ("Recv N2ap message: N2AP eNB: from Socket at time " << Simulator::Now ().GetSeconds());
  Ptr<Packet> packet = socket->Recv ();
  NS_LOG_LOGIC ("packetLen = " << packet->GetSize ());

  NgcN2APHeader n2apHeader;
  packet->RemoveHeader (n2apHeader);

  NS_LOG_LOGIC ("N2ap header: " << n2apHeader);

  uint8_t procedureCode = n2apHeader.GetProcedureCode ();

  if (procedureCode == NgcN2APHeader::InitialContextSetupRequest)
  {
    NS_LOG_LOGIC ("Recv N2ap message: INITIAL CONTEXT SETUP REQUEST");
    NgcN2APInitialContextSetupRequestHeader reqHeader;
    packet->RemoveHeader(reqHeader);

    NS_LOG_INFO ("N2ap Initial Context Setup Request " << reqHeader);

    uint64_t amfUeN2apId = reqHeader.GetAmfUeN2Id();
    uint16_t enbUeN2apId = reqHeader.GetEnbUeN2Id();
    std::list<NgcN2apSap::ErabToBeSetupItem> erabToBeSetup = reqHeader.GetErabToBeSetupItem ();
    
    NS_LOG_LOGIC ("amfUeN2apId " << amfUeN2apId);
    NS_LOG_LOGIC ("enbUeN2apId " << enbUeN2apId);

    m_n2apSapUser->InitialContextSetupRequest(amfUeN2apId, enbUeN2apId, erabToBeSetup);
   
  }
  else if (procedureCode == NgcN2APHeader::N2Request) //smsohn TODO: add cause to N2RequestHeader
  {
    NS_LOG_LOGIC ("Recv N2ap message: N2 REQUEST");
    NgcN2APN2RequestHeader reqHeader;
    packet->RemoveHeader(reqHeader);

    NS_LOG_INFO ("N2ap N2 Request " << reqHeader);

    uint64_t amfUeN2apId = reqHeader.GetAmfUeN2Id();
    uint16_t enbUeN2apId = reqHeader.GetEnbUeN2Id();
    std::list<NgcN2apSap::ErabToBeSetupItem> erabToBeSetup = reqHeader.GetErabToBeSetupItem ();
    
    NS_LOG_LOGIC ("amfUeN2apId " << amfUeN2apId);
    NS_LOG_LOGIC ("enbUeN2apId " << enbUeN2apId);

    uint16_t cause = 0;
    m_n2apSapUser->N2Request(amfUeN2apId, enbUeN2apId, erabToBeSetup, cause);
   
  }
  else if (procedureCode == NgcN2APHeader::PathSwitchRequestAck)
  {
    NS_LOG_LOGIC ("Recv N2ap message: PATH SWITCH REQUEST ACK");
    NgcN2APPathSwitchRequestAcknowledgeHeader reqHeader;
    packet->RemoveHeader(reqHeader);

    NS_LOG_INFO ("N2ap Path Switch Request Acknowledge Header " << reqHeader);

    uint64_t amfUeN2apId = reqHeader.GetAmfUeN2Id();
    uint16_t enbUeN2apId = reqHeader.GetEnbUeN2Id();
    uint16_t ecgi = reqHeader.GetEcgi();
    std::list<NgcN2apSap::ErabSwitchedInUplinkItem> pathErab = reqHeader.GetErabSwitchedInUplinkItemList ();
    
    NS_LOG_LOGIC ("amfUeN2apId " << amfUeN2apId);
    NS_LOG_LOGIC ("enbUeN2apId " << enbUeN2apId);
    NS_LOG_LOGIC ("ecgi " << ecgi);

    m_n2apSapUser->PathSwitchRequestAcknowledge(enbUeN2apId, amfUeN2apId, ecgi, pathErab);
  }
  else if (procedureCode == NgcN2APHeader::IdentityRequest) /* jhlim: for signal 6. */
  {
	NS_LOG_LOGIC ("Recv N2ap message: IDENTITY REQUEST ");
	NgcN2APInitialContextSetupRequestHeader reqHeader; // fix to Identity request header.
	packet->RemoveHeader(reqHeader);

	NS_LOG_INFO ("N2ap Identity Request " << reqHeader);

	uint64_t amfUeN2apId = reqHeader.GetAmfUeN2Id();
	uint16_t enbUeN2apId = reqHeader.GetEnbUeN2Id();

	NS_LOG_LOGIC ("amfUeN2apId " << amfUeN2apId);
	NS_LOG_LOGIC ("enbUeN2apId " << enbUeN2apId);
	m_n2apSapUser->IdentityRequest(amfUeN2apId, enbUeN2apId);
  }
  else if (procedureCode == NgcN2APHeader::RegistrationAccept) /* jhlim: for signal 11. */
  {
	NS_LOG_LOGIC ("Recv N2ap message: REGISTRATION ACCEPT ");
	NgcN2APInitialContextSetupRequestHeader reqHeader;
	packet->RemoveHeader(reqHeader);

	NS_LOG_INFO ("N2ap Registration Accept " << reqHeader);

	uint64_t amfUeN2apId = reqHeader.GetAmfUeN2Id();
	uint16_t enbUeN2apId = reqHeader.GetEnbUeN2Id();
	uint64_t guti = reqHeader.GetGuti();

	NS_LOG_LOGIC ("amfUeN2apId " << amfUeN2apId);
	NS_LOG_LOGIC ("enbUeN2apId " << enbUeN2apId);
	NS_LOG_LOGIC ("guti " << guti);

	m_n2apSapUser->RegistrationAccept(amfUeN2apId, enbUeN2apId, guti);
  }
  else
  {
	printf("header value: %d\n", procedureCode);
    NS_ASSERT_MSG (false, "ProcedureCode NOT SUPPORTED!!!!!");
  }
}

//
// Implementation of the N2ap SAP Provider
//
void
NgcN2apEnb::DoSendRegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t stmsi, uint16_t ecgi) 
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
  NS_LOG_LOGIC("stmsi = " << stmsi);
  NS_LOG_LOGIC("ecgi = " << ecgi);

  // TODO check if an assert is needed

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [m_amfId]; // in case of multiple amf, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address amfIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("amfIpAddr = " << amfIpAddr);

  NS_LOG_INFO ("Send N2ap message: REGISTRATION REQUEST " << Simulator::Now ().GetSeconds());

  // build the header
  NgcN2APRegistrationRequestHeader registrationRequest;
  registrationRequest.SetAmfUeN2Id(amfUeN2Id);
  registrationRequest.SetEnbUeN2Id(enbUeN2Id);
  registrationRequest.SetSTmsi(stmsi);
  registrationRequest.SetEcgi(ecgi);
  NS_LOG_INFO ("N2ap Registration Request header " << registrationRequest);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::RegistrationRequest);
  n2apHeader.SetLengthOfIes (registrationRequest.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (registrationRequest.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (registrationRequest);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
//  std::cout << "Sends packet to " <<amfIpAddr << ":" <<m_n2apUdpPort << std::endl; // jhlim
  sourceSocket->SendTo (packet, 0, InetSocketAddress (amfIpAddr, m_n2apUdpPort));
}

void
NgcN2apEnb::DoSendN2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t stmsi, uint16_t ecgi) 
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
  NS_LOG_LOGIC("stmsi = " << stmsi);
  NS_LOG_LOGIC("ecgi = " << ecgi);

  // TODO check if an assert is needed

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [m_amfId]; // in case of multiple amf, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address amfIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("amfIpAddr = " << amfIpAddr);

  NS_LOG_INFO ("Send N2ap message: INITIAL UE MESSAGE " << Simulator::Now ().GetSeconds());

  // build the header
  NgcN2APN2MessageHeader N2Message;
  N2Message.SetAmfUeN2Id(amfUeN2Id);
  N2Message.SetEnbUeN2Id(enbUeN2Id);
  N2Message.SetSTmsi(stmsi);
  N2Message.SetEcgi(ecgi);
  NS_LOG_INFO ("N2ap N2 Message header " << N2Message);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::N2Message);
  n2apHeader.SetLengthOfIes (N2Message.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (N2Message.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (N2Message);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
//  std::cout << "Sends packet to " <<amfIpAddr << ":" <<m_n2apUdpPort << std::endl; // jhlim
  sourceSocket->SendTo (packet, 0, InetSocketAddress (amfIpAddr, m_n2apUdpPort));
}

void 
NgcN2apEnb::DoSendErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication )
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);

  // TODO check if an assert is needed

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [m_amfId]; // in case of multiple amf, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address amfIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("amfIpAddr = " << amfIpAddr);

  NS_LOG_INFO ("Send N2ap message: E-RAB RELEASE INDICATION " << Simulator::Now ().GetSeconds());

  NgcN2APErabReleaseIndicationHeader indHeader;
  
  indHeader.SetAmfUeN2Id(amfUeN2Id);
  indHeader.SetEnbUeN2Id(enbUeN2Id);
  indHeader.SetErabReleaseIndication(erabToBeReleaseIndication);
  NS_LOG_INFO ("N2ap E-rab Release Indication header " << indHeader);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::ErabReleaseIndication);
  n2apHeader.SetLengthOfIes (indHeader.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (indHeader.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (indHeader);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (amfIpAddr, m_n2apUdpPort));
}

void 
NgcN2apEnb::DoSendInitialContextSetupResponse (uint64_t amfUeN2Id,
              uint16_t enbUeN2Id,
              std::list<NgcN2apSap::ErabSetupItem> erabSetupList) 
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);

  // TODO check if an assert is needed

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [m_amfId]; // in case of multiple amf, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address amfIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("amfIpAddr = " << amfIpAddr);

  NS_LOG_INFO ("Send N2ap message: INITIAL CONTEXT SETUP RESPONSE " << Simulator::Now ().GetSeconds());

  NgcN2APInitialContextSetupResponseHeader indHeader;
  
  indHeader.SetAmfUeN2Id(amfUeN2Id);
  indHeader.SetEnbUeN2Id(enbUeN2Id);
  indHeader.SetErabSetupItem(erabSetupList);
  NS_LOG_INFO ("N2AP Initial Context Setup Response header " << indHeader);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::InitialContextSetupResponse);
  n2apHeader.SetLengthOfIes (indHeader.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (indHeader.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (indHeader);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (amfIpAddr, m_n2apUdpPort));
}

// jhlim
void 
NgcN2apEnb::DoSendIdentityResponse (uint64_t amfUeN2Id,
              uint16_t enbUeN2Id) 
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);

  // TODO check if an assert is needed

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [m_amfId]; // in case of multiple amf, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address amfIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("amfIpAddr = " << amfIpAddr);

  NS_LOG_INFO ("Send N2ap message: IDENTITY RESPONSE " << Simulator::Now ().GetSeconds());

  NgcN2APInitialContextSetupResponseHeader indHeader; // fix to IdentityResponseHeader.
  
  indHeader.SetAmfUeN2Id(amfUeN2Id);
  indHeader.SetEnbUeN2Id(enbUeN2Id);
  NS_LOG_INFO ("N2AP Identity Response header " << indHeader);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::IdentityResponse);
  n2apHeader.SetLengthOfIes (indHeader.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (indHeader.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (indHeader);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (amfIpAddr, m_n2apUdpPort));
}
void 
NgcN2apEnb::DoSendRegistrationComplete (uint64_t amfUeN2Id,
              uint16_t enbUeN2Id) 
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);

  // TODO check if an assert is needed

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [m_amfId]; // in case of multiple amf, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address amfIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("amfIpAddr = " << amfIpAddr);

  NS_LOG_INFO ("Send N2ap message: REGISTRATION COMPLETE " << Simulator::Now ().GetSeconds());

  NgcN2APInitialContextSetupResponseHeader indHeader; // fix to RegistrationCompleteHeader.
  
  indHeader.SetAmfUeN2Id(amfUeN2Id);
  indHeader.SetEnbUeN2Id(enbUeN2Id);
  NS_LOG_INFO ("N2AP Registration Complete header " << indHeader);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::RegistrationComplete);
  n2apHeader.SetLengthOfIes (indHeader.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (indHeader.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (indHeader);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (amfIpAddr, m_n2apUdpPort));
}

void 
NgcN2apEnb::DoSendPathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t gci, 
            std::list<NgcN2apSap::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
  NS_LOG_LOGIC("ecgi = " << gci);

  // TODO check if an assert is needed

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [m_amfId]; // in case of multiple amf, extend the call
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address amfIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("amfIpAddr = " << amfIpAddr);

  NS_LOG_INFO ("Send N2ap message: PATH SWITCH REQUEST " << Simulator::Now ().GetSeconds());

  NgcN2APPathSwitchRequestHeader indHeader;
  
  indHeader.SetAmfUeN2Id(amfUeN2Id);
  indHeader.SetEnbUeN2Id(enbUeN2Id);
  indHeader.SetEcgi(gci);
  indHeader.SetErabSwitchedInDownlinkItemList(erabToBeSwitchedInDownlinkList);
  NS_LOG_INFO ("N2AP Path Switch Request header " << indHeader);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::PathSwitchRequest);
  n2apHeader.SetLengthOfIes (indHeader.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (indHeader.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (indHeader);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (amfIpAddr, m_n2apUdpPort));
}

//////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED (NgcN2apAmf);

NgcN2apAmf::NgcN2apAmf (const Ptr<Socket> n2apSocket, uint16_t amfId)
  : m_n2apUdpPort (36412) // As defined by IANA
{
  NS_LOG_FUNCTION (this);
  m_localN2APSocket = n2apSocket;
  m_n2apSapProvider = new MemberNgcN2apSapAmfProvider<NgcN2apAmf> (this);
  m_localN2APSocket->SetRecvCallback (MakeCallback (&NgcN2apAmf::RecvFromN2apSocket, this));
  m_amfId = amfId;
}

NgcN2apAmf::~NgcN2apAmf ()
{
  NS_LOG_FUNCTION (this);
}

void
NgcN2apAmf::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_n2apInterfaceSockets.clear ();
  m_n2apInterfaceCellIds.clear ();
  delete m_n2apSapProvider;
}

TypeId
NgcN2apAmf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2apAmf")
    .SetParent<Object> ()
    .SetGroupName("Nr");
  return tid;
}

void
NgcN2apAmf::SetNgcN2apSapAmfUser (NgcN2apSapAmf * s)
{
  NS_LOG_FUNCTION (this << s);
  m_n2apSapUser = s;
}

NgcN2apSapAmfProvider*
NgcN2apAmf::GetNgcN2apSapAmfProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_n2apSapProvider;
}


void
NgcN2apAmf::AddN2apInterface (uint16_t enbId, Ipv4Address enbAddress)
{
  NS_LOG_FUNCTION (this << enbId << enbAddress << m_amfId);

  NS_ASSERT_MSG (m_n2apInterfaceSockets.find (enbId) == m_n2apInterfaceSockets.end (),
                 "Mapping for enbId = " << enbId << " is already known");
  m_n2apInterfaceSockets [enbId] = Create<N2apIfaceInfo> (enbAddress, m_localN2APSocket); // TODO m_localN2APSocket is useless
}

void 
NgcN2apAmf::RecvFromN2apSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  NS_LOG_LOGIC ("Recv N2ap message: N2AP AMF: from Socket " << Simulator::Now ().GetSeconds());
  Ptr<Packet> packet = socket->Recv ();
  NS_LOG_LOGIC ("packetLen = " << packet->GetSize ());

  NgcN2APHeader n2apHeader;
  packet->RemoveHeader (n2apHeader);

  NS_LOG_LOGIC ("N2ap header: " << n2apHeader);

  uint8_t procedureCode = n2apHeader.GetProcedureCode ();

  if (procedureCode == NgcN2APHeader::RegistrationRequest)
  {
    NS_LOG_LOGIC ("Recv N2ap message: REGISTRATION REQUEST");
    NgcN2APRegistrationRequestHeader registrationRequest;
    packet->RemoveHeader(registrationRequest);
    NS_LOG_INFO ("N2ap Registration Request header " << registrationRequest);

    uint64_t amfUeN2Id = registrationRequest.GetAmfUeN2Id();
    uint16_t enbUeN2Id = registrationRequest.GetEnbUeN2Id();
    uint64_t stmsi = registrationRequest.GetSTmsi();
    uint16_t ecgi = registrationRequest.GetEcgi();


    NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
    NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
    NS_LOG_LOGIC("stmsi = " << stmsi);
    NS_LOG_LOGIC("ecgi = " << ecgi);

    // TODO check if ASSERT is needed

    m_n2apSapUser->RegistrationRequest(amfUeN2Id, enbUeN2Id, stmsi, ecgi);

  }
  else if (procedureCode == NgcN2APHeader::PathSwitchRequest)
  {
    NS_LOG_LOGIC ("Recv N2ap message: PATH SWITCH REQUEST " << Simulator::Now ().GetSeconds());
    NgcN2APPathSwitchRequestHeader psrHeader;
    packet->RemoveHeader(psrHeader);
    NS_LOG_INFO ("N2ap Path Switch Request header " << psrHeader);
    
    uint64_t amfUeN2Id = psrHeader.GetAmfUeN2Id();
    uint16_t enbUeN2Id = psrHeader.GetEnbUeN2Id();
    uint16_t ecgi = psrHeader.GetEcgi();

    std::list<NgcN2apSap::ErabSwitchedInDownlinkItem> erabToBeSwitched = psrHeader.GetErabSwitchedInDownlinkItemList ();


    NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
    NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
    NS_LOG_LOGIC("ecgi = " << ecgi);

    m_n2apSapUser->PathSwitchRequest (enbUeN2Id, amfUeN2Id, ecgi, erabToBeSwitched);
  }
  else if (procedureCode == NgcN2APHeader::ErabReleaseIndication)
  {
   NS_LOG_LOGIC ("Recv N2ap message: E-RAB RELEASE INDICATION " << Simulator::Now ().GetSeconds());
    NgcN2APErabReleaseIndicationHeader eriHeader;
    packet->RemoveHeader(eriHeader);
    NS_LOG_INFO ("N2ap Erab Release Indication header " << eriHeader);
    
    uint64_t amfUeN2Id = eriHeader.GetAmfUeN2Id();
    uint16_t enbUeN2Id = eriHeader.GetEnbUeN2Id();

    std::list<NgcN2apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication = eriHeader.GetErabToBeReleaseIndication ();

    NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
    NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);

    m_n2apSapUser->ErabReleaseIndication (amfUeN2Id, enbUeN2Id, erabToBeReleaseIndication);
  }
  else if (procedureCode == NgcN2APHeader::IdentityResponse)
  {
    NS_LOG_LOGIC ("Recv N2ap message: IDENTITY RESPONSE");
    NgcN2APInitialContextSetupResponseHeader reqHeader; // fix to Identity response header.
	
    packet->RemoveHeader(reqHeader);
    NS_LOG_INFO ("N2ap Identity Response header " << reqHeader);

    uint64_t amfUeN2Id = reqHeader.GetAmfUeN2Id();
    uint16_t enbUeN2Id = reqHeader.GetEnbUeN2Id();
    //uint64_t stmsi = registrationRequest.GetSTmsi();
    //uint16_t ecgi = registrationRequest.GetEcgi();


    NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
    NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
    //NS_LOG_LOGIC("stmsi = " << stmsi);
    //NS_LOG_LOGIC("ecgi = " << ecgi);

    // TODO check if ASSERT is needed

    m_n2apSapUser->IdentityResponse(amfUeN2Id, enbUeN2Id);
  }
  else if (procedureCode == NgcN2APHeader::RegistrationComplete) /* jhlim: for signal 12. */
  {
	NS_LOG_LOGIC ("Recv N2ap message: REGISTRATION COMPLETE ");
	NgcN2APInitialContextSetupResponseHeader reqHeader; // fix to registration complete header.
	packet->RemoveHeader(reqHeader);
	NS_LOG_INFO ("N2ap Registration Complete header " << reqHeader);

	uint64_t amfUeN2Id = reqHeader.GetAmfUeN2Id();
	uint16_t enbUeN2Id = reqHeader.GetEnbUeN2Id();

	NS_LOG_LOGIC ("amfUeN2apId " << amfUeN2Id);
	NS_LOG_LOGIC ("enbUeN2apId " << enbUeN2Id);

	m_n2apSapUser->RegistrationComplete(amfUeN2Id, enbUeN2Id);
  }
  else
  {

    NS_ASSERT_MSG (false, "ProcedureCode NOT SUPPORTED!!!");
  }
}

void 
NgcN2apAmf::DoSendInitialContextSetupRequest (uint64_t amfUeN2Id,
                                           uint16_t enbUeN2Id,
                                           std::list<NgcN2apSap::ErabToBeSetupItem> erabToBeSetupList,
                                           uint16_t cellId)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
  NS_LOG_LOGIC("eNB id = " << cellId);

  NS_ASSERT_MSG (m_n2apInterfaceSockets.find (cellId) != m_n2apInterfaceSockets.end (),
               "Missing infos for cellId = " << cellId);

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [cellId];
  Ipv4Address enbIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("enbIpAddr = " << enbIpAddr);

  NS_LOG_INFO ("Send N2ap message: INITIAL CONTEXT SETUP REQUEST " << Simulator::Now ().GetSeconds());

  NgcN2APInitialContextSetupRequestHeader reqHeader;
  
  reqHeader.SetAmfUeN2Id(amfUeN2Id);
  reqHeader.SetEnbUeN2Id(enbUeN2Id);
  reqHeader.SetErabToBeSetupItem(erabToBeSetupList);
  NS_LOG_INFO ("N2AP Initial Context Setup Request header " << reqHeader);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::InitialContextSetupRequest);
  n2apHeader.SetLengthOfIes (reqHeader.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (reqHeader.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (reqHeader);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
  m_localN2APSocket->SendTo (packet, 0, InetSocketAddress (enbIpAddr, m_n2apUdpPort));
}

// jhlim
void 
NgcN2apAmf::DoSendIdentityRequest (uint64_t amfUeN2Id,
                                           uint16_t enbUeN2Id,
                                           uint16_t cellId)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
  NS_LOG_LOGIC("eNB id = " << cellId);

  NS_ASSERT_MSG (m_n2apInterfaceSockets.find (cellId) != m_n2apInterfaceSockets.end (),
               "Missing infos for cellId = " << cellId);

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [cellId];
  Ipv4Address enbIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("enbIpAddr = " << enbIpAddr);

  NS_LOG_INFO ("Send N2ap message: IDENTITY REQUEST " << Simulator::Now ().GetSeconds());

  NgcN2APInitialContextSetupRequestHeader reqHeader; // fix to IdentityRequestHeader.
  
  reqHeader.SetAmfUeN2Id(amfUeN2Id);
  reqHeader.SetEnbUeN2Id(enbUeN2Id);
  NS_LOG_INFO ("N2AP Identity Request header " << reqHeader);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::IdentityRequest);
  n2apHeader.SetLengthOfIes (reqHeader.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (reqHeader.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (reqHeader);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
  m_localN2APSocket->SendTo (packet, 0, InetSocketAddress (enbIpAddr, m_n2apUdpPort));
}

//smsohn
void 
NgcN2apAmf::DoSendN2Request (uint64_t amfUeN2Id,
                                           uint16_t enbUeN2Id,
                                           std::list<NgcN2apSap::ErabToBeSetupItem> erabToBeSetupList,
                                           uint16_t cellId, uint16_t cause)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
  NS_LOG_LOGIC("eNB id = " << cellId);

  NS_ASSERT_MSG (m_n2apInterfaceSockets.find (cellId) != m_n2apInterfaceSockets.end (),
               "Missing infos for cellId = " << cellId);

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [cellId];
  Ipv4Address enbIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("enbIpAddr = " << enbIpAddr);

  NS_LOG_INFO ("Send N2ap message: N2 REQUEST " << Simulator::Now ().GetSeconds());

  NgcN2APN2RequestHeader reqHeader;
  reqHeader.SetAmfUeN2Id(amfUeN2Id);
  reqHeader.SetEnbUeN2Id(enbUeN2Id);
  reqHeader.SetErabToBeSetupItem(erabToBeSetupList);
  NS_LOG_INFO ("N2AP N2 Request header " << reqHeader);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::InitialContextSetupRequest);
  n2apHeader.SetLengthOfIes (reqHeader.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (reqHeader.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (reqHeader);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
//  std::cout << "Sends packet to " <<enbIpAddr << ":" <<m_n2apUdpPort << std::endl; //smsohn
  m_localN2APSocket->SendTo (packet, 0, InetSocketAddress (enbIpAddr, m_n2apUdpPort));
}

void
NgcN2apAmf::DoSendRegistrationAccept (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId, uint64_t guti)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
  NS_LOG_LOGIC("eNB id = " << cellId);

  NS_ASSERT_MSG (m_n2apInterfaceSockets.find (cellId) != m_n2apInterfaceSockets.end (),
               "Missing infos for cellId = " << cellId);

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [cellId];
  Ipv4Address enbIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("enbIpAddr = " << enbIpAddr);

  NS_LOG_INFO ("Send N2ap message: REGISTRATION ACCEPT " << Simulator::Now ().GetSeconds());

  NgcN2APInitialContextSetupRequestHeader reqHeader; // fix to RegistraionAcceptHeader
  
  reqHeader.SetAmfUeN2Id(amfUeN2Id);
  reqHeader.SetEnbUeN2Id(enbUeN2Id);
  NS_LOG_INFO ("N2AP Registration Accept header " << reqHeader);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::RegistrationAccept);
  n2apHeader.SetLengthOfIes (reqHeader.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (reqHeader.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (reqHeader);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
  m_localN2APSocket->SendTo (packet, 0, InetSocketAddress (enbIpAddr, m_n2apUdpPort));
}
  
void 
NgcN2apAmf::DoSendPathSwitchRequestAcknowledge (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, 
                                        std::list<NgcN2apSap::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList)
{
  // cgi is the cellId of the other endpoint of this interface
  uint16_t cellId = cgi;

  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC("amfUeN2apId = " << amfUeN2Id);
  NS_LOG_LOGIC("enbUeN2apId = " << enbUeN2Id);
  NS_LOG_LOGIC("eNB id = " << cellId);

  NS_ASSERT_MSG (m_n2apInterfaceSockets.find (cellId) != m_n2apInterfaceSockets.end (),
               "Missing infos for cellId = " << cellId);

  Ptr<N2apIfaceInfo> socketInfo = m_n2apInterfaceSockets [cellId];
  Ipv4Address enbIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("enbIpAddr = " << enbIpAddr);

  NS_LOG_INFO ("Send N2ap message: PATH SWITCH REQUEST ACKNOWLEDGE " << Simulator::Now ().GetSeconds());

  NgcN2APPathSwitchRequestAcknowledgeHeader reqHeader;
  
  reqHeader.SetAmfUeN2Id(amfUeN2Id);
  reqHeader.SetEnbUeN2Id(enbUeN2Id);
  reqHeader.SetEcgi(cgi);
  reqHeader.SetErabSwitchedInUplinkItemList(erabToBeSwitchedInUplinkList);
  NS_LOG_INFO ("N2AP PathSwitchRequestAcknowledge header " << reqHeader);

  NgcN2APHeader n2apHeader;
  n2apHeader.SetProcedureCode (NgcN2APHeader::PathSwitchRequestAck);
  n2apHeader.SetLengthOfIes (reqHeader.GetLengthOfIes ());
  n2apHeader.SetNumberOfIes (reqHeader.GetNumberOfIes ());
  NS_LOG_INFO ("N2ap header: " << n2apHeader);

  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (reqHeader);
  packet->AddHeader (n2apHeader);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the N2ap message through the socket
  m_localN2APSocket->SendTo (packet, 0, InetSocketAddress (enbIpAddr, m_n2apUdpPort));
}



} // namespace ns3
