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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *
 * Modified by Michele Polese <michele.polese@gmail.com>
 *     (support for RRC_CONNECTED->RRC_IDLE state transition + fix for bug 2161
 *      + extension to application & support for real N2AP link)
 */

#include <ns3/fatal-error.h>
#include <ns3/log.h>

#include "ngc-n2ap-sap.h"
#include "ngc-n11-sap.h"

#include "ngc-amf-application.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NgcAmfApplication");

NS_OBJECT_ENSURE_REGISTERED (NgcAmfApplication);

NgcAmfApplication::NgcAmfApplication ()
  : m_n11SapSmf (0)
{
  NS_LOG_FUNCTION (this);
  m_n2apSapAmf = new MemberNgcN2apSapAmf<NgcAmfApplication> (this);
  m_n11SapAmf = new MemberNgcN11SapAmf<NgcAmfApplication> (this);
}


NgcAmfApplication::~NgcAmfApplication ()
{
  NS_LOG_FUNCTION (this);
}

void
NgcAmfApplication::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_n2apSapAmf;
  delete m_n11SapAmf;
}

TypeId
NgcAmfApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcAmfApplication")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcAmfApplication> ()
    ;
  return tid;
}

NgcN2apSapAmf* 
NgcAmfApplication::GetN2apSapAmf ()
{
  return m_n2apSapAmf;
}

void 
NgcAmfApplication::SetN2apSapAmfProvider(NgcN2apSapAmfProvider* provider)
{
  m_n2apSapAmfProvider = provider;
}


void 
NgcAmfApplication::SetN11SapSmf (NgcN11SapSmf * s)
{
  m_n11SapSmf = s;
}

NgcN11SapAmf* 
NgcAmfApplication::GetN11SapAmf ()
{
  return m_n11SapAmf;
}

void 
NgcAmfApplication::AddEnb (uint16_t gci, Ipv4Address enbN2uAddr)
{
  NS_LOG_FUNCTION (this << gci << enbN2uAddr);
  Ptr<EnbInfo> enbInfo = Create<EnbInfo> ();
  enbInfo->gci = gci;
  enbInfo->n2uAddr = enbN2uAddr;
  m_enbInfoMap[gci] = enbInfo;
}

void 
NgcAmfApplication::AddUe (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  Ptr<UeInfo> ueInfo = Create<UeInfo> ();
  ueInfo->imsi = imsi;
  ueInfo->amfUeN2Id = imsi;
///  std::cout << ueInfo->bearersToBeActivated.size() <<"sjkang1021------>" <<std::endl;
  m_ueInfoMap[imsi] = ueInfo;
  ueInfo->bearerCounter = 0;
}

uint8_t
NgcAmfApplication::AddBearer (uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer)
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


/* jhlim */
void
NgcAmfApplication::SendNamfCommunicationUeContextTransfer (uint64_t imsi)
{
	std::cout << "NgcAmfApplication::SendNamfCommunicationUeContextTransfer() - New AMF sends Namf_Communication_UEContextTransfer to Old AMF" << std::endl; 
	DoNamfCommunicationUeContextTransfer (imsi);
}

void
NgcAmfApplication::DoNamfCommunicationUeContextTransfer (uint64_t imsi)
{
	std::cout << "NgcAmfApplication::DoNamfCommunicationUeContextTransfer() - Old AMF does Namf_Communication_UEContextTransfer" << std::endl; 
	uint64_t context = imsi;
	SendNamfCommunicationUeContextTransferResponse(imsi, context);
}

void
NgcAmfApplication::SendNamfCommunicationUeContextTransferResponse (uint64_t imsi, uint64_t context)
{
	std::cout << "NgcAmfApplication::SendNamfCommunicationUeContextTransferResponse() - Old AMF sends Namf_Communication_UEContextTransfer response to New AMF" << std::endl;
	DoNamfCommunicationUeContextTransferResponse (imsi, context);
}

void
NgcAmfApplication::DoNamfCommunicationUeContextTransferResponse (uint64_t imsi, uint64_t context)
{
	std::cout << "NgcAmfApplication::DoNamfCommunicationUeContextTransferResponse() - New AMF does Namf_Communication_UEContextTransfer response" << std::endl;
	// Now new AMF gets a UE's context from the old AMF.
}

void
NgcAmfApplication::SendNamfCommunicationRegistrationCompleteNotify (uint64_t imsi)
{
	std::cout << "NgcAmfAPplication::SendNamfCommunicationRegistrationCompleteNotify() - New AMF sends Namf_Communication_RegistrationCompleteNotify to Old AMF" << std::endl;
}

bool
NgcAmfApplication::IsGuti (uint64_t imsi)
{
	//check whether imsi is GUTI or not.
	return false;
}

// N2-AP SAP AMF forwarded methods
/*
void 
NgcAmfApplication::DoRegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t gci)
{
  NS_LOG_FUNCTION (this << amfUeN2Id << enbUeN2Id << imsi << gci);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  it->second->cellId = gci;
  NgcN11SapSmf::CreateSessionRequestMessage msg;
  msg.imsi = imsi;
  msg.uli.gci = gci;
  //std::cout << "sjkang1021---------->" <<std::endl;

  for (std::list<BearerInfo>::iterator bit = it->second->bearersToBeActivated.begin ();
       bit != it->second->bearersToBeActivated.end ();
       ++bit)
    {
      NgcN11SapSmf::BearerContextToBeCreated bearerContext;
      bearerContext.epsBearerId =  bit->bearerId;
      NS_LOG_INFO("Amf: sending as bearerId " << (uint32_t) bit->bearerId);
      bearerContext.bearerLevelQos = bit->bearer; 
      bearerContext.tft = bit->tft;
      //std::cout << "sjkang1021---------->" <<std::endl;
      msg.bearerContextsToBeCreated.push_back (bearerContext);
    }
  m_n11SapSmf->CreateSessionRequest (msg);
}
*/


/* jhlim: 3. Registration Request
	Receive N2 message (N2 parameters, Registration Request (as in step 1), and UE access selection and PDU session selection information, UE Context request) */
void 
NgcAmfApplication::DoRegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t gci)
{
  //demo log
  std::cout << "NgcAmfApplication::DoRegistrationRequest() - AMF receives registration request from (R)AN" << std::endl;
  NS_LOG_FUNCTION (this << amfUeN2Id << enbUeN2Id << imsi << gci);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  it->second->cellId = gci;
  std::string identityRequest;

  /* jhlim: for the packet transfer */
  NgcN11SapSmf::CreateSessionRequestMessage msg;
  msg.imsi = imsi;
  msg.uli.gci = gci;

  for (std::list<BearerInfo>::iterator bit = it->second->bearersToBeActivated.begin ();
  		bit != it->second->bearersToBeActivated.end ();
		++bit)
	 {
		 NgcN11SapSmf::BearerContextToBeCreated bearerContext;
		 bearerContext.epsBearerId = bit->bearerId;
		 NS_LOG_INFO("Mme: sending as bearerId " << (uint32_t) bit->bearerId);
		 bearerContext.bearerLevelQos = bit->bearer;
		 bearerContext.tft = bit->tft;
		 msg.bearerContextsToBeCreated.push_back (bearerContext);
	 }
	m_n11SapSmf->CreateSessionRequest (msg);

  // Conditional 4-5.
  //if(IsGuti(imsi)) 
  // if GUTI exists, 
  //	NamfCommunicationUeContextTransfer(imsi); // call this to Old AMF.  
  
  // Conditional 6-7. Identity Request message to UE by NAS signal
  // if neither UE nor old AMF send SUCI
   
  identityRequest = "suci";
  //printf("suci IdentityRequest\n");
  m_n2apSapAmfProvider->SendIdentityRequest (amfUeN2Id, enbUeN2Id, it->second->cellId, identityRequest);
  
  // Conditional 8.
  // if old AMF exists,
  // NamfCommunicationRegistrationCompleteNotify(imsi);

  // Conditional 9. (same as 6-7)
  // if PEI is not exists,
  /*
  identityRequest = "pei";
  printf("pei IdentityRequest\n");
  m_n2apSapAmfProvider->SendIdentityRequest (amfUeN2Id, enbUeN2Id, it->second->cellId, identityRequest);
  */
  // 11-12. Registration Accept
  //     (5G-GUTI, Registration Area, PDU Session status, ...)
  ////uint64_t guti = 1; // assign 5G-GUTI for UE
  ////printf("AMF sends registration accept\n");
  ////m_n2apSapAmfProvider->SendRegistrationAccept(amfUeN2Id, enbUeN2Id, it->second->cellId, guti);
}

void
NgcAmfApplication::DoRegistrationComplete (uint64_t amfUeN2Id, uint16_t enbUeN2Id)
{
	NS_LOG_FUNCTION (this);
	NS_LOG_INFO("Registration Compelete.");
	std::cout << "NgcAmfApplication::DoRegistrationComplete() - Registration complete" << std::endl;
}
// jhlim
void 
NgcAmfApplication::DoIdentityResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id)
{
	NS_LOG_FUNCTION (this << amfUeN2Id << enbUeN2Id);
	std::cout << "NgcAmfApplication::DoIdentityResponse() - New AMF does identity response " << std::endl;
}

// hmlee
void
NgcAmfApplication::DoNsmfPDUSessionUpdateSMContext()
{
	NS_LOG_FUNCTION (this);
}

// hmlee
void
NgcAmfApplication::DoNsmfPDUSessionReleaseSMContext()
{

}

void 
NgcAmfApplication::DoN2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t gci)
{
  NS_LOG_FUNCTION (this << amfUeN2Id << enbUeN2Id << imsi << gci);
  std::map<uint64_t, Ptr<UeInfo> >::iterator it = m_ueInfoMap.find (imsi);
  NS_ASSERT_MSG (it != m_ueInfoMap.end (), "could not find any UE with IMSI " << imsi);
  it->second->cellId = gci;
  NgcN11SapSmf::UpdateSMContextRequestMessage msg;
  msg.imsi = imsi;
  msg.uli.gci = gci;
  //std::cout << "sjkang1021---------->" <<std::endl;

  for (std::list<FlowInfo>::iterator bit = it->second->flowsToBeActivated.begin ();
       bit != it->second->flowsToBeActivated.end ();
       ++bit)
    {
      NgcN11SapSmf::N2SMInformationToBeCreated n2SMInformation;
      n2SMInformation.qosFlowId =  bit->flowId;
      NS_LOG_INFO("Amf: sending as bearerId " << (uint32_t) bit->flowId);
      n2SMInformation.flowLevelQos = bit->flow; 
      n2SMInformation.tft = bit->tft;
      //std::cout << "sjkang1021---------->" <<std::endl;
      msg.n2SMInformationToBeCreated.push_back (n2SMInformation);
    }
  m_n11SapSmf->UpdateSMContextRequest (msg);
}

void 
NgcAmfApplication::DoInitialContextSetupResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapAmf::ErabSetupItem> erabSetupList)
{
  NS_LOG_FUNCTION (this << amfUeN2Id << enbUeN2Id);
  NS_FATAL_ERROR ("unimplemented");
}

void 
NgcAmfApplication::DoPathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t gci, std::list<NgcN2apSapAmf::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
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
NgcAmfApplication::DoCreateSessionResponse (NgcN11SapAmf::CreateSessionResponseMessage msg)
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
  m_n2apSapAmfProvider->SendInitialContextSetupRequest (amfUeN2Id, enbUeN2Id, erabToBeSetupList, cellId);
}

//smsohn (DoUpdateSMContextResponse -> SendN2Request)
void 
NgcAmfApplication::DoUpdateSMContextResponse (NgcN11SapAmf::UpdateSMContextResponseMessage msg)
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
  m_n2apSapAmfProvider->SendN2Request (amfUeN2Id, enbUeN2Id, erabToBeSetupList, cellId, cause);
}


void 
NgcAmfApplication::DoModifyBearerResponse (NgcN11SapAmf::ModifyBearerResponseMessage msg)
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
  m_n2apSapAmfProvider->SendPathSwitchRequestAcknowledge (enbUeN2Id, amfUeN2Id, cgi, erabToBeSwitchedInUplinkList);
}

void
NgcAmfApplication::DoErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapAmf::ErabToBeReleasedIndication> erabToBeReleaseIndication)
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
NgcAmfApplication::DoDeleteBearerRequest (NgcN11SapAmf::DeleteBearerRequestMessage msg)
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

void NgcAmfApplication::RemoveBearer (Ptr<UeInfo> ueInfo, uint8_t epsBearerId)
{
  NS_LOG_FUNCTION (this << epsBearerId);
  for (std::list<BearerInfo>::iterator bearerIterator = ueInfo->bearersToBeActivated.begin ();
       bearerIterator != ueInfo->bearersToBeActivated.end ();
       ++bearerIterator)
    {
      if (bearerIterator->bearerId == epsBearerId)
        {
          ueInfo->bearersToBeActivated.erase (bearerIterator);
          ueInfo->bearerCounter = ueInfo->bearerCounter - 1;
          break;
        }
    }
}

} // namespace ns3
