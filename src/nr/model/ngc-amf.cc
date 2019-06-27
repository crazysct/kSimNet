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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */

#include <ns3/fatal-error.h>
#include <ns3/log.h>

#include "ngc-n2ap-sap.h"
#include "ngc-n11-sap.h"

#include "ngc-amf.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NgcAmf");

NS_OBJECT_ENSURE_REGISTERED (NgcAmf);

NgcAmf::NgcAmf ()
  : m_n11SapSmf (0)
{
  NS_LOG_FUNCTION (this);
  m_n2apSapAmf = new MemberNgcN2apSapAmf<NgcAmf> (this);
  m_n11SapAmf = new MemberNgcN11SapAmf<NgcAmf> (this);
}


NgcAmf::~NgcAmf ()
{
  NS_LOG_FUNCTION (this);
}

void
NgcAmf::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_n2apSapAmf;
  delete m_n11SapAmf;
}

TypeId
NgcAmf::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcAmf")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcAmf> ()
    ;
  return tid;
}

NgcN2apSapAmf* 
NgcAmf::GetN2apSapAmf ()
{
  return m_n2apSapAmf;
}

void 
NgcAmf::SetN11SapSmf (NgcN11SapSmf * s)
{
  m_n11SapSmf = s;
}

NgcN11SapAmf* 
NgcAmf::GetN11SapAmf ()
{
  return m_n11SapAmf;
}

void 
NgcAmf::AddEnb (uint16_t gci, Ipv4Address enbN2uAddr, NgcN2apSapEnb* enbN2apSap)
{
  NS_LOG_FUNCTION (this << gci << enbN2uAddr);
  Ptr<EnbInfo> enbInfo = Create<EnbInfo> ();
  enbInfo->gci = gci;
  enbInfo->n2uAddr = enbN2uAddr;
  enbInfo->n2apSapEnb = enbN2apSap;
  m_enbInfoMap[gci] = enbInfo;
}

void 
NgcAmf::AddUe (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  Ptr<UeInfo> ueInfo = Create<UeInfo> ();
  ueInfo->imsi = imsi;
  ueInfo->amfUeN2Id = imsi;
  m_ueInfoMap[imsi] = ueInfo;
  ueInfo->bearerCounter = 0;
}

uint8_t
NgcAmf::AddBearer (uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << imsi);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  NS_ASSERT_MSG (it->second->bearerCounter < 11, "too many bearers already! " << it->second->bearerCounter);
  BearerInfo bearerInfo;
  bearerInfo.bearerId = ++(it->second->bearerCounter);
  bearerInfo.tft = tft;
  bearerInfo.bearer = bearer;  
  it->second->bearersToBeActivated.push_back (bearerInfo);
  return bearerInfo.bearerId;
}


// N2-AP SAP AMF forwarded methods

void 
NgcAmf::DoRegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t gci)
{
  std::cout<<"NgcAmf::DoregistrationRequest (" << amfUeN2Id << ", " << enbUeN2Id << ", " << imsi << ", " << gci << ") is called"<<std::endl;

  NS_LOG_FUNCTION (this << amfUeN2Id << enbUeN2Id << imsi << gci);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  it->second->cellId = gci;
  NgcN11SapSmf::CreateSessionRequestMessage msg;
  msg.imsi = imsi;
  msg.uli.gci = gci;
  for (std::list<BearerInfo>::iterator bit = it->second->bearersToBeActivated.begin ();
       bit != it->second->bearersToBeActivated.end ();
       ++bit)
    {
      NgcN11SapSmf::BearerContextToBeCreated bearerContext;
      bearerContext.epsBearerId =  bit->bearerId; 
      bearerContext.bearerLevelQos = bit->bearer; 
      bearerContext.tft = bit->tft;
      msg.bearerContextsToBeCreated.push_back (bearerContext);
    }
  m_n11SapSmf->CreateSessionRequest (msg);
}

// hmlee
void
NgcAmf::DoNsmfPDUSessionUpdateSMContext ()
{
	// Need implementation in SMF
}

// hmlee
void
NgcAmf::DoNsmfPDUSessionReleaseSMContext ()
{
	// Need implementation in SMF
}
void
NgcAmf::DoN2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t gci)
{
  std::cout<<"NgcAmf::DoN2Message (" << amfUeN2Id << ", " << enbUeN2Id << ", " << imsi << ", " << gci << ") is called"<<std::endl;

  NS_LOG_FUNCTION (this << amfUeN2Id << enbUeN2Id << imsi << gci);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  it->second->cellId = gci;
  NgcN11SapSmf::UpdateSMContextRequestMessage msg;
  msg.imsi = imsi;
  msg.uli.gci = gci;
  for (std::list<BearerInfo>::iterator bit = it->second->bearersToBeActivated.begin ();
       bit != it->second->bearersToBeActivated.end ();
       ++bit)
    {
      NgcN11SapSmf::N2SMInformationToBeCreated n2SMInformation;
      n2SMInformation.qosFlowId =  bit->bearerId; 
      n2SMInformation.flowLevelQos = bit->bearer; 
      n2SMInformation.tft = bit->tft;
      msg.n2SMInformationToBeCreated.push_back (n2SMInformation);
    }
  m_n11SapSmf->UpdateSMContextRequest (msg);
}

void 
NgcAmf::DoInitialContextSetupResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapAmf::ErabSetupItem> erabSetupList)
{
  NS_LOG_FUNCTION (this << amfUeN2Id << enbUeN2Id);
  NS_FATAL_ERROR ("unimplemented");
}

void 
NgcAmf::DoPathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t gci, std::list<NgcN2apSapAmf::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
{
  NS_LOG_FUNCTION (this << amfUeN2Id << enbUeN2Id << gci);

  uint64_t imsi = amfUeN2Id; 
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  NS_LOG_INFO ("IMSI " << imsi << " old eNB: " << it->second->cellId << ", new eNB: " << gci);
  it->second->cellId = gci;
  it->second->enbUeN2Id = enbUeN2Id;

  NgcN11SapSmf::ModifyBearerRequestMessage msg;
  msg.teid = imsi; // trick to avoid the need for allocating TEIDs on the N11 interface
  msg.uli.gci = gci;
  // bearer modification is not supported for now
  m_n11SapSmf->ModifyBearerRequest (msg);
}


// N11 SAP AMF forwarded methods

void 
NgcAmf::DoCreateSessionResponse (NgcN11SapAmf::CreateSessionResponseMessage msg)
{
  NS_LOG_FUNCTION (this << msg.teid);
  uint64_t imsi = msg.teid;
  std::list<NgcN2apSapEnb::ErabToBeSetupItem> erabToBeSetupList;
  for (std::list<NgcN11SapAmf::BearerContextCreated>::iterator bit = msg.bearerContextsCreated.begin ();
       bit != msg.bearerContextsCreated.end ();
       ++bit)
    {
      NgcN2apSapEnb::ErabToBeSetupItem erab;
      erab.erabId = bit->epsBearerId;
      erab.erabLevelQosParameters = bit->bearerLevelQos;
      erab.transportLayerAddress = bit->smfFteid.address;
      erab.smfTeid = bit->smfFteid.teid;      
      erabToBeSetupList.push_back (erab);
    }
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  uint16_t cellId = it->second->cellId;
  uint16_t enbUeN2Id = it->second->enbUeN2Id;
  uint64_t amfUeN2Id = it->second->amfUeN2Id;
  std::map<uint16_t, Ptr<EnbInfo> >::iterator jt = m_enbInfoMap.find (cellId);
  NS_ASSERT_MSG (jt != m_enbInfoMap.end (), "could not find any eNB with CellId " << cellId);
  jt->second->n2apSapEnb->InitialContextSetupRequest (amfUeN2Id, enbUeN2Id, erabToBeSetupList);
}

//smsohn : DoUpdateSMContextResponse -> N2Request 
void 
NgcAmf::DoUpdateSMContextResponse (NgcN11SapAmf::UpdateSMContextResponseMessage msg)
{
  NS_LOG_FUNCTION (this << msg.teid);
  uint64_t imsi = msg.teid;
  std::list<NgcN2apSapEnb::ErabToBeSetupItem> erabToBeSetupList;
  for (std::list<NgcN11SapAmf::N2SMInformationCreated>::iterator bit = msg.n2SMInformationCreated.begin ();
       bit != msg.n2SMInformationCreated.end ();
       ++bit)
    {
      NgcN2apSapEnb::ErabToBeSetupItem erab;
      erab.erabId = bit->qosFlowId;
      erab.erabLevelQosParameters = bit->flowLevelQos;
      erab.transportLayerAddress = bit->smfFteid.address;
      erab.smfTeid = bit->smfFteid.teid;      
      erabToBeSetupList.push_back (erab);
    }
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  uint16_t cellId = it->second->cellId;
  uint16_t enbUeN2Id = it->second->enbUeN2Id;
  uint64_t amfUeN2Id = it->second->amfUeN2Id;
  std::map<uint16_t, Ptr<EnbInfo> >::iterator jt = m_enbInfoMap.find (cellId);
  NS_ASSERT_MSG (jt != m_enbInfoMap.end (), "could not find any eNB with CellId " << cellId);

  uint16_t cause = 0; //smsohn
  jt->second->n2apSapEnb->N2Request (amfUeN2Id, enbUeN2Id, erabToBeSetupList, cause);
}


void 
NgcAmf::DoModifyBearerResponse (NgcN11SapAmf::ModifyBearerResponseMessage msg)
{
  NS_LOG_FUNCTION (this << msg.teid);
  NS_ASSERT (msg.cause == NgcN11SapAmf::ModifyBearerResponseMessage::REQUEST_ACCEPTED);
  uint64_t imsi = msg.teid;
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  uint64_t enbUeN2Id = it->second->enbUeN2Id;
  uint64_t amfUeN2Id = it->second->amfUeN2Id;
  uint16_t cgi = it->second->cellId;
  std::list<NgcN2apSapEnb::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList; // unused for now
  std::map<uint16_t, Ptr<EnbInfo> >::iterator jt = m_enbInfoMap.find (it->second->cellId);
  NS_ASSERT_MSG (jt != m_enbInfoMap.end (), "could not find any eNB with CellId " << it->second->cellId);
  jt->second->n2apSapEnb->PathSwitchRequestAcknowledge (enbUeN2Id, amfUeN2Id, cgi, erabToBeSwitchedInUplinkList);
}

void
NgcAmf::DoErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapAmf::ErabToBeReleasedIndication> erabToBeReleaseIndication)
{
  NS_LOG_FUNCTION (this << amfUeN2Id << enbUeN2Id);
  uint64_t imsi = amfUeN2Id;
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);

  NgcN11SapSmf::DeleteBearerCommandMessage msg;
  // trick to avoid the need for allocating TEIDs on the N11 interface
  msg.teid = imsi;

  for (std::list<NgcN2apSapAmf::ErabToBeReleasedIndication>::iterator bit = erabToBeReleaseIndication.begin (); bit != erabToBeReleaseIndication.end (); ++bit)
    {
      NgcN11SapSmf::BearerContextToBeRemoved bearerContext;
      bearerContext.epsBearerId =  bit->erabId;
      msg.bearerContextsToBeRemoved.push_back (bearerContext);
    }
  //Delete Bearer command towards ngc-smf-upf-application
  m_n11SapSmf->DeleteBearerCommand (msg);
}

void
NgcAmf::DoDeleteBearerRequest (NgcN11SapAmf::DeleteBearerRequestMessage msg)
{
  NS_LOG_FUNCTION (this);
  uint64_t imsi = msg.teid;
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  NgcN11SapSmf::DeleteBearerResponseMessage res;

  res.teid = imsi;

  for (std::list<NgcN11SapAmf::BearerContextRemoved>::iterator bit = msg.bearerContextsRemoved.begin ();
       bit != msg.bearerContextsRemoved.end ();
       ++bit)
    {
      NgcN11SapSmf::BearerContextRemovedSmfUpf bearerContext;
      bearerContext.epsBearerId = bit->epsBearerId;
      res.bearerContextsRemoved.push_back (bearerContext);

      RemoveBearer (it->second, bearerContext.epsBearerId); //schedules function to erase, context of de-activated bearer
    }
  //schedules Delete Bearer Response towards ngc-smf-upf-application
  m_n11SapSmf->DeleteBearerResponse (res);
}

void NgcAmf::RemoveBearer (Ptr<UeInfo> ueInfo, uint8_t epsBearerId)
{
  NS_LOG_FUNCTION (this << epsBearerId);
  for (std::list<BearerInfo>::iterator bit = ueInfo->bearersToBeActivated.begin ();
       bit != ueInfo->bearersToBeActivated.end ();
       ++bit)
    {
      if (bit->bearerId == epsBearerId)
        {
          ueInfo->bearersToBeActivated.erase (bit);
          break;
        }
    }
}

// jhlim
void
NgcAmf::DoIdentityRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId)
{
  NS_LOG_FUNCTION (this);
}
void
NgcAmf::DoIdentityResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id)
{
  NS_LOG_FUNCTION (this);
}
void
NgcAmf::DoRegistrationAccept (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId, uint64_t guti)
{
  NS_LOG_FUNCTION (this);
}
void
NgcAmf::DoRegistrationComplete (uint64_t amfUeN2Id, uint16_t enbUeN2Id)
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
