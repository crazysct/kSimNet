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
 * Modified by Michele Polese <michele.polese@gmail.com>
 *     (support for RRC_CONNECTED->RRC_IDLE state transition + support for real N2AP link)
 */


#include "ngc-enb-application.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/ipv4.h"
#include "ns3/inet-socket-address.h"
#include "ns3/uinteger.h"

#include "ngc-gtpu-header.h"
#include "eps-bearer-tag.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NgcEnbApplication");

NgcEnbApplication::EpsFlowId_t::EpsFlowId_t ()
{
}

NgcEnbApplication::EpsFlowId_t::EpsFlowId_t (const uint16_t a, const uint8_t b)
  : m_rnti (a),
    m_bid (b)
{
}

bool
operator == (const NgcEnbApplication::EpsFlowId_t &a, const NgcEnbApplication::EpsFlowId_t &b)
{
  return ( (a.m_rnti == b.m_rnti) && (a.m_bid == b.m_bid) );
}

bool
operator < (const NgcEnbApplication::EpsFlowId_t& a, const NgcEnbApplication::EpsFlowId_t& b)
{
  return ( (a.m_rnti < b.m_rnti) || ( (a.m_rnti == b.m_rnti) && (a.m_bid < b.m_bid) ) );
}


TypeId
NgcEnbApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcEnbApplication")
    .SetParent<Object> ()
    .SetGroupName("Nr");
  return tid;
}

void
NgcEnbApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_nrSocket = 0;
  m_n2uSocket = 0;
  delete m_n2SapProvider;
  delete m_n2apSapEnb;
}


NgcEnbApplication::NgcEnbApplication (Ptr<Socket> nrSocket, Ptr<Socket> n2uSocket, Ipv4Address enbN2uAddress, Ipv4Address smfN2uAddress, uint16_t cellId)
  : m_nrSocket (nrSocket),
    m_n2uSocket (n2uSocket),    
    m_enbN2uAddress (enbN2uAddress),
    m_smfN2uAddress (smfN2uAddress),
    m_gtpuUdpPort (2152), // fixed by the standard
    m_n2SapUser (0),
    m_n2apSapEnbProvider (0),
	//m_n2apSapAmf (0), // jhlim
    m_cellId (cellId)
{
  NS_LOG_FUNCTION (this << nrSocket << n2uSocket << smfN2uAddress);
  m_n2uSocket->SetRecvCallback (MakeCallback (&NgcEnbApplication::RecvFromN2uSocket, this));
  m_nrSocket->SetRecvCallback (MakeCallback (&NgcEnbApplication::RecvFromNrSocket, this));
  m_n2SapProvider = new MemberNgcEnbN2SapProvider<NgcEnbApplication> (this);
  m_n2apSapEnb = new MemberNgcN2apSapEnb<NgcEnbApplication> (this);
}


NgcEnbApplication::~NgcEnbApplication (void)
{
  NS_LOG_FUNCTION (this);
}


void 
NgcEnbApplication::SetN2SapUser (NgcEnbN2SapUser * s)
{
  m_n2SapUser = s;
}


NgcEnbN2SapProvider* 
NgcEnbApplication::GetN2SapProvider ()
{
  return m_n2SapProvider;
}

void 
NgcEnbApplication::SetN2apSapAmf (NgcN2apSapEnbProvider * s)
{
	m_n2apSapEnbProvider = s;
}

// jhlim 
void
NgcEnbApplication::SetN2apSapAmf (NgcN2apSapAmf * s)
{
  m_n2apSapAmf = s;
}
  
NgcN2apSapEnb* 
NgcEnbApplication::GetN2apSapEnb ()
{
  return m_n2apSapEnb;
}

bool
NgcEnbApplication::is5gGuti(uint64_t imsi)
{
	return false;
}

std::string
NgcEnbApplication::GetCmState(uint64_t imsi)
{
	std::string state = "CM-IDLE"; // UE doesn't have a connected AMF.
	return state;
}

/* jhlim */
NgcN2apSapAmf*
NgcEnbApplication::GetUeConnectedAmf(uint64_t imsi)
{
	// find AMF included in imsi(5G-GUTI).
	return m_n2apSapAmf;
}

NgcN2apSapAmf*
NgcEnbApplication::DoAmfSelection(uint64_t imsi)
{
	std::string state = GetCmState(imsi);
	NS_ASSERT_MSG((state != "CM-CONNECTED") && (state != "CM-IDLE"), "UE's CM-STATE has wrong value.");
	if(state == "CM-CONNECTED") {
		return GetUeConnectedAmf(imsi);
	}
	else { // state "CM-IDLE")
		return m_n2apSapAmf;
	}
}

/* jhlim: In 5G, one of AMFs is selected.
		 But in lena, only one AMF is registered by Enb  */
void 
NgcEnbApplication::DoRegistrationRequest (uint64_t imsi, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  // side effect: create entry if not exist
  m_imsiRntiMap[imsi] = rnti;

  /* jhlim: 2. AMF Selection */
  if(m_n2apSapEnbProvider == NULL)
	  m_n2apSapAmf = DoAmfSelection(imsi);
	  
  /* jhlim: forwards the Registration Request to an AMF */

  if(m_n2apSapEnbProvider == NULL)
  {
	  m_n2apSapAmf->RegistrationRequest (imsi, rnti, imsi, m_cellId);
  }
  else
  {
	  m_n2apSapEnbProvider->SendRegistrationRequest (imsi, rnti, imsi, m_cellId); // TODO if more than one AMF is used, extend this call
  }

}

// jhlim
void
NgcEnbApplication::DoIdentityResponse (uint64_t imsi, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);

  m_imsiRntiMap[imsi] = rnti;

  if(m_n2apSapEnbProvider == NULL)
	  m_n2apSapAmf->IdentityResponse (imsi, rnti);
  else
	  m_n2apSapEnbProvider->SendIdentityResponse (imsi, rnti); // TODO if more than one AMF is used, extend this call
}
void 
NgcEnbApplication::DoN2Message (uint64_t imsi, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  // side effect: create entry if not exist
  m_imsiRntiMap[imsi] = rnti;
  //m_n2apSapAmf->InitialUeMessage (imsi, rnti, imsi, m_cellId); // jhlim
  if(m_n2apSapEnbProvider == NULL)
	  m_n2apSapAmf->N2Message (imsi, rnti, imsi, m_cellId);
  else
	  m_n2apSapEnbProvider->SendN2Message (imsi, rnti, imsi, m_cellId); // TODO if more than one AMF is used, extend this call

}

/* jhlim */
void
NgcEnbApplication::DoRegistrationComplete (uint64_t imsi, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);

  m_imsiRntiMap[imsi] = rnti;

  if(m_n2apSapEnbProvider == NULL)
	  m_n2apSapAmf->RegistrationComplete (imsi, rnti);
  else
	  m_n2apSapEnbProvider->SendRegistrationComplete (imsi, rnti);
}

void
NgcEnbApplication::DoN2Message (uint64_t imsi, uint16_t rnti, int dummy)
{
	NS_LOG_FUNCTION (this);
	m_imsiRntiMap[imsi] = rnti;
	m_n2apSapAmf->N2Message (imsi, rnti, imsi, m_cellId);
}

void 
NgcEnbApplication::DoPathSwitchRequest (NgcEnbN2SapProvider::PathSwitchRequestParameters params)
{
  NS_LOG_FUNCTION (this);
  uint16_t enbUeN2Id = params.rnti;  
  uint64_t amfUeN2Id = params.amfUeN2Id;
  uint64_t imsi = amfUeN2Id;
  // side effect: create entry if not exist
  m_imsiRntiMap[imsi] = params.rnti;

  uint16_t gci = params.cellId;
  std::list<NgcN2apSapAmf::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList;
  for (std::list<NgcEnbN2SapProvider::BearerToBeSwitched>::iterator bit = params.bearersToBeSwitched.begin ();
       bit != params.bearersToBeSwitched.end ();
       ++bit)
    {
      EpsFlowId_t flowId;
      flowId.m_rnti = params.rnti;
      flowId.m_bid = bit->epsBearerId;
      uint32_t teid = bit->teid;
      
      EpsFlowId_t rbid (params.rnti, bit->epsBearerId);
      // side effect: create entries if not exist
      m_rbidTeidMap[params.rnti][bit->epsBearerId] = teid;
      m_teidRbidMap[teid] = rbid;

      NgcN2apSapAmf::ErabSwitchedInDownlinkItem erab;
      erab.erabId = bit->epsBearerId;
      erab.enbTransportLayerAddress = m_enbN2uAddress;
      erab.enbTeid = bit->teid;

      erabToBeSwitchedInDownlinkList.push_back (erab);
    }
  m_n2apSapEnbProvider->SendPathSwitchRequest (enbUeN2Id, amfUeN2Id, gci, erabToBeSwitchedInDownlinkList);
}

/* jhlim */
/*
void 
NgcEnbApplication::DoPathSwitchRequest (NgcEnbN2SapProvider::PathSwitchRequestParameters params, int dummy)
{
  NS_LOG_FUNCTION (this);
  uint16_t enbUeN2Id = params.rnti;  
  uint64_t amfUeN2Id = params.amfUeN2Id;
  uint64_t imsi = amfUeN2Id;
  // side effect: create entry if not exist
  m_imsiRntiMap[imsi] = params.rnti;

  uint16_t gci = params.cellId;
  std::list<NgcN2apSapAmf::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList;
  for (std::list<NgcEnbN2SapProvider::BearerToBeSwitched>::iterator bit = params.bearersToBeSwitched.begin ();
       bit != params.bearersToBeSwitched.end ();
       ++bit)
    {
      EpsFlowId_t flowId;
      flowId.m_rnti = params.rnti;
      flowId.m_bid = bit->epsBearerId;
      uint32_t teid = bit->teid;
      
      EpsFlowId_t rbid (params.rnti, bit->epsBearerId);
      // side effect: create entries if not exist
      m_rbidTeidMap[params.rnti][bit->epsBearerId] = teid;
      m_teidRbidMap[teid] = rbid;

      NgcN2apSapAmf::ErabSwitchedInDownlinkItem erab;
      erab.erabId = bit->epsBearerId;
      erab.enbTransportLayerAddress = m_enbN2uAddress;
      erab.enbTeid = bit->teid;

      erabToBeSwitchedInDownlinkList.push_back (erab);
    }
  m_n2apSapAmf->PathSwitchRequest (enbUeN2Id, amfUeN2Id, gci, erabToBeSwitchedInDownlinkList);
}
*/
void 
NgcEnbApplication::DoUeContextRelease (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::map<uint16_t, std::map<uint8_t, uint32_t> >::iterator rntiIt = m_rbidTeidMap.find (rnti);
  if (rntiIt != m_rbidTeidMap.end ())
    {
      for (std::map<uint8_t, uint32_t>::iterator bidIt = rntiIt->second.begin ();
           bidIt != rntiIt->second.end ();
           ++bidIt)
        {
          uint32_t teid = bidIt->second;
          m_teidRbidMap.erase (teid);
        }
      m_rbidTeidMap.erase (rntiIt);
    }
}

void 
NgcEnbApplication::DoInitialContextSetupRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapEnb::ErabToBeSetupItem> erabToBeSetupList)
{ //sjkang1021
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO("In EnpEnbApplication DoInitialContextSetupRequest size of the erabToBeSetupList is " << erabToBeSetupList.size());

  for (std::list<NgcN2apSapEnb::ErabToBeSetupItem>::iterator erabIt = erabToBeSetupList.begin ();
       erabIt != erabToBeSetupList.end ();
       ++erabIt)
    {
      // request the RRC to setup a radio bearer
      uint64_t imsi = amfUeN2Id;
      std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
      NS_ASSERT_MSG (imsiIt != m_imsiRntiMap.end (), "unknown IMSI");
      uint16_t rnti = imsiIt->second;
      
      struct NgcEnbN2SapUser::DataRadioBearerSetupRequestParameters params;
      params.rnti = rnti;
      params.bearer = erabIt->erabLevelQosParameters;
      params.bearerId = erabIt->erabId;
      params.gtpTeid = erabIt->smfTeid;
      m_n2SapUser->DataRadioBearerSetupRequest (params);

      EpsFlowId_t rbid (rnti, erabIt->erabId);
      // side effect: create entries if not exist
      m_rbidTeidMap[rnti][erabIt->erabId] = params.gtpTeid;
      m_teidRbidMap[params.gtpTeid] = rbid;

    }
}

//jhlim
void 
NgcEnbApplication::DoIdentityRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO("In EnpEnbApplication DoIdentityRequest");

  uint64_t imsi = amfUeN2Id;
  std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
  uint16_t rnti = imsiIt->second;
  struct NgcEnbN2SapUser::IdentityRequestParameters params;
  params.rnti = rnti;
  m_n2SapUser->IdentityRequest(params);
}
void 
NgcEnbApplication::DoRegistrationAccept (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t guti)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO("In EnpEnbApplication DoRegistrationAccept");

  uint64_t imsi = amfUeN2Id;
  std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
  uint16_t rnti = imsiIt->second;
  struct NgcEnbN2SapUser::RegistrationAcceptParameters params;
  params.rnti = rnti;
  m_n2SapUser->RegistrationAccept(params);
}
//smsohn 
void 
NgcEnbApplication::DoN2Request (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapEnb::ErabToBeSetupItem> erabToBeSetupList, uint16_t cause)
{ //sjkang1021
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO("In EnpEnbApplication DoN2Request size of the erabToBeSetupList is " << erabToBeSetupList.size());

  for (std::list<NgcN2apSapEnb::ErabToBeSetupItem>::iterator erabIt = erabToBeSetupList.begin ();
       erabIt != erabToBeSetupList.end ();
       ++erabIt)
    {
      // request the RRC to setup a radio bearer
      uint64_t imsi = amfUeN2Id;
      std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
      NS_ASSERT_MSG (imsiIt != m_imsiRntiMap.end (), "unknown IMSI");
      uint16_t rnti = imsiIt->second;
      
      struct NgcEnbN2SapUser::DataRadioBearerSetupRequestParameters params;
      params.rnti = rnti;
      params.flow = erabIt->erabLevelQosParameters;
      params.flowId = erabIt->erabId;
      params.gtpTeid = erabIt->smfTeid;
      m_n2SapUser->DataRadioBearerSetupRequest (params);
      //smsohn TODO: DataRadioBearerSetupRequest

      EpsFlowId_t rbid (rnti, erabIt->erabId);
      // side effect: create entries if not exist
      m_rbidTeidMap[rnti][erabIt->erabId] = params.gtpTeid;
      m_teidRbidMap[params.gtpTeid] = rbid;

    }
}

void 
NgcEnbApplication::DoPathSwitchRequestAcknowledge (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t gci, std::list<NgcN2apSapEnb::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList)
{
  NS_LOG_FUNCTION (this);

  uint64_t imsi = amfUeN2Id;
  std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
  NS_ASSERT_MSG (imsiIt != m_imsiRntiMap.end (), "unknown IMSI");
  uint16_t rnti = imsiIt->second;
  NgcEnbN2SapUser::PathSwitchRequestAcknowledgeParameters params;
  params.rnti = rnti;
  m_n2SapUser->PathSwitchRequestAcknowledge (params);
}

void 
NgcEnbApplication::RecvFromNrSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);  
  NS_ASSERT (socket == m_nrSocket);
  Ptr<Packet> packet = socket->Recv ();

  /// \internal
  /// Workaround for \bugid{231}
  //SocketAddressTag satag;
  //packet->RemovePacketTag (satag);

  EpsBearerTag tag;
  bool found = packet->RemovePacketTag (tag);
  NS_ASSERT (found);
  uint16_t rnti = tag.GetRnti ();
  uint8_t bid = tag.GetBid ();
  NS_LOG_LOGIC ("received packet with RNTI=" << (uint32_t) rnti << ", BID=" << (uint32_t)  bid);
  std::map<uint16_t, std::map<uint8_t, uint32_t> >::iterator rntiIt = m_rbidTeidMap.find (rnti);
  if (rntiIt == m_rbidTeidMap.end ())
    {
      NS_LOG_WARN ("UE context not found, discarding packet when receiving from nrSocket");
    }
  else
    {
      std::map<uint8_t, uint32_t>::iterator bidIt = rntiIt->second.find (bid);
      NS_ASSERT (bidIt != rntiIt->second.end ());
      uint32_t teid = bidIt->second;
      SendToN2uSocket (packet, teid);
    }
}

void 
NgcEnbApplication::RecvFromN2uSocket (Ptr<Socket> socket)
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

  std::map<uint32_t, EpsFlowId_t>::iterator it = m_teidRbidMap.find (teid);
  if (it != m_teidRbidMap.end ())
    {
      SendToNrSocket (packet, it->second.m_rnti, it->second.m_bid);
    }
  else
    {
      packet = 0;
      NS_LOG_DEBUG("UE context not found, discarding packet when receiving from n2uSocket");
    }  
}

void 
NgcEnbApplication::SendToNrSocket (Ptr<Packet> packet, uint16_t rnti, uint8_t bid)
{
  NS_LOG_FUNCTION (this << packet << rnti << (uint16_t) bid << packet->GetSize ());  
  EpsBearerTag tag (rnti, bid);
  packet->AddPacketTag (tag);
  int sentBytes = m_nrSocket->Send (packet);
  NS_ASSERT (sentBytes > 0);
}


void 
NgcEnbApplication::SendToN2uSocket (Ptr<Packet> packet, uint32_t teid)
{
  NS_LOG_FUNCTION (this << packet << teid <<  packet->GetSize ());  
  NrGtpuHeader gtpu;
  gtpu.SetTeid (teid);
  // From 3GPP TS 29.281 v10.0.0 Section 5.1
  // Length of the payload + the non obligatory GTP-U header
  gtpu.SetLength (packet->GetSize () + gtpu.GetSerializedSize () - 8);  
  packet->AddHeader (gtpu);
  uint32_t flags = 0;
  m_n2uSocket->SendTo (packet, flags, InetSocketAddress (m_smfN2uAddress, m_gtpuUdpPort));
}

void
NgcEnbApplication::DoReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << bearerId );
  std::list<NgcN2apSapAmf::ErabToBeReleasedIndication> erabToBeReleaseIndication;
  NgcN2apSapAmf::ErabToBeReleasedIndication erab;
  erab.erabId = bearerId;
  erabToBeReleaseIndication.push_back (erab);
  //From 3GPP TS 23401-950 Section 5.4.4.2, enB sends EPS bearer Identity in Bearer Release Indication message to AMF
  m_n2apSapEnbProvider->SendErabReleaseIndication (imsi, rnti, erabToBeReleaseIndication);
}

/* jhlim */
/*
void
NgcEnbApplication::DoReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId, int dummy)
{
  NS_LOG_FUNCTION (this << bearerId );
  std::list<NgcN2apSapAmf::ErabToBeReleasedIndication> erabToBeReleaseIndication;
  NgcN2apSapAmf::ErabToBeReleasedIndication erab;
  erab.erabId = bearerId;
  erabToBeReleaseIndication.push_back (erab);
  //From 3GPP TS 23401-950 Section 5.4.4.2, enB sends EPS bearer Identity in Bearer Release Indication message to AMF
  m_n2apSapAmf->ErabReleaseIndication (imsi, rnti, erabToBeReleaseIndication);
}
*/
}  // namespace ns3
