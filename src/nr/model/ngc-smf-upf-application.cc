/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jnin@cttc.cat>
 *         Nicola Baldo <nbaldo@cttc.cat>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */


#include "ngc-smf-upf-application.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/ipv4.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ngc-gtpu-header.h"
#include "ns3/abort.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NgcSmfUpfApplication");

/////////////////////////
// UeInfo
/////////////////////////


NgcSmfUpfApplication::UeInfo::UeInfo ()
{
  NS_LOG_FUNCTION (this);
}

void
NgcSmfUpfApplication::UeInfo::AddBearer (Ptr<NgcTft> tft, uint8_t bearerId, uint32_t teid)
{
  NS_LOG_FUNCTION (this << tft << teid);
  m_teidByBearerIdMap[bearerId] = teid;
  return m_tftClassifier.Add (tft, teid);
}

void
NgcSmfUpfApplication::UeInfo::RemoveBearer (uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << bearerId);
  m_teidByBearerIdMap.erase (bearerId);
}

uint32_t
NgcSmfUpfApplication::UeInfo::Classify (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  // we hardcode DOWNLINK direction since the UPF is espected to
  // classify only downlink packets (uplink packets will go to the
  // internet without any classification). 
  return m_tftClassifier.Classify (p, NgcTft::DOWNLINK);
}

Ipv4Address 
NgcSmfUpfApplication::UeInfo::GetEnbAddr ()
{
  return m_enbAddr;
}

void
NgcSmfUpfApplication::UeInfo::SetEnbAddr (Ipv4Address enbAddr)
{
  m_enbAddr = enbAddr;
}

Ipv4Address 
NgcSmfUpfApplication::UeInfo::GetUeAddr ()
{
  return m_ueAddr;
}

void
NgcSmfUpfApplication::UeInfo::SetUeAddr (Ipv4Address ueAddr)
{
  m_ueAddr = ueAddr;
}

/////////////////////////
// NgcSmfUpfApplication
/////////////////////////


TypeId
NgcSmfUpfApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcSmfUpfApplication")
    .SetParent<Object> ()
    .SetGroupName("Nr");
  return tid;
}

void
NgcSmfUpfApplication::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_n2uSocket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
  m_n2uSocket = 0;
  delete (m_n11SapSmf);
}

  

NgcSmfUpfApplication::NgcSmfUpfApplication (const Ptr<VirtualNetDevice> tunDevice, const Ptr<Socket> n2uSocket)
  : m_n2uSocket (n2uSocket),
    m_tunDevice (tunDevice),
    m_gtpuUdpPort (2152), // fixed by the standard
    m_teidCount (0),
    m_n11SapAmf (0)
{
  NS_LOG_FUNCTION (this << tunDevice << n2uSocket);
  m_n2uSocket->SetRecvCallback (MakeCallback (&NgcSmfUpfApplication::RecvFromN2uSocket, this));
  m_n11SapSmf = new MemberNgcN11SapSmf<NgcSmfUpfApplication> (this);
}

  
NgcSmfUpfApplication::~NgcSmfUpfApplication ()
{
  NS_LOG_FUNCTION (this);
}


bool
NgcSmfUpfApplication::RecvFromTunDevice (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << source << dest << packet << packet->GetSize ());

  // get IP address of UE
  Ptr<Packet> pCopy = packet->Copy ();
  Ipv4Header ipv4Header;
  pCopy->RemoveHeader (ipv4Header);
  Ipv4Address ueAddr =  ipv4Header.GetDestination ();
  NS_LOG_LOGIC ("packet addressed to UE " << ueAddr);

  // find corresponding UeInfo address
  std::map<Ipv4Address, Ptr<UeInfo> >::iterator it = m_ueInfoByAddrMap.find (ueAddr);
  if (it == m_ueInfoByAddrMap.end ())
    {        
      NS_LOG_WARN ("unknown UE address " << ueAddr);
    }
  else
    {
      Ipv4Address enbAddr = it->second->GetEnbAddr ();      
      uint32_t teid = it->second->Classify (packet);   
      if (teid == 0)
        {
          NS_LOG_WARN ("no matching bearer for this packet");                   
        }
      else
        {
          SendToN2uSocket (packet, enbAddr, teid);
        }
    }
  // there is no reason why we should notify the TUN
  // VirtualNetDevice that he failed to send the packet: if we receive
  // any bogus packet, it will just be silently discarded.
  const bool succeeded = true;
  return succeeded;
}

void 
NgcSmfUpfApplication::RecvFromN2uSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);  
  NS_ASSERT (socket == m_n2uSocket);
  Ptr<Packet> packet = socket->Recv ();
  NrGtpuHeader gtpu;
  packet->RemoveHeader (gtpu);
  uint32_t teid = gtpu.GetTeid ();

  /// \internal
  /// Workaround for \bugid{231}
  //SocketAddressTag tag;
  //packet->RemovePacketTag (tag);

  SendToTunDevice (packet, teid);
}

void 
NgcSmfUpfApplication::SendToTunDevice (Ptr<Packet> packet, uint32_t teid)
{
  NS_LOG_FUNCTION (this << packet << teid);
  NS_LOG_LOGIC (" packet size: " << packet->GetSize () << " bytes");
  m_tunDevice->Receive (packet, 0x0800, m_tunDevice->GetAddress (), m_tunDevice->GetAddress (), NetDevice::PACKET_HOST);
}

void 
NgcSmfUpfApplication::SendToN2uSocket (Ptr<Packet> packet, Ipv4Address enbAddr, uint32_t teid)
{
  NS_LOG_FUNCTION (this << packet << enbAddr << teid);

  NrGtpuHeader gtpu;
  gtpu.SetTeid (teid);
  // From 3GPP TS 29.281 v10.0.0 Section 5.1
  // Length of the payload + the non obligatory GTP-U header
  gtpu.SetLength (packet->GetSize () + gtpu.GetSerializedSize () - 8);  
  packet->AddHeader (gtpu);
  uint32_t flags = 0;
  m_n2uSocket->SendTo (packet, flags, InetSocketAddress (enbAddr, m_gtpuUdpPort));
}


void 
NgcSmfUpfApplication::SetN11SapAmf (NgcN11SapAmf * s)
{
  m_n11SapAmf = s;
}

NgcN11SapSmf* 
NgcSmfUpfApplication::GetN11SapSmf ()
{
  return m_n11SapSmf;
}

void 
NgcSmfUpfApplication::AddEnb (uint16_t cellId, Ipv4Address enbAddr, Ipv4Address smfAddr)
{
  NS_LOG_FUNCTION (this << cellId << enbAddr << smfAddr);
  EnbInfo enbInfo;
  enbInfo.enbAddr = enbAddr;
  enbInfo.smfAddr = smfAddr;
  m_enbInfoByCellId[cellId] = enbInfo;
}

void 
NgcSmfUpfApplication::AddUe (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  Ptr<UeInfo> ueInfo = Create<UeInfo> ();
  m_ueInfoByImsiMap[imsi] = ueInfo;
}

void 
NgcSmfUpfApplication::SetUeAddress (uint64_t imsi, Ipv4Address ueAddr)
{
  NS_LOG_FUNCTION (this << imsi << ueAddr);
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi); 
  m_ueInfoByAddrMap[ueAddr] = ueit->second;
  ueit->second->SetUeAddr (ueAddr);
}

void 
NgcSmfUpfApplication::DoCreateSessionRequest (NgcN11SapSmf::CreateSessionRequestMessage req)
{
  NS_LOG_FUNCTION (this << req.imsi);
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (req.imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << req.imsi); 
  uint16_t cellId = req.uli.gci;
  std::map<uint16_t, EnbInfo>::iterator enbit = m_enbInfoByCellId.find (cellId);
  NS_ASSERT_MSG (enbit != m_enbInfoByCellId.end (), "unknown CellId " << cellId); 
  Ipv4Address enbAddr = enbit->second.enbAddr;
  ueit->second->SetEnbAddr (enbAddr);

  NgcN11SapAmf::CreateSessionResponseMessage res;
  res.teid = req.imsi; // trick to avoid the need for allocating TEIDs on the N11 interface

  for (std::list<NgcN11SapSmf::BearerContextToBeCreated>::iterator bit = req.bearerContextsToBeCreated.begin ();
       bit != req.bearerContextsToBeCreated.end ();
       ++bit)
    {
      // simple sanity check. If you ever need more than 4M teids
      // throughout your simulation, you'll need to implement a smarter teid
      // management algorithm. 
      NS_ABORT_IF (m_teidCount == 0xFFFFFFFF);
      uint32_t teid = ++m_teidCount;  
      ueit->second->AddBearer (bit->tft, bit->epsBearerId, teid);

      NgcN11SapAmf::BearerContextCreated bearerContext;
      bearerContext.smfFteid.teid = teid;
      bearerContext.smfFteid.address = enbit->second.smfAddr;
      bearerContext.epsBearerId =  bit->epsBearerId; 
      bearerContext.bearerLevelQos = bit->bearerLevelQos; 
      bearerContext.tft = bit->tft;
      res.bearerContextsCreated.push_back (bearerContext);
    }
  m_n11SapAmf->CreateSessionResponse (res);
  
}

void 
NgcSmfUpfApplication::DoUpdateSMContextRequest (NgcN11SapSmf::UpdateSMContextRequestMessage req)
{
  NS_LOG_FUNCTION (this << req.imsi);
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (req.imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << req.imsi); 
  uint16_t cellId = req.uli.gci;
  std::map<uint16_t, EnbInfo>::iterator enbit = m_enbInfoByCellId.find (cellId);
  NS_ASSERT_MSG (enbit != m_enbInfoByCellId.end (), "unknown CellId " << cellId); 
  Ipv4Address enbAddr = enbit->second.enbAddr;
  ueit->second->SetEnbAddr (enbAddr);

  NgcN11SapAmf::UpdateSMContextResponseMessage res;
  res.teid = req.imsi; // trick to avoid the need for allocating TEIDs on the N11 interface

  for (std::list<NgcN11SapSmf::N2SMInformationToBeCreated>::iterator bit = req.n2SMInformationToBeCreated.begin ();
       bit != req.n2SMInformationToBeCreated.end ();
       ++bit)
    {
      // simple sanity check. If you ever need more than 4M teids
      // throughout your simulation, you'll need to implement a smarter teid
      // management algorithm. 
      NS_ABORT_IF (m_teidCount == 0xFFFFFFFF);
      uint32_t teid = ++m_teidCount;  
      ueit->second->AddBearer (bit->tft, bit->qosFlowId, teid);

      
      NgcN11SapAmf::N2SMInformationCreated n2SMInformation;
      n2SMInformation.smfFteid.teid = teid;
      n2SMInformation.smfFteid.address = enbit->second.smfAddr;
      n2SMInformation.qosFlowId =  bit->qosFlowId; 
      n2SMInformation.flowLevelQos = bit->flowLevelQos; 
      n2SMInformation.tft = bit->tft;
/*
      NgcN11SapAmf::BearerContextCreated bearerContext;
      bearerContext.smfFteid.teid = teid;
      bearerContext.smfFteid.address = enbit->second.smfAddr;
      bearerContext.epsBearerId =  bit->epsBearerId; 
      bearerContext.bearerLevelQos = bit->bearerLevelQos; 
      bearerContext.tft = bit->tft;*/
      res.n2SMInformationCreated.push_back (n2SMInformation);
    }
  m_n11SapAmf->UpdateSMContextResponse (res);
  
}
void 
NgcSmfUpfApplication::DoModifyBearerRequest (NgcN11SapSmf::ModifyBearerRequestMessage req)
{
  NS_LOG_FUNCTION (this << req.teid);
  uint64_t imsi = req.teid; // trick to avoid the need for allocating TEIDs on the N11 interface
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi); 
  uint16_t cellId = req.uli.gci;
  std::map<uint16_t, EnbInfo>::iterator enbit = m_enbInfoByCellId.find (cellId);
  NS_ASSERT_MSG (enbit != m_enbInfoByCellId.end (), "unknown CellId " << cellId); 
  Ipv4Address enbAddr = enbit->second.enbAddr;
  ueit->second->SetEnbAddr (enbAddr);
  // no actual bearer modification: for now we just support the minimum needed for path switch request (handover)
  NgcN11SapAmf::ModifyBearerResponseMessage res;
  res.teid = imsi; // trick to avoid the need for allocating TEIDs on the N11 interface
  res.cause = NgcN11SapAmf::ModifyBearerResponseMessage::REQUEST_ACCEPTED;
  m_n11SapAmf->ModifyBearerResponse (res);
}
 
void
NgcSmfUpfApplication::DoDeleteBearerCommand (NgcN11SapSmf::DeleteBearerCommandMessage req)
{
  NS_LOG_FUNCTION (this << req.teid);
  uint64_t imsi = req.teid; // trick to avoid the need for allocating TEIDs on the N11 interface
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi);

  NgcN11SapAmf::DeleteBearerRequestMessage res;
  res.teid = imsi;

  for (std::list<NgcN11SapSmf::BearerContextToBeRemoved>::iterator bit = req.bearerContextsToBeRemoved.begin ();
       bit != req.bearerContextsToBeRemoved.end ();
       ++bit)
    {
      NgcN11SapAmf::BearerContextRemoved bearerContext;
      bearerContext.epsBearerId =  bit->epsBearerId;
      res.bearerContextsRemoved.push_back (bearerContext);
    }
  //schedules Delete Bearer Request towards AMF
  m_n11SapAmf->DeleteBearerRequest (res);
}

void
NgcSmfUpfApplication::DoDeleteBearerResponse (NgcN11SapSmf::DeleteBearerResponseMessage req)
{
  NS_LOG_FUNCTION (this << req.teid);
  uint64_t imsi = req.teid; // trick to avoid the need for allocating TEIDs on the N11 interface
  std::map<uint64_t, Ptr<UeInfo> >::iterator ueit = m_ueInfoByImsiMap.find (imsi);
  NS_ASSERT_MSG (ueit != m_ueInfoByImsiMap.end (), "unknown IMSI " << imsi);

  for (std::list<NgcN11SapSmf::BearerContextRemovedSmfUpf>::iterator bit = req.bearerContextsRemoved.begin ();
       bit != req.bearerContextsRemoved.end ();
       ++bit)
    {
      //Function to remove de-activated bearer contexts from S-Gw and P-Gw side
      ueit->second->RemoveBearer (bit->epsBearerId);
    }
}

}  // namespace ns3
