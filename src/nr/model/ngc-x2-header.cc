/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#include "ns3/log.h"
#include "ns3/ngc-x2-header.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NgcX2Header");

NS_OBJECT_ENSURE_REGISTERED (NgcX2Header);

NgcX2Header::NgcX2Header ()
  : m_messageType (0xfa),
    m_procedureCode (0xfa),
    m_lengthOfIes (0xfa),
    m_numberOfIes (0xfa)
{
}

NgcX2Header::~NgcX2Header ()
{
  m_messageType = 0xfb;
  m_procedureCode = 0xfb;
  m_lengthOfIes = 0xfb;
  m_numberOfIes = 0xfb;
}

TypeId
NgcX2Header::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2Header")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2Header> ()
  ;
  return tid;
}

TypeId
NgcX2Header::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2Header::GetSerializedSize (void) const
{
  return 7;
}

void
NgcX2Header::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 (m_messageType);
  i.WriteU8 (m_procedureCode);

  i.WriteU8 (0x00); // criticality = REJECT
  i.WriteU8 (m_lengthOfIes + 3);
  i.WriteHtonU16 (0);
  i.WriteU8 (m_numberOfIes);
}

uint32_t
NgcX2Header::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_messageType = i.ReadU8 ();
  m_procedureCode = i.ReadU8 ();

  i.ReadU8 ();
  m_lengthOfIes = i.ReadU8 () - 3;
  i.ReadNtohU16 ();
  m_numberOfIes = i.ReadU8 ();
  
  return GetSerializedSize ();
}

void
NgcX2Header::Print (std::ostream &os) const
{
  os << "MessageType=" << (uint32_t) m_messageType;
  os << " ProcedureCode=" << (uint32_t) m_procedureCode;
  os << " LengthOfIEs=" << (uint32_t) m_lengthOfIes;
  os << " NumberOfIEs=" << (uint32_t) m_numberOfIes;
}

uint8_t
NgcX2Header::GetMessageType () const
{
  return m_messageType;
}

void
NgcX2Header::SetMessageType (uint8_t messageType)
{
  m_messageType = messageType;
}

uint8_t
NgcX2Header::GetProcedureCode () const
{
  return m_procedureCode;
}

void
NgcX2Header::SetProcedureCode (uint8_t procedureCode)
{
  m_procedureCode = procedureCode;
}


void
NgcX2Header::SetLengthOfIes (uint32_t lengthOfIes)
{
  m_lengthOfIes = lengthOfIes;
}

void
NgcX2Header::SetNumberOfIes (uint32_t numberOfIes)
{
  m_numberOfIes = numberOfIes;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED(NgcX2AssistantInfoHeader); //sjkang1114
NgcX2AssistantInfoHeader::NgcX2AssistantInfoHeader(){
	TxonQueueSize=0xfffffffa;
	TxedQueueSize=0xfffffffa;
	ReTxQueingDelay=0xfffffffa;
	RetxQueueSize=0xfffffffa;
	TxQueingDelay=0xfffffffa;
	sourceCellId =0xfa;
	rnti=0xfa;
	drbId=0xfa;
	headerLength=23;
}
NgcX2AssistantInfoHeader::~NgcX2AssistantInfoHeader(){
	TxonQueueSize=0xfffffffb;
		TxedQueueSize=0xfffffffb;
		ReTxQueingDelay=0xfffffffb;
		RetxQueueSize=0xfffffffb;
		TxQueingDelay=0xfffffffb;
		sourceCellId=0xfb;
		rnti=0xfb;
		drbId=0xfb;
		headerLength=0;

}
TypeId
NgcX2AssistantInfoHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2AssistantInfoHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2AssistantInfoHeader> ()
  ;
  return tid;
}
TypeId
NgcX2AssistantInfoHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
uint32_t
NgcX2AssistantInfoHeader::GetSerializedSize()const{
return headerLength;
}
void
NgcX2AssistantInfoHeader::Serialize(Buffer::Iterator start)const {
	Buffer::Iterator i = start;
	 i.WriteHtonU32(TxedQueueSize);
	 i.WriteHtonU32(RetxQueueSize);
	 i.WriteHtonU32(TxonQueueSize);
	 i.WriteHtonU32(TxQueingDelay);
	 i.WriteHtonU32(ReTxQueingDelay);
	 i.WriteU8(sourceCellId);
	 i.WriteU8(rnti);
	 i.WriteU8(drbId);

}
uint32_t
NgcX2AssistantInfoHeader::Deserialize(Buffer::Iterator start){
	Buffer::Iterator i =start;
	TxedQueueSize =i.ReadNtohU32();
	RetxQueueSize = i.ReadNtohU32();
	TxonQueueSize= i.ReadNtohU32();
	TxQueingDelay = i.ReadNtohU32();
	ReTxQueingDelay= i.ReadNtohU32();
	sourceCellId = i.ReadU8();
	rnti=i.ReadU8();
	drbId=i.ReadU8();
	headerLength =23;
	return GetSerializedSize();
}
void
NgcX2AssistantInfoHeader::Print(std::ostream &os) const
{
	os << "TxedQ = " << TxedQueueSize;
	os<< "RetxQ = " << RetxQueueSize ;
	os<<"TxonQ = " << TxonQueueSize;
	os<<"TxQ_delay = "<<TxQueingDelay ;
	os<< "ReTxQ_delay = " << ReTxQueingDelay ;
	os<<"source Cell ID = " << sourceCellId ;
}
uint32_t
NgcX2AssistantInfoHeader::GetTxonQueue()const{
	return TxonQueueSize;
}
void
NgcX2AssistantInfoHeader::SetTxonQueue(uint32_t txq){
	TxonQueueSize=txq;
}
uint32_t
NgcX2AssistantInfoHeader::GetRetxQueue()const{
	return RetxQueueSize;
}
void
NgcX2AssistantInfoHeader::SetRetxQueue(uint32_t retxq){
	RetxQueueSize=retxq;
}
uint32_t
NgcX2AssistantInfoHeader::GetTxedQueue()const{
	return TxedQueueSize;
}
void
NgcX2AssistantInfoHeader::SetTxedQueue(uint32_t txedq){
	TxedQueueSize=txedq;
}
uint32_t
NgcX2AssistantInfoHeader::GetTxonQueingDelay() const {
	return TxQueingDelay;
}
void
NgcX2AssistantInfoHeader::SetTxonQueingDelay(uint32_t ququingdelay){
	TxQueingDelay = ququingdelay;
}
uint32_t
NgcX2AssistantInfoHeader::GetReTxQueuingDelay() const {
	return ReTxQueingDelay;
}
void
NgcX2AssistantInfoHeader::SetReTxQueingDelay(uint32_t retxququingdelay){
	ReTxQueingDelay = retxququingdelay;
}
uint8_t
NgcX2AssistantInfoHeader::GetSourceCellId() const {
	return sourceCellId;
}
void
NgcX2AssistantInfoHeader::SetSourceCellId(uint8_t cellId){
	sourceCellId = cellId;
}
void
NgcX2AssistantInfoHeader::SetRnti(uint8_t rnti){
	this->rnti =rnti;
}
uint8_t
NgcX2AssistantInfoHeader::GetRnti(){
	return rnti;
}
void
NgcX2AssistantInfoHeader::SetDrbId(uint8_t drbId){
	this->drbId = drbId;
}
uint8_t
NgcX2AssistantInfoHeader::GetDrbId(){
	return drbId;
}
uint16_t
NgcX2AssistantInfoHeader::GetNumbefOfIes(){
	return 8;
}
/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2HandoverRequestHeader);

NgcX2HandoverRequestHeader::NgcX2HandoverRequestHeader ()
  : m_numberOfIes (1 + 1 + 1 + 1 + 1 + 1),
    m_headerLength (6 + 5 + 12 + (3 + 4 + 8 + 8 + 4) + 1 + 4),
    m_oldEnbUeX2apId (0xfffa),
 //   m_cause (0xfffa),
    m_targetCellId (0xfffa),
//	m_nrCellId(0xfffa),
    m_amfUeN2apId (0xfffffffa),
    m_isMc (0xfa),
	m_isMc_2(0xfa)
{
  m_erabsToBeSetupList.clear ();
}

NgcX2HandoverRequestHeader::~NgcX2HandoverRequestHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_oldEnbUeX2apId = 0xfffb;
  m_cause = 0xfffb;
  m_targetCellId = 0xfffb;
//  m_nrCellId = 0xfffb; //sjkang
  m_amfUeN2apId = 0xfffffffb;
  m_isMc = 0xfb;
  m_isMc_2 = 0xfb;
  m_erabsToBeSetupList.clear ();
  m_rlcRequestsList.clear();
}

TypeId
NgcX2HandoverRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2HandoverRequestHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2HandoverRequestHeader> ()
  ;
  return tid;
}

TypeId
NgcX2HandoverRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2HandoverRequestHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2HandoverRequestHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (10);              // id = OLD_ENB_UE_X2AP_ID
  i.WriteU8 (0);                    // criticality = REJECT
  i.WriteU8 (2);                    // length of OLD_ENB_UE_X2AP_ID
  i.WriteHtonU16 (m_oldEnbUeX2apId);

  i.WriteHtonU16 (5);               // id = CAUSE
  i.WriteU8 (1 << 6);               // criticality = IGNORE
  i.WriteU8 (1);                    // length of CAUSE
  i.WriteU8 (m_cause);

  i.WriteHtonU16 (11);              // id = TARGET_CELLID
  i.WriteU8 (0);                    // criticality = REJECT
  i.WriteU8 (8);                    // length of TARGET_CELLID
  i.WriteHtonU32 (0x123456);        // fake PLMN
  i.WriteHtonU32 (m_targetCellId << 4);


  i.WriteHtonU16 (14);              // id = UE_CONTEXT_INFORMATION
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteHtonU32 (m_amfUeN2apId);
  i.WriteHtonU64 (m_ueAggregateMaxBitRateDownlink);
  i.WriteHtonU64 (m_ueAggregateMaxBitRateUplink);

  std::vector <NgcX2Sap::ErabToBeSetupItem>::size_type sz = m_erabsToBeSetupList.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (int j = 0; j < (int) sz; j++)
    {
      i.WriteHtonU16 (m_erabsToBeSetupList [j].erabId);
      i.WriteHtonU16 (m_erabsToBeSetupList [j].erabLevelQosParameters.qci);
      i.WriteHtonU64 (m_erabsToBeSetupList [j].erabLevelQosParameters.gbrQosInfo.gbrDl);
      i.WriteHtonU64 (m_erabsToBeSetupList [j].erabLevelQosParameters.gbrQosInfo.gbrUl);
      i.WriteHtonU64 (m_erabsToBeSetupList [j].erabLevelQosParameters.gbrQosInfo.mbrDl);
      i.WriteHtonU64 (m_erabsToBeSetupList [j].erabLevelQosParameters.gbrQosInfo.mbrUl);
      i.WriteU8 (m_erabsToBeSetupList [j].erabLevelQosParameters.arp.priorityLevel);
      i.WriteU8 (m_erabsToBeSetupList [j].erabLevelQosParameters.arp.preemptionCapability);
      i.WriteU8 (m_erabsToBeSetupList [j].erabLevelQosParameters.arp.preemptionVulnerability);
      i.WriteU8 (m_erabsToBeSetupList [j].dlForwarding);
      i.WriteHtonU32 (m_erabsToBeSetupList [j].transportLayerAddress.Get ());
      i.WriteHtonU32 (m_erabsToBeSetupList [j].gtpTeid);
    }

  // RlcSteupRequest vector - for secondary cell HO
  std::vector <NgcX2Sap::RlcSetupRequest>::size_type sz_rlc = m_rlcRequestsList.size (); 
  i.WriteHtonU32 (sz_rlc);              // number of RLCs to be setup
  for (int j = 0; j < (int) sz_rlc; j++)
  {
    i.WriteHtonU16 (m_rlcRequestsList[j].sourceCellId);
    i.WriteHtonU16 (m_rlcRequestsList[j].targetCellId); 
    i.WriteHtonU32 (m_rlcRequestsList[j].gtpTeid); 
    i.WriteHtonU16 (m_rlcRequestsList[j].mmWaveRnti); 
    i.WriteHtonU16 (m_rlcRequestsList[j].nrRnti);
    i.WriteU8 (m_rlcRequestsList[j].drbid);

    // LcInfo
    i.WriteHtonU16  (m_rlcRequestsList[j].lcinfo.rnti); // TODO consider if unnecessary
    i.WriteU8       (m_rlcRequestsList[j].lcinfo.lcId);
    i.WriteU8       (m_rlcRequestsList[j].lcinfo.lcGroup);
    i.WriteU8       (m_rlcRequestsList[j].lcinfo.qci);
    i.WriteU8       (m_rlcRequestsList[j].lcinfo.isGbr);
    i.WriteHtonU64  (m_rlcRequestsList[j].lcinfo.mbrUl);
    i.WriteHtonU64  (m_rlcRequestsList[j].lcinfo.mbrDl);
    i.WriteHtonU64  (m_rlcRequestsList[j].lcinfo.gbrUl);
    i.WriteHtonU64  (m_rlcRequestsList[j].lcinfo.gbrDl);

    // RlcConfig
    i.WriteHtonU32 (m_rlcRequestsList[j].rlcConfig.choice); // TODO check size

    // LogicalChannelConfiguration
    i.WriteU8      (m_rlcRequestsList[j].logicalChannelConfig.priority);
    i.WriteHtonU16 (m_rlcRequestsList[j].logicalChannelConfig.prioritizedBitRateKbps);
    i.WriteHtonU16 (m_rlcRequestsList[j].logicalChannelConfig.bucketSizeDurationMs);
    i.WriteU8      (m_rlcRequestsList[j].logicalChannelConfig.logicalChannelGroup);
  }

  i.WriteU8(m_isMc);
  i.WriteU8(m_isMc_2); //sjkang
 // i.WriteU8(m_nrCellId); //sjkang0416
}

uint32_t
NgcX2HandoverRequestHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  i.ReadNtohU16 ();
  i.ReadU8 ();
  i.ReadU8 ();
  m_oldEnbUeX2apId = i.ReadNtohU16 ();
  m_headerLength += 6;
  m_numberOfIes++;

  i.ReadNtohU16 ();
  i.ReadU8 ();
  i.ReadU8 ();
  m_cause = i.ReadU8 ();
  m_headerLength += 5;
  m_numberOfIes++;
  
  i.ReadNtohU16 ();
  i.ReadU8 ();
  i.ReadU8 ();
  i.ReadNtohU32 ();
  m_targetCellId = i.ReadNtohU32 () >> 4;
  m_headerLength += 12;
  m_numberOfIes++;

  i.ReadNtohU16 ();
  i.ReadU8 ();
  m_amfUeN2apId = i.ReadNtohU32 ();
  m_ueAggregateMaxBitRateDownlink = i.ReadNtohU64 ();
  m_ueAggregateMaxBitRateUplink   = i.ReadNtohU64 ();
  int sz = i.ReadNtohU32 ();
  m_headerLength += 27;
  m_numberOfIes++;

  for (int j = 0; j < sz; j++)
    {
      NgcX2Sap::ErabToBeSetupItem erabItem;

      erabItem.erabId = i.ReadNtohU16 ();
 
      erabItem.erabLevelQosParameters = EpsBearer ((EpsBearer::Qci) i.ReadNtohU16 ());
      erabItem.erabLevelQosParameters.gbrQosInfo.gbrDl = i.ReadNtohU64 ();
      erabItem.erabLevelQosParameters.gbrQosInfo.gbrUl = i.ReadNtohU64 ();
      erabItem.erabLevelQosParameters.gbrQosInfo.mbrDl = i.ReadNtohU64 ();
      erabItem.erabLevelQosParameters.gbrQosInfo.mbrUl = i.ReadNtohU64 ();
      erabItem.erabLevelQosParameters.arp.priorityLevel = i.ReadU8 ();
      erabItem.erabLevelQosParameters.arp.preemptionCapability = i.ReadU8 ();
      erabItem.erabLevelQosParameters.arp.preemptionVulnerability = i.ReadU8 ();

      erabItem.dlForwarding = i.ReadU8 ();
      erabItem.transportLayerAddress = Ipv4Address (i.ReadNtohU32 ());
      erabItem.gtpTeid = i.ReadNtohU32 ();

      m_erabsToBeSetupList.push_back (erabItem);
      m_headerLength += 48;
    }

  sz = i.ReadNtohU32 ();  
 
  for (int j = 0; j < sz; j++)
  {
    NgcX2Sap::RlcSetupRequest rlcReq;

    rlcReq.sourceCellId = i.ReadNtohU16 ();
    rlcReq.targetCellId = i.ReadNtohU16 (); 
    rlcReq.gtpTeid = i.ReadNtohU32 (); 
    rlcReq.mmWaveRnti = i.ReadNtohU16 (); 
    rlcReq.nrRnti = i.ReadNtohU16 ();
    rlcReq.drbid = i.ReadU8 ();

    // LcInfo
    rlcReq.lcinfo.rnti = i.ReadNtohU16 (); // TODO consider if unnecessary
    rlcReq.lcinfo.lcId = i.ReadU8      ();
    rlcReq.lcinfo.lcGroup = i.ReadU8   ();
    rlcReq.lcinfo.qci = i.ReadU8       ();
    rlcReq.lcinfo.isGbr = i.ReadU8     ();
    rlcReq.lcinfo.mbrUl = i.ReadNtohU64();
    rlcReq.lcinfo.mbrDl = i.ReadNtohU64();
    rlcReq.lcinfo.gbrUl = i.ReadNtohU64();
    rlcReq.lcinfo.gbrDl = i.ReadNtohU64();

    // RlcConfig
    uint32_t val = i.ReadNtohU32 ();
    if (val == NrRrcSap::RlcConfig::AM) {
      rlcReq.rlcConfig.choice = NrRrcSap::RlcConfig::AM;
    }
    else if (val == NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL) {
      rlcReq.rlcConfig.choice = NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL;
    }
    else if (val == NrRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_UL) {
      rlcReq.rlcConfig.choice = NrRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_UL;
    }
    else if (val == NrRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_DL) {
      rlcReq.rlcConfig.choice = NrRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_DL;
    }
    else if (val == NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL_LOWLAT) {
      rlcReq.rlcConfig.choice = NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL_LOWLAT;
    }
    else {
      NS_FATAL_ERROR("Unknown value for RlcConfig " << val);
    }

    // LogicalChannelConfiguration
    rlcReq.logicalChannelConfig.priority = i.ReadU8     ();
    rlcReq.logicalChannelConfig.prioritizedBitRateKbps = i.ReadNtohU16();
    rlcReq.logicalChannelConfig.bucketSizeDurationMs = i.ReadNtohU16();
    rlcReq.logicalChannelConfig.logicalChannelGroup = i.ReadU8     ();

    m_rlcRequestsList.push_back(rlcReq);
    m_headerLength += 61;
  }

  m_isMc = i.ReadU8();
  m_numberOfIes++;
  m_headerLength++;
  m_isMc_2 = i.ReadU8();  //sjkang
  m_numberOfIes++;
   m_headerLength++;
  // m_nrCellId = i.ReadU8();
//   m_numberOfIes++;
//   m_headerLength++;
  return GetSerializedSize ();
}

void
NgcX2HandoverRequestHeader::Print (std::ostream &os) const
{
  os << "OldEnbUeX2apId = " << m_oldEnbUeX2apId;
  os << " Cause = " << m_cause;
  os << " TargetCellId = " << m_targetCellId;
  os << " AmfUeN2apId = " << m_amfUeN2apId;
  os << " UeAggrMaxBitRateDownlink = " << m_ueAggregateMaxBitRateDownlink;
  os << " UeAggrMaxBitRateUplink = " << m_ueAggregateMaxBitRateUplink;
  os << " NumOfBearers = " << m_erabsToBeSetupList.size ();
  os << " NumOfRlcRequests = " << m_rlcRequestsList.size ();
  os << " isMc = " << m_isMc;
  os << "isMc_2 =" << m_isMc_2;
  //os<<"nr cell ID" << m_nrCellId; //sjkang

  std::vector <NgcX2Sap::ErabToBeSetupItem>::size_type sz = m_erabsToBeSetupList.size ();
  if (sz > 0)
    {
      os << " [";
    }
  for (int j = 0; j < (int) sz; j++)
    {
      os << m_erabsToBeSetupList[j].erabId;
      if (j < (int) sz - 1)
        {
          os << ", ";
        }
      else
        {
          os << "]";
        }
    }

}

uint16_t
NgcX2HandoverRequestHeader::GetOldEnbUeX2apId () const
{
  return m_oldEnbUeX2apId;
}

void
NgcX2HandoverRequestHeader::SetOldEnbUeX2apId (uint16_t x2apId)
{
  m_oldEnbUeX2apId = x2apId;
}

uint16_t
NgcX2HandoverRequestHeader::GetCause () const
{
  return m_cause;
}

void
NgcX2HandoverRequestHeader::SetCause (uint16_t cause)
{
  m_cause = cause;
}

bool
NgcX2HandoverRequestHeader::GetIsMc () const
{
  return m_isMc;
}

void
NgcX2HandoverRequestHeader::SetIsMc (bool isMc)
{
  m_isMc = isMc;
}
bool
NgcX2HandoverRequestHeader::GetIsMc_2 () const
{
  return m_isMc_2;
}

void
NgcX2HandoverRequestHeader::SetIsMc_2 (bool isMc)
{
  m_isMc_2 = isMc;
}
uint16_t
NgcX2HandoverRequestHeader::GetTargetCellId () const
{
  return m_targetCellId;
}

void
NgcX2HandoverRequestHeader::SetTargetCellId (uint16_t targetCellId)
{
  m_targetCellId = targetCellId;
}

uint32_t
NgcX2HandoverRequestHeader::GetAmfUeN2apId () const
{
  return m_amfUeN2apId;
}

void
NgcX2HandoverRequestHeader::SetAmfUeN2apId (uint32_t amfUeN2apId)
{
  m_amfUeN2apId = amfUeN2apId;
}

std::vector <NgcX2Sap::RlcSetupRequest>
NgcX2HandoverRequestHeader::GetRlcSetupRequests () const
{
  return m_rlcRequestsList;
}

void
NgcX2HandoverRequestHeader::SetRlcSetupRequests (std::vector <NgcX2Sap::RlcSetupRequest> rlcRequests)
{
  m_headerLength += 61 * rlcRequests.size ();
  m_rlcRequestsList = rlcRequests;
}

std::vector <NgcX2Sap::ErabToBeSetupItem>
NgcX2HandoverRequestHeader::GetBearers () const
{
  return m_erabsToBeSetupList;
}

void
NgcX2HandoverRequestHeader::SetBearers (std::vector <NgcX2Sap::ErabToBeSetupItem> bearers)
{
  m_headerLength += 48 * bearers.size ();
  m_erabsToBeSetupList = bearers;
}

uint64_t
NgcX2HandoverRequestHeader::GetUeAggregateMaxBitRateDownlink () const
{
  return m_ueAggregateMaxBitRateDownlink;
}

void
NgcX2HandoverRequestHeader::SetUeAggregateMaxBitRateDownlink (uint64_t bitRate)
{
  m_ueAggregateMaxBitRateDownlink = bitRate;
}

uint64_t
NgcX2HandoverRequestHeader::GetUeAggregateMaxBitRateUplink () const
{
  return m_ueAggregateMaxBitRateUplink;
}

void
NgcX2HandoverRequestHeader::SetUeAggregateMaxBitRateUplink (uint64_t bitRate)
{
  m_ueAggregateMaxBitRateUplink = bitRate;
}

uint32_t
NgcX2HandoverRequestHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2HandoverRequestHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2RlcSetupRequestHeader);

NgcX2RlcSetupRequestHeader::NgcX2RlcSetupRequestHeader ()
  : m_numberOfIes (1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1),
    m_headerLength (2 + 2 + 4 + 2 + 2 + 1 + 38 + 4 + 6),
    m_sourceCellId (0xfffa),
    m_targetCellId (0xfffa),
    m_gtpTeid (0xfffffffa),
    m_mmWaveRnti (0xfffa),
    m_nrRnti (0xfffa),
    m_drbid (0xfa)
{
}

NgcX2RlcSetupRequestHeader::~NgcX2RlcSetupRequestHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_sourceCellId = 0xfffb;
  m_targetCellId = 0xfffb;
  m_gtpTeid = 0xfffffffb;
  m_mmWaveRnti = 0xfffb;
  m_nrRnti = 0xfffb;
  m_drbid = 0xfb;
}

TypeId
NgcX2RlcSetupRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2RlcSetupRequestHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2RlcSetupRequestHeader> ()
  ;
  return tid;
}

TypeId
NgcX2RlcSetupRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2RlcSetupRequestHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2RlcSetupRequestHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_sourceCellId);
  i.WriteHtonU16 (m_targetCellId); 
  i.WriteHtonU32 (m_gtpTeid); 
  i.WriteHtonU16 (m_mmWaveRnti); 
  i.WriteHtonU16 (m_nrRnti);
  i.WriteU8 (m_drbid);

  // LcInfo
  i.WriteHtonU16 (m_lcInfo.rnti); // TODO consider if unnecessary
  i.WriteU8 (m_lcInfo.lcId);
  i.WriteU8 (m_lcInfo.lcGroup);
  i.WriteU8 (m_lcInfo.qci);
  i.WriteU8 (m_lcInfo.isGbr);
  i.WriteHtonU64 (m_lcInfo.mbrUl);
  i.WriteHtonU64 (m_lcInfo.mbrDl);
  i.WriteHtonU64 (m_lcInfo.gbrUl);
  i.WriteHtonU64 (m_lcInfo.gbrDl);

  // RlcConfig
  i.WriteHtonU32 (m_rlcConfig.choice); // TODO check size

  // LogicalChannelConfiguration
  i.WriteU8 (m_lcConfig.priority);
  i.WriteHtonU16 (m_lcConfig.prioritizedBitRateKbps);
  i.WriteHtonU16 (m_lcConfig.bucketSizeDurationMs);
  i.WriteU8 (m_lcConfig.logicalChannelGroup);
}

uint32_t
NgcX2RlcSetupRequestHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_sourceCellId = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;
  
  m_targetCellId = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;

  m_gtpTeid = i.ReadNtohU32 ();
  m_headerLength += 4;
  m_numberOfIes++;

  m_mmWaveRnti = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;

  m_nrRnti = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;

  m_drbid = i.ReadU8 ();
  m_headerLength += 1;
  m_numberOfIes++;

  m_lcInfo.rnti = i.ReadNtohU16 (); // TODO consider if unnecessary
  m_lcInfo.lcId = i.ReadU8 ();
  m_lcInfo.lcGroup = i.ReadU8 ();
  m_lcInfo.qci = i.ReadU8 ();
  m_lcInfo.isGbr = i.ReadU8 ();
  m_lcInfo.mbrUl = i.ReadNtohU64 ();
  m_lcInfo.mbrDl = i.ReadNtohU64 ();
  m_lcInfo.gbrUl = i.ReadNtohU64 ();
  m_lcInfo.gbrDl = i.ReadNtohU64 ();
  m_headerLength += 38;
  m_numberOfIes++;

  uint32_t val = i.ReadNtohU32 ();
  if (val == NrRrcSap::RlcConfig::AM) {
    m_rlcConfig.choice = NrRrcSap::RlcConfig::AM;
  }
  else if (val == NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL) {
    m_rlcConfig.choice = NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL;
  }
  else if (val == NrRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_UL) {
    m_rlcConfig.choice = NrRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_UL;
  }
  else if (val == NrRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_DL) {
    m_rlcConfig.choice = NrRrcSap::RlcConfig::UM_UNI_DIRECTIONAL_DL;
  }
  else if (val == NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL_LOWLAT) {
    m_rlcConfig.choice = NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL_LOWLAT;
  }
  else {
    NS_FATAL_ERROR("Unknown value for RlcConfig " << val);
  }
  m_headerLength += 4;
  m_numberOfIes++;

  // LogicalChannelConfiguration
  m_lcConfig.priority               = i.ReadU8      ();
  m_lcConfig.prioritizedBitRateKbps = i.ReadNtohU16 ();
  m_lcConfig.bucketSizeDurationMs   = i.ReadNtohU16 ();
  m_lcConfig.logicalChannelGroup    = i.ReadU8      ();
  m_headerLength += 6;
  m_numberOfIes++;

  return GetSerializedSize ();
}

void
NgcX2RlcSetupRequestHeader::Print (std::ostream &os) const
{
  os << "SourceCellId = " << m_sourceCellId;
  os << " TargetCellId = " << m_targetCellId;
  os << " gtpTeid = " << m_gtpTeid;
  os << " MmWaveRnti = " << m_mmWaveRnti;
  os << " NrRnti = " << m_nrRnti;
  os << " DrbId = " << (uint32_t)m_drbid;
  os << " RlcConfig " << m_rlcConfig.choice;
  os << " bucketSizeDurationMs " << m_lcConfig.bucketSizeDurationMs;
  // TODO complete print
}

uint16_t
NgcX2RlcSetupRequestHeader::GetSourceCellId () const
{
  return m_sourceCellId;
}

void
NgcX2RlcSetupRequestHeader::SetSourceCellId (uint16_t cellId)
{
  m_sourceCellId = cellId;
}

uint16_t
NgcX2RlcSetupRequestHeader::GetTargetCellId () const
{
  return m_targetCellId;
}

void
NgcX2RlcSetupRequestHeader::SetTargetCellId (uint16_t targetCellId)
{
  m_targetCellId = targetCellId;
}

uint32_t
NgcX2RlcSetupRequestHeader::GetGtpTeid () const
{
  return m_gtpTeid;
}

void
NgcX2RlcSetupRequestHeader::SetGtpTeid (uint32_t gtpTeid)
{
  m_gtpTeid = gtpTeid;
}

void
NgcX2RlcSetupRequestHeader::SetMmWaveRnti (uint16_t rnti)
{
  m_mmWaveRnti = rnti;
}

uint16_t
NgcX2RlcSetupRequestHeader::GetMmWaveRnti () const
{
  return m_mmWaveRnti;
}

void
NgcX2RlcSetupRequestHeader::SetNrRnti (uint16_t rnti)
{
  m_nrRnti = rnti;
}

uint16_t
NgcX2RlcSetupRequestHeader::GetNrRnti () const
{
  return m_nrRnti;
}

uint32_t
NgcX2RlcSetupRequestHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2RlcSetupRequestHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

void
NgcX2RlcSetupRequestHeader::SetDrbid (uint8_t drbid)
{
  m_drbid = drbid;
}

uint8_t
NgcX2RlcSetupRequestHeader::GetDrbid () const
{
  return m_drbid;
}

NrEnbCmacSapProvider::LcInfo 
NgcX2RlcSetupRequestHeader::GetLcInfo() const
{
  return m_lcInfo;
}

void 
NgcX2RlcSetupRequestHeader::SetLcInfo(NrEnbCmacSapProvider::LcInfo lcInfo)
{
  m_lcInfo = lcInfo;
}

NrRrcSap::RlcConfig 
NgcX2RlcSetupRequestHeader::GetRlcConfig() const
{
  return m_rlcConfig;
}

void 
NgcX2RlcSetupRequestHeader::SetRlcConfig(NrRrcSap::RlcConfig rlcConfig)
{
  m_rlcConfig = rlcConfig;
}

NrRrcSap::LogicalChannelConfig 
NgcX2RlcSetupRequestHeader::GetLogicalChannelConfig()
{
  return m_lcConfig;
}

void 
NgcX2RlcSetupRequestHeader::SetLogicalChannelConfig(NrRrcSap::LogicalChannelConfig conf)
{
  m_lcConfig = conf;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2RlcSetupCompletedHeader);

NgcX2RlcSetupCompletedHeader::NgcX2RlcSetupCompletedHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (2 + 2 + 4),
    m_sourceCellId (0xfffa),
    m_targetCellId (0xfffa),
    m_gtpTeid (0xfffffffa)
{
}

NgcX2RlcSetupCompletedHeader::~NgcX2RlcSetupCompletedHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_sourceCellId = 0xfffb;
  m_targetCellId = 0xfffb;
  m_gtpTeid = 0xfffffffb;
}

TypeId
NgcX2RlcSetupCompletedHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2RlcSetupCompletedHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2RlcSetupCompletedHeader> ()
  ;
  return tid;
}

TypeId
NgcX2RlcSetupCompletedHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2RlcSetupCompletedHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2RlcSetupCompletedHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_sourceCellId);
  i.WriteHtonU16 (m_targetCellId); 
  i.WriteHtonU32 (m_gtpTeid); 
}

uint32_t
NgcX2RlcSetupCompletedHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_sourceCellId = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;
  
  m_targetCellId = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;

  m_gtpTeid = i.ReadNtohU32 ();
  m_headerLength += 4;
  m_numberOfIes++;

  return GetSerializedSize ();
}

void
NgcX2RlcSetupCompletedHeader::Print (std::ostream &os) const
{
  os << "SourceCellId = " << m_sourceCellId;
  os << " TargetCellId = " << m_targetCellId;
  os << " gtpTeid = " << m_gtpTeid;
}

uint16_t
NgcX2RlcSetupCompletedHeader::GetSourceCellId () const
{
  return m_sourceCellId;
}

void
NgcX2RlcSetupCompletedHeader::SetSourceCellId (uint16_t cellId)
{
  m_sourceCellId = cellId;
}

uint16_t
NgcX2RlcSetupCompletedHeader::GetTargetCellId () const
{
  return m_targetCellId;
}

void
NgcX2RlcSetupCompletedHeader::SetTargetCellId (uint16_t targetCellId)
{
  m_targetCellId = targetCellId;
}

uint32_t
NgcX2RlcSetupCompletedHeader::GetGtpTeid () const
{
  return m_gtpTeid;
}

void
NgcX2RlcSetupCompletedHeader::SetGtpTeid (uint32_t gtpTeid)
{
  m_gtpTeid = gtpTeid;
}


uint32_t
NgcX2RlcSetupCompletedHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2RlcSetupCompletedHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}


/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2McHandoverHeader);

NgcX2McHandoverHeader::NgcX2McHandoverHeader ()
  : m_numberOfIes (1 + 1 + 1+1),
    m_headerLength (2 + 2 + 8+2),
    m_targetCellId (0xfffa),
	m_secondMmWaveCellId(0xfffa),
    m_oldCellId (0xfffa),
    m_imsi (0xfffffffffffffffa)
{
}

NgcX2McHandoverHeader::~NgcX2McHandoverHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_targetCellId = 0xfffb;
  m_oldCellId = 0xfffb;
  m_secondMmWaveCellId = 0xfffb;  //sjkang
  m_imsi = 0xfffffffffffffffb;
}

TypeId
NgcX2McHandoverHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2McHandoverHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2McHandoverHeader> ()
  ;
  return tid;
}

TypeId
NgcX2McHandoverHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2McHandoverHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2McHandoverHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_targetCellId); 
  i.WriteHtonU16(m_secondMmWaveCellId);
  i.WriteHtonU16 (m_oldCellId); 
  i.WriteHtonU64 (m_imsi); 
}

uint32_t
NgcX2McHandoverHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;
  
  m_targetCellId = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;

  m_secondMmWaveCellId = i.ReadNtohU16();
  m_headerLength += 2;
  m_numberOfIes++;

  m_oldCellId = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;

  m_imsi = i.ReadNtohU64 ();
  m_headerLength += 8;
  m_numberOfIes++;

  return GetSerializedSize ();
}

void
NgcX2McHandoverHeader::Print (std::ostream &os) const
{
  os << " TargetCellId = " << m_targetCellId;
  os << " oldCellId = " << m_oldCellId;
  os << " imsi = " << m_imsi;
 os <<"Second mmWave Cell ID" << m_secondMmWaveCellId;
}

uint16_t
NgcX2McHandoverHeader::GetTargetCellId () const
{
  return m_targetCellId;
}

void
NgcX2McHandoverHeader::SetTargetCellId (uint16_t targetCellId)
{
  m_targetCellId = targetCellId;
}
uint16_t
NgcX2McHandoverHeader::GetSecondMmWaveCellId() const
{
	return m_secondMmWaveCellId;
}
void
NgcX2McHandoverHeader::SetSecondMmWaveCellId(uint16_t secondID){
	m_secondMmWaveCellId = secondID; //sjkang
}
uint64_t
NgcX2McHandoverHeader::GetImsi () const
{
  return m_imsi;
}

void
NgcX2McHandoverHeader::SetImsi (uint64_t imsi)
{
  m_imsi = imsi;
}

uint16_t
NgcX2McHandoverHeader::GetOldCellId () const
{
  return m_oldCellId;
}

void
NgcX2McHandoverHeader::SetOldCellId (uint16_t oldCellId)
{
  m_oldCellId = oldCellId;
}


uint32_t
NgcX2McHandoverHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2McHandoverHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2SecondaryCellHandoverCompletedHeader);

NgcX2SecondaryCellHandoverCompletedHeader::NgcX2SecondaryCellHandoverCompletedHeader ()
  : m_numberOfIes (1 + 1 + 1+1+1),
    m_headerLength (2 + 2 + 8+1+1),
    m_mmWaveRnti (0xfffa),
    m_oldEnbUeX2apId (0xfffa),
    m_imsi (0xfffffffffffffffa),
	m_isMc(0xfa),
	m_isMc_2(0xfa)

{
}

NgcX2SecondaryCellHandoverCompletedHeader::~NgcX2SecondaryCellHandoverCompletedHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_mmWaveRnti = 0xfffb;
  m_oldEnbUeX2apId = 0xfffb;
  m_imsi = 0xfffffffffffffffb;
  m_isMc = 0xfa;
  m_isMc_2 = 0xfa;
}

TypeId
NgcX2SecondaryCellHandoverCompletedHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2SecondaryCellHandoverCompletedHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2SecondaryCellHandoverCompletedHeader> ()
  ;
  return tid;
}

TypeId
NgcX2SecondaryCellHandoverCompletedHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

bool
NgcX2SecondaryCellHandoverCompletedHeader::GetisMc(void) const{
	return m_isMc;
}
bool
NgcX2SecondaryCellHandoverCompletedHeader::GetisMc_2(void) const{
	return m_isMc_2;
}
uint32_t
NgcX2SecondaryCellHandoverCompletedHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2SecondaryCellHandoverCompletedHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_mmWaveRnti); 
  i.WriteHtonU16 (m_oldEnbUeX2apId); 
  i.WriteHtonU64 (m_imsi);
  i.WriteU8(m_isMc);
  i.WriteU8(m_isMc_2);
}

uint32_t
NgcX2SecondaryCellHandoverCompletedHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;
  
  m_mmWaveRnti = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;

  m_oldEnbUeX2apId = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;

  m_imsi = i.ReadNtohU64 ();
  m_headerLength += 8;
  m_numberOfIes++;

  m_isMc = i.ReadU8();
  m_headerLength ++;
  m_numberOfIes ++;

  m_isMc_2 = i.ReadU8();
  m_headerLength ++;
  m_numberOfIes++;

  return GetSerializedSize ();
}

void
NgcX2SecondaryCellHandoverCompletedHeader::Print (std::ostream &os) const
{
  os << " MmWaveRnti = " << m_mmWaveRnti;
  os << " oldEnbUeX2apId = " << m_oldEnbUeX2apId;
  os << " imsi = " << m_imsi;
  os << "isMC = "<< m_isMc ;
  os << "isMc2 = "<< m_isMc_2;
}

uint16_t
NgcX2SecondaryCellHandoverCompletedHeader::GetMmWaveRnti () const
{
  return m_mmWaveRnti;
}

void
NgcX2SecondaryCellHandoverCompletedHeader::SetMmWaveRnti (uint16_t rnti)
{
  m_mmWaveRnti = rnti;
}
void
NgcX2SecondaryCellHandoverCompletedHeader::SetisMc(bool isMc)  //sjkang0710
{
	m_isMc = isMc;
}
void
NgcX2SecondaryCellHandoverCompletedHeader::SetisMc_2(bool isMc_2) //sjkang0710
{
	m_isMc_2 = isMc_2;
}
uint64_t
NgcX2SecondaryCellHandoverCompletedHeader::GetImsi () const
{
  return m_imsi;
}

void
NgcX2SecondaryCellHandoverCompletedHeader::SetImsi (uint64_t imsi)
{
  m_imsi = imsi;
}

uint16_t
NgcX2SecondaryCellHandoverCompletedHeader::GetOldEnbUeX2apId () const
{
  return m_oldEnbUeX2apId;
}

void
NgcX2SecondaryCellHandoverCompletedHeader::SetOldEnbUeX2apId (uint16_t oldId)
{
  m_oldEnbUeX2apId = oldId;
}


uint32_t
NgcX2SecondaryCellHandoverCompletedHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2SecondaryCellHandoverCompletedHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}
///////////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED(NgcX2DuplicationRlcBufferHeader);
NgcX2DuplicationRlcBufferHeader::NgcX2DuplicationRlcBufferHeader()
    :m_numberOfIes(4),
	 m_headerLength(7),
	 m_imsi(0),
	 m_targetCellID(0),
	 m_cellIDforBufferForwarding(0),
	 m_option(false)
		{

		}
NgcX2DuplicationRlcBufferHeader::~NgcX2DuplicationRlcBufferHeader(){
	m_numberOfIes = 0;
	m_headerLength = 0;
	m_imsi = 0;
	m_targetCellID =0;
	m_cellIDforBufferForwarding = 0;
	m_option = false;
}
TypeId
NgcX2DuplicationRlcBufferHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2DuplicationRlcBufferHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2DuplicationRlcBufferHeader> ()
  ;
  return tid;
}
TypeId
NgcX2DuplicationRlcBufferHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void
NgcX2DuplicationRlcBufferHeader::SetImsi(uint16_t imsi){
	m_imsi  = imsi;
}
uint16_t
NgcX2DuplicationRlcBufferHeader::GetImsi() const {
	return m_imsi;
}
void
NgcX2DuplicationRlcBufferHeader::SetTargetCellID(uint16_t targetCellID){
	m_targetCellID = targetCellID;
}
uint16_t
NgcX2DuplicationRlcBufferHeader::GetTargetCellID() const {
	return m_targetCellID;
}
void
NgcX2DuplicationRlcBufferHeader::SetCellIDForBufferForwarding(uint16_t cellIDforBufferForwarding){
	m_cellIDforBufferForwarding = cellIDforBufferForwarding;
}
uint16_t
NgcX2DuplicationRlcBufferHeader::GetCellIDForBufferForwarding() const {
	return m_cellIDforBufferForwarding;
}
bool
NgcX2DuplicationRlcBufferHeader::GetOption() const{
	return m_option;
}
void
NgcX2DuplicationRlcBufferHeader::SetOption(bool option){
	m_option = option;
}
void
NgcX2DuplicationRlcBufferHeader::Serialize(Buffer::Iterator start) const{
	 Buffer::Iterator i = start;

	  i.WriteHtonU16 (m_imsi);
	  i.WriteHtonU16(m_targetCellID);
	  i.WriteHtonU16(m_cellIDforBufferForwarding);
	  i.WriteU8(m_option);

}
uint32_t
NgcX2DuplicationRlcBufferHeader::Deserialize(Buffer::Iterator start){
	 Buffer::Iterator i = start;

	  m_headerLength = 0;
	  m_numberOfIes = 0;

	  m_imsi = i.ReadNtohU16 ();
	  m_headerLength += 2;
	  m_numberOfIes++;

	  m_targetCellID = i.ReadNtohU16();
	  m_headerLength += 2;
	  m_numberOfIes++;

	  m_cellIDforBufferForwarding = i.ReadNtohU16();
	  m_headerLength += 2;
	  m_numberOfIes++;

	  m_option = i.ReadU8();
	  m_headerLength +=2;
	  m_numberOfIes ++;

	  return GetSerializedSize ();
}
uint32_t
NgcX2DuplicationRlcBufferHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}
void
NgcX2DuplicationRlcBufferHeader::Print (std::ostream &os) const
{
  os << " imsi = " << m_imsi;
  os << " target CellID = " << m_targetCellID ;
  os << "cell ID for forwarding RLC buffer " << m_cellIDforBufferForwarding;
  os << "option if we use buffer forwarding or duplication" << m_option ;
}
uint32_t
NgcX2DuplicationRlcBufferHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2DuplicationRlcBufferHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}
/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2NotifyCoordinatorHandoverFailedHeader);

NgcX2NotifyCoordinatorHandoverFailedHeader::NgcX2NotifyCoordinatorHandoverFailedHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (2 + 2 + 8),
    m_targetCellId (0xfffa),
    m_sourceCellId (0xfffa),
    m_imsi (0xfffffffffffffffa)
{
}

NgcX2NotifyCoordinatorHandoverFailedHeader::~NgcX2NotifyCoordinatorHandoverFailedHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_targetCellId = 0xfffb;
  m_sourceCellId = 0xfffb;
  m_imsi = 0xfffffffffffffffb;
}

TypeId
NgcX2NotifyCoordinatorHandoverFailedHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2NotifyCoordinatorHandoverFailedHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2NotifyCoordinatorHandoverFailedHeader> ()
  ;
  return tid;
}

TypeId
NgcX2NotifyCoordinatorHandoverFailedHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2NotifyCoordinatorHandoverFailedHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2NotifyCoordinatorHandoverFailedHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_targetCellId); 
  i.WriteHtonU16 (m_sourceCellId); 
  i.WriteHtonU64 (m_imsi); 
}

uint32_t
NgcX2NotifyCoordinatorHandoverFailedHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;
  
  m_targetCellId = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;

  m_sourceCellId = i.ReadNtohU16 ();
  m_headerLength += 2;
  m_numberOfIes++;

  m_imsi = i.ReadNtohU64 ();
  m_headerLength += 8;
  m_numberOfIes++;

  return GetSerializedSize ();
}

void
NgcX2NotifyCoordinatorHandoverFailedHeader::Print (std::ostream &os) const
{
  os << " TargetCellId = " << m_targetCellId;
  os << " oldCellId = " << m_sourceCellId;
  os << " imsi = " << m_imsi;
}

uint16_t
NgcX2NotifyCoordinatorHandoverFailedHeader::GetTargetCellId () const
{
  return m_targetCellId;
}

void
NgcX2NotifyCoordinatorHandoverFailedHeader::SetTargetCellId (uint16_t targetCellId)
{
  m_targetCellId = targetCellId;
}

uint64_t
NgcX2NotifyCoordinatorHandoverFailedHeader::GetImsi () const
{
  return m_imsi;
}

void
NgcX2NotifyCoordinatorHandoverFailedHeader::SetImsi (uint64_t imsi)
{
  m_imsi = imsi;
}

uint16_t
NgcX2NotifyCoordinatorHandoverFailedHeader::GetSourceCellId () const
{
  return m_sourceCellId;
}

void
NgcX2NotifyCoordinatorHandoverFailedHeader::SetSourceCellId (uint16_t oldCellId)
{
  m_sourceCellId = oldCellId;
}


uint32_t
NgcX2NotifyCoordinatorHandoverFailedHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2NotifyCoordinatorHandoverFailedHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}



/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2HandoverRequestAckHeader);

NgcX2HandoverRequestAckHeader::NgcX2HandoverRequestAckHeader ()
  : m_numberOfIes (1 + 1 + 1 + 1),
    m_headerLength (2 + 2 + 4 + 4),
    m_oldEnbUeX2apId (0xfffa),
    m_newEnbUeX2apId (0xfffa)
{
}

NgcX2HandoverRequestAckHeader::~NgcX2HandoverRequestAckHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_oldEnbUeX2apId = 0xfffb;
  m_newEnbUeX2apId = 0xfffb;
  m_erabsAdmittedList.clear ();
  m_erabsNotAdmittedList.clear ();
}

TypeId
NgcX2HandoverRequestAckHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2HandoverRequestAckHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2HandoverRequestAckHeader> ()
  ;
  return tid;
}

TypeId
NgcX2HandoverRequestAckHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2HandoverRequestAckHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2HandoverRequestAckHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_oldEnbUeX2apId);
  i.WriteHtonU16 (m_newEnbUeX2apId);

  std::vector <NgcX2Sap::ErabAdmittedItem>::size_type sz = m_erabsAdmittedList.size (); 
  i.WriteHtonU32 (sz);
  for (int j = 0; j < (int) sz; j++)
    {
      i.WriteHtonU16 (m_erabsAdmittedList [j].erabId);
      i.WriteHtonU32 (m_erabsAdmittedList [j].ulGtpTeid);
      i.WriteHtonU32 (m_erabsAdmittedList [j].dlGtpTeid);
    }

  std::vector <NgcX2Sap::ErabNotAdmittedItem>::size_type sz2 = m_erabsNotAdmittedList.size (); 
  i.WriteHtonU32 (sz2);
  for (int j = 0; j < (int) sz2; j++)
    {
      i.WriteHtonU16 (m_erabsNotAdmittedList [j].erabId);
      i.WriteHtonU16 (m_erabsNotAdmittedList [j].cause);
    }
}

uint32_t
NgcX2HandoverRequestAckHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_oldEnbUeX2apId = i.ReadNtohU16 ();
  m_newEnbUeX2apId = i.ReadNtohU16 ();
  m_headerLength += 4;
  m_numberOfIes += 2;

  int sz = i.ReadNtohU32 ();
  m_headerLength += 4;
  m_numberOfIes++;

  for (int j = 0; j < sz; j++)
    {
      NgcX2Sap::ErabAdmittedItem erabItem;

      erabItem.erabId = i.ReadNtohU16 ();
      erabItem.ulGtpTeid = i.ReadNtohU32 ();
      erabItem.dlGtpTeid = i.ReadNtohU32 ();

      m_erabsAdmittedList.push_back (erabItem);
      m_headerLength += 10;
    }

  sz = i.ReadNtohU32 ();
  m_headerLength += 4;
  m_numberOfIes++;

  for (int j = 0; j < sz; j++)
    {
      NgcX2Sap::ErabNotAdmittedItem erabItem;

      erabItem.erabId = i.ReadNtohU16 ();
      erabItem.cause  = i.ReadNtohU16 ();

      m_erabsNotAdmittedList.push_back (erabItem);
      m_headerLength += 4;
    }

  return GetSerializedSize ();
}

void
NgcX2HandoverRequestAckHeader::Print (std::ostream &os) const
{
  os << "OldEnbUeX2apId=" << m_oldEnbUeX2apId;
  os << " NewEnbUeX2apId=" << m_newEnbUeX2apId;

  os << " AdmittedBearers=" << m_erabsAdmittedList.size ();
  std::vector <NgcX2Sap::ErabAdmittedItem>::size_type sz = m_erabsAdmittedList.size ();
  if (sz > 0)
    {
      os << " [";
    }
  for (int j = 0; j < (int) sz; j++)
    {
      os << m_erabsAdmittedList[j].erabId;
      if (j < (int) sz - 1)
        {
          os << ", ";
        }
      else
        {
          os << "]";
        }
    }
  
  os << " NotAdmittedBearers=" << m_erabsNotAdmittedList.size ();
  std::vector <NgcX2Sap::ErabNotAdmittedItem>::size_type sz2 = m_erabsNotAdmittedList.size ();
  if (sz2 > 0)
    {
      os << " [";
    }
  for (int j = 0; j < (int) sz2; j++)
    {
      os << m_erabsNotAdmittedList[j].erabId;
      if (j < (int) sz2 - 1)
        {
          os << ", ";
        }
      else
        {
          os << "]";
        }
    }

}

uint16_t
NgcX2HandoverRequestAckHeader::GetOldEnbUeX2apId () const
{
  return m_oldEnbUeX2apId;
}

void
NgcX2HandoverRequestAckHeader::SetOldEnbUeX2apId (uint16_t x2apId)
{
  m_oldEnbUeX2apId = x2apId;
}

uint16_t
NgcX2HandoverRequestAckHeader::GetNewEnbUeX2apId () const
{
  return m_newEnbUeX2apId;
}

void
NgcX2HandoverRequestAckHeader::SetNewEnbUeX2apId (uint16_t x2apId)
{
  m_newEnbUeX2apId = x2apId;
}

std::vector <NgcX2Sap::ErabAdmittedItem> 
NgcX2HandoverRequestAckHeader::GetAdmittedBearers () const
{
  return m_erabsAdmittedList;
}

void
NgcX2HandoverRequestAckHeader::SetAdmittedBearers (std::vector <NgcX2Sap::ErabAdmittedItem> bearers)
{
  m_headerLength += 10 * bearers.size ();
  m_erabsAdmittedList = bearers;
}

std::vector <NgcX2Sap::ErabNotAdmittedItem>
NgcX2HandoverRequestAckHeader::GetNotAdmittedBearers () const
{
  return m_erabsNotAdmittedList;
}

void
NgcX2HandoverRequestAckHeader::SetNotAdmittedBearers (std::vector <NgcX2Sap::ErabNotAdmittedItem> bearers)
{
  m_headerLength += 4 * bearers.size ();
  m_erabsNotAdmittedList = bearers;
}

uint32_t
NgcX2HandoverRequestAckHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2HandoverRequestAckHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////
NS_OBJECT_ENSURE_REGISTERED (NgcX2HandoverPreparationFailureHeader);

NgcX2HandoverPreparationFailureHeader::NgcX2HandoverPreparationFailureHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (2 + 2 + 2),
    m_oldEnbUeX2apId (0xfffa),
    m_cause (0xfffa),
    m_criticalityDiagnostics (0xfffa)
{
}

NgcX2HandoverPreparationFailureHeader::~NgcX2HandoverPreparationFailureHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_oldEnbUeX2apId = 0xfffb;
  m_cause = 0xfffb;
  m_criticalityDiagnostics = 0xfffb;
}

TypeId
NgcX2HandoverPreparationFailureHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2HandoverPreparationFailureHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2HandoverPreparationFailureHeader> ()
  ;
  return tid;
}

TypeId
NgcX2HandoverPreparationFailureHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2HandoverPreparationFailureHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2HandoverPreparationFailureHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_oldEnbUeX2apId);
  i.WriteHtonU16 (m_cause);
  i.WriteHtonU16 (m_criticalityDiagnostics);
}

uint32_t
NgcX2HandoverPreparationFailureHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_oldEnbUeX2apId = i.ReadNtohU16 ();
  m_cause = i.ReadNtohU16 ();
  m_criticalityDiagnostics = i.ReadNtohU16 ();

  m_headerLength = 6;
  m_numberOfIes = 3;

  return GetSerializedSize ();
}

void
NgcX2HandoverPreparationFailureHeader::Print (std::ostream &os) const
{
  os << "OldEnbUeX2apId = " << m_oldEnbUeX2apId;
  os << " Cause = " << m_cause;
  os << " CriticalityDiagnostics = " << m_criticalityDiagnostics;
}

uint16_t
NgcX2HandoverPreparationFailureHeader::GetOldEnbUeX2apId () const
{
  return m_oldEnbUeX2apId;
}

void
NgcX2HandoverPreparationFailureHeader::SetOldEnbUeX2apId (uint16_t x2apId)
{
  m_oldEnbUeX2apId = x2apId;
}

uint16_t
NgcX2HandoverPreparationFailureHeader::GetCause () const
{
  return m_cause;
}

void
NgcX2HandoverPreparationFailureHeader::SetCause (uint16_t cause)
{
  m_cause = cause;
}

uint16_t
NgcX2HandoverPreparationFailureHeader::GetCriticalityDiagnostics () const
{
  return m_criticalityDiagnostics;
}

void
NgcX2HandoverPreparationFailureHeader::SetCriticalityDiagnostics (uint16_t criticalityDiagnostics)
{
  m_criticalityDiagnostics = criticalityDiagnostics;
}

uint32_t
NgcX2HandoverPreparationFailureHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2HandoverPreparationFailureHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2SnStatusTransferHeader);

NgcX2SnStatusTransferHeader::NgcX2SnStatusTransferHeader ()
  : m_numberOfIes (3),
    m_headerLength (6),
    m_oldEnbUeX2apId (0xfffa),
    m_newEnbUeX2apId (0xfffa)
{
  m_erabsSubjectToStatusTransferList.clear (); 
}

NgcX2SnStatusTransferHeader::~NgcX2SnStatusTransferHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_oldEnbUeX2apId = 0xfffb;
  m_newEnbUeX2apId = 0xfffb;
  m_erabsSubjectToStatusTransferList.clear (); 
}

TypeId
NgcX2SnStatusTransferHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2SnStatusTransferHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2SnStatusTransferHeader> ()
  ;
  return tid;
}

TypeId
NgcX2SnStatusTransferHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2SnStatusTransferHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2SnStatusTransferHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_oldEnbUeX2apId);
  i.WriteHtonU16 (m_newEnbUeX2apId);

  std::vector <NgcX2Sap::ErabsSubjectToStatusTransferItem>::size_type sz = m_erabsSubjectToStatusTransferList.size ();
  i.WriteHtonU16 (sz);              // number of ErabsSubjectToStatusTransferItems

  for (int j = 0; j < (int) sz; j++)
    {
      NgcX2Sap::ErabsSubjectToStatusTransferItem item = m_erabsSubjectToStatusTransferList [j];

      i.WriteHtonU16 (item.erabId);

      uint16_t bitsetSize = NgcX2Sap::m_maxPdcpSn / 64;
      for (int k = 0; k < bitsetSize; k++)
        {
          uint64_t statusValue = 0;
          for (int m = 0; m < 64; m++)
            {
              statusValue |= item.receiveStatusOfUlPdcpSdus[64 * k + m] << m;
            }
          i.WriteHtonU64 (statusValue);
        }

      i.WriteHtonU16 (item.ulPdcpSn);
      i.WriteHtonU32 (item.ulHfn);
      i.WriteHtonU16 (item.dlPdcpSn);
      i.WriteHtonU32 (item.dlHfn);
    }
}

uint32_t
NgcX2SnStatusTransferHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_oldEnbUeX2apId = i.ReadNtohU16 ();
  m_newEnbUeX2apId = i.ReadNtohU16 ();
  int sz = i.ReadNtohU16 ();

  m_numberOfIes = 3;
  m_headerLength = 6 + sz * (14 + (NgcX2Sap::m_maxPdcpSn / 64));

  for (int j = 0; j < sz; j++)
    {
      NgcX2Sap::ErabsSubjectToStatusTransferItem ErabItem;
      ErabItem.erabId = i.ReadNtohU16 ();

      uint16_t bitsetSize = NgcX2Sap::m_maxPdcpSn / 64;
      for (int k = 0; k < bitsetSize; k++)
        {
          uint64_t statusValue = i.ReadNtohU64 ();
          for (int m = 0; m < 64; m++)
            {
              ErabItem.receiveStatusOfUlPdcpSdus[64 * k + m] = (statusValue >> m) & 1;
            }
        }

      ErabItem.ulPdcpSn = i.ReadNtohU16 ();
      ErabItem.ulHfn    = i.ReadNtohU32 ();
      ErabItem.dlPdcpSn = i.ReadNtohU16 ();
      ErabItem.dlHfn    = i.ReadNtohU32 ();

      m_erabsSubjectToStatusTransferList.push_back (ErabItem);
    }

  return GetSerializedSize ();
}

void
NgcX2SnStatusTransferHeader::Print (std::ostream &os) const
{
  os << "OldEnbUeX2apId = " << m_oldEnbUeX2apId;
  os << " NewEnbUeX2apId = " << m_newEnbUeX2apId;
  os << " ErabsSubjectToStatusTransferList size = " << m_erabsSubjectToStatusTransferList.size ();

  std::vector <NgcX2Sap::ErabsSubjectToStatusTransferItem>::size_type sz = m_erabsSubjectToStatusTransferList.size ();
  if (sz > 0)
    {
      os << " [";
    }
  for (int j = 0; j < (int) sz; j++)
    {
      os << m_erabsSubjectToStatusTransferList[j].erabId;
      if (j < (int) sz - 1)
        {
          os << ", ";
        }
      else
        {
          os << "]";
        }
    }
}

uint16_t
NgcX2SnStatusTransferHeader::GetOldEnbUeX2apId () const
{
  return m_oldEnbUeX2apId;
}

void
NgcX2SnStatusTransferHeader::SetOldEnbUeX2apId (uint16_t x2apId)
{
  m_oldEnbUeX2apId = x2apId;
}

uint16_t
NgcX2SnStatusTransferHeader::GetNewEnbUeX2apId () const
{
  return m_newEnbUeX2apId;
}

void
NgcX2SnStatusTransferHeader::SetNewEnbUeX2apId (uint16_t x2apId)
{
  m_newEnbUeX2apId = x2apId;
}

std::vector <NgcX2Sap::ErabsSubjectToStatusTransferItem>
NgcX2SnStatusTransferHeader::GetErabsSubjectToStatusTransferList () const
{
  return m_erabsSubjectToStatusTransferList;
}

void
NgcX2SnStatusTransferHeader::SetErabsSubjectToStatusTransferList (std::vector <NgcX2Sap::ErabsSubjectToStatusTransferItem> erabs)
{
  m_headerLength += erabs.size () * (14 + (NgcX2Sap::m_maxPdcpSn / 8));
  m_erabsSubjectToStatusTransferList = erabs;
}

uint32_t
NgcX2SnStatusTransferHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2SnStatusTransferHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2UeContextReleaseHeader);

NgcX2UeContextReleaseHeader::NgcX2UeContextReleaseHeader ()
  : m_numberOfIes (1 + 1),
    m_headerLength (2 + 2),
    m_oldEnbUeX2apId (0xfffa),
    m_newEnbUeX2apId (0xfffa)
{
}

NgcX2UeContextReleaseHeader::~NgcX2UeContextReleaseHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_oldEnbUeX2apId = 0xfffb;
  m_newEnbUeX2apId = 0xfffb;
}

TypeId
NgcX2UeContextReleaseHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2UeContextReleaseHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2UeContextReleaseHeader> ()
  ;
  return tid;
}

TypeId
NgcX2UeContextReleaseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2UeContextReleaseHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2UeContextReleaseHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_oldEnbUeX2apId);
  i.WriteHtonU16 (m_newEnbUeX2apId);
}

uint32_t
NgcX2UeContextReleaseHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_oldEnbUeX2apId = i.ReadNtohU16 ();
  m_newEnbUeX2apId = i.ReadNtohU16 ();
  m_numberOfIes = 2;
  m_headerLength = 4;

  return GetSerializedSize ();
}

void
NgcX2UeContextReleaseHeader::Print (std::ostream &os) const
{
  os << "OldEnbUeX2apId=" << m_oldEnbUeX2apId;
  os << " NewEnbUeX2apId=" << m_newEnbUeX2apId;
}

uint16_t
NgcX2UeContextReleaseHeader::GetOldEnbUeX2apId () const
{
  return m_oldEnbUeX2apId;
}

void
NgcX2UeContextReleaseHeader::SetOldEnbUeX2apId (uint16_t x2apId)
{
  m_oldEnbUeX2apId = x2apId;
}

uint16_t
NgcX2UeContextReleaseHeader::GetNewEnbUeX2apId () const
{
  return m_newEnbUeX2apId;
}

void
NgcX2UeContextReleaseHeader::SetNewEnbUeX2apId (uint16_t x2apId)
{
  m_newEnbUeX2apId = x2apId;
}

uint32_t
NgcX2UeContextReleaseHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2UeContextReleaseHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2LoadInformationHeader);

NgcX2LoadInformationHeader::NgcX2LoadInformationHeader ()
  : m_numberOfIes (1),
    m_headerLength (6)
{
  m_cellInformationList.clear ();
}

NgcX2LoadInformationHeader::~NgcX2LoadInformationHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_cellInformationList.clear ();
}

TypeId
NgcX2LoadInformationHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2LoadInformationHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2LoadInformationHeader> ()
  ;
  return tid;
}

TypeId
NgcX2LoadInformationHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2LoadInformationHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2LoadInformationHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (6);               // id = CELL_INFORMATION
  i.WriteU8 (1 << 6);               // criticality = IGNORE
  i.WriteU8 (4);                    // length of CELL_INFORMATION_ID

  std::vector <NgcX2Sap::CellInformationItem>::size_type sz = m_cellInformationList.size ();
  i.WriteHtonU16 (sz);              // number of cellInformationItems

  for (int j = 0; j < (int) sz; j++)
    {
      i.WriteHtonU16 (m_cellInformationList [j].sourceCellId);

      std::vector <NgcX2Sap::UlInterferenceOverloadIndicationItem>::size_type sz2;
      sz2 = m_cellInformationList [j].ulInterferenceOverloadIndicationList.size ();
      i.WriteHtonU16 (sz2);         // number of UlInterferenceOverloadIndicationItem

      for (int k = 0; k < (int) sz2; k++)
        {
          i.WriteU8 (m_cellInformationList [j].ulInterferenceOverloadIndicationList [k]);
        }

      std::vector <NgcX2Sap::UlHighInterferenceInformationItem>::size_type sz3;
      sz3 = m_cellInformationList [j].ulHighInterferenceInformationList.size ();
      i.WriteHtonU16 (sz3);         // number of UlHighInterferenceInformationItem

      for (int k = 0; k < (int) sz3; k++)
        {
          i.WriteHtonU16 (m_cellInformationList [j].ulHighInterferenceInformationList [k].targetCellId);

          std::vector <bool>::size_type sz4;
          sz4 = m_cellInformationList [j].ulHighInterferenceInformationList [k].ulHighInterferenceIndicationList.size ();
          i.WriteHtonU16 (sz4);

          for (int m = 0; m < (int) sz4; m++)
            {
              i.WriteU8 (m_cellInformationList [j].ulHighInterferenceInformationList [k].ulHighInterferenceIndicationList [m]);
            }
        }

      std::vector <bool>::size_type sz5;
      sz5 = m_cellInformationList [j].relativeNarrowbandTxBand.rntpPerPrbList.size ();
      i.WriteHtonU16 (sz5);

      for (int k = 0; k < (int) sz5; k++)
        {
          i.WriteU8 (m_cellInformationList [j].relativeNarrowbandTxBand.rntpPerPrbList [k]);
        }

      i.WriteHtonU16 (m_cellInformationList [j].relativeNarrowbandTxBand.rntpThreshold);
      i.WriteHtonU16 (m_cellInformationList [j].relativeNarrowbandTxBand.antennaPorts);
      i.WriteHtonU16 (m_cellInformationList [j].relativeNarrowbandTxBand.pB);
      i.WriteHtonU16 (m_cellInformationList [j].relativeNarrowbandTxBand.pdcchInterferenceImpact);
    }
}

uint32_t
NgcX2LoadInformationHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  i.ReadNtohU16 ();
  i.ReadU8 ();
  i.ReadU8 ();
  int sz = i.ReadNtohU16 ();
  m_headerLength += 6;
  m_numberOfIes++;

  for (int j = 0; j < sz; j++)
    {
      NgcX2Sap::CellInformationItem cellInfoItem;
      cellInfoItem.sourceCellId = i.ReadNtohU16 ();
      m_headerLength += 2;

      int sz2 = i.ReadNtohU16 ();
      m_headerLength += 2;
      for (int k = 0; k < sz2; k++)
        {
          NgcX2Sap::UlInterferenceOverloadIndicationItem item = (NgcX2Sap::UlInterferenceOverloadIndicationItem) i.ReadU8 ();
          cellInfoItem.ulInterferenceOverloadIndicationList.push_back (item);
        }
      m_headerLength += sz2;

      int sz3 = i.ReadNtohU16 ();
      m_headerLength += 2;
      for (int k = 0; k < sz3; k++)
        {
          NgcX2Sap::UlHighInterferenceInformationItem item;
          item.targetCellId = i.ReadNtohU16 ();
          m_headerLength += 2;

          int sz4 = i.ReadNtohU16 ();
          m_headerLength += 2;
          for (int m = 0; m < sz4; m++)
            {
              item.ulHighInterferenceIndicationList.push_back (i.ReadU8 ());
            }
          m_headerLength += sz4;

          cellInfoItem.ulHighInterferenceInformationList.push_back (item);
        }

      int sz5 = i.ReadNtohU16 ();
      m_headerLength += 2;
      for (int k = 0; k < sz5; k++)
        {
          cellInfoItem.relativeNarrowbandTxBand.rntpPerPrbList.push_back (i.ReadU8 ());
        }
      m_headerLength += sz5;

      cellInfoItem.relativeNarrowbandTxBand.rntpThreshold = i.ReadNtohU16 ();
      cellInfoItem.relativeNarrowbandTxBand.antennaPorts = i.ReadNtohU16 ();
      cellInfoItem.relativeNarrowbandTxBand.pB = i.ReadNtohU16 ();
      cellInfoItem.relativeNarrowbandTxBand.pdcchInterferenceImpact = i.ReadNtohU16 ();
      m_headerLength += 8;

      m_cellInformationList.push_back (cellInfoItem);
    }

  return GetSerializedSize ();
}

void
NgcX2LoadInformationHeader::Print (std::ostream &os) const
{
  os << "NumOfCellInformationItems=" << m_cellInformationList.size ();
}

std::vector <NgcX2Sap::CellInformationItem>
NgcX2LoadInformationHeader::GetCellInformationList () const
{
  return m_cellInformationList;
}

void
NgcX2LoadInformationHeader::SetCellInformationList (std::vector <NgcX2Sap::CellInformationItem> cellInformationList)
{
  m_cellInformationList = cellInformationList;
  m_headerLength += 2;

  std::vector <NgcX2Sap::CellInformationItem>::size_type sz = m_cellInformationList.size ();
  for (int j = 0; j < (int) sz; j++)
    {
      m_headerLength += 2;

      std::vector <NgcX2Sap::UlInterferenceOverloadIndicationItem>::size_type sz2;
      sz2 = m_cellInformationList [j].ulInterferenceOverloadIndicationList.size ();
      m_headerLength += 2 + sz2;

      std::vector <NgcX2Sap::UlHighInterferenceInformationItem>::size_type sz3;
      sz3 = m_cellInformationList [j].ulHighInterferenceInformationList.size ();
      m_headerLength += 2;

      for (int k = 0; k < (int) sz3; k++)
        {
          std::vector <bool>::size_type sz4;
          sz4 = m_cellInformationList [j].ulHighInterferenceInformationList [k].ulHighInterferenceIndicationList.size ();
          m_headerLength += 2 + 2 + sz4;
        }

      std::vector <bool>::size_type sz5;
      sz5 = m_cellInformationList [j].relativeNarrowbandTxBand.rntpPerPrbList.size ();
      m_headerLength += 2 + sz5 + 8;
    }
}

uint32_t
NgcX2LoadInformationHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2LoadInformationHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2ResourceStatusUpdateHeader);

NgcX2ResourceStatusUpdateHeader::NgcX2ResourceStatusUpdateHeader ()
  : m_numberOfIes (3),
    m_headerLength (6),
    m_enb1MeasurementId (0xfffa),
    m_enb2MeasurementId (0xfffa)
{
  m_cellMeasurementResultList.clear ();
}

NgcX2ResourceStatusUpdateHeader::~NgcX2ResourceStatusUpdateHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enb1MeasurementId = 0xfffb;
  m_enb2MeasurementId = 0xfffb;
  m_cellMeasurementResultList.clear ();
}

TypeId
NgcX2ResourceStatusUpdateHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2ResourceStatusUpdateHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2ResourceStatusUpdateHeader> ()
  ;
  return tid;
}

TypeId
NgcX2ResourceStatusUpdateHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2ResourceStatusUpdateHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2ResourceStatusUpdateHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_enb1MeasurementId);
  i.WriteHtonU16 (m_enb2MeasurementId);

  std::vector <NgcX2Sap::CellMeasurementResultItem>::size_type sz = m_cellMeasurementResultList.size ();
  i.WriteHtonU16 (sz);              // number of CellMeasurementResultItem

  for (int j = 0; j < (int) sz; j++)
    {
      NgcX2Sap::CellMeasurementResultItem item = m_cellMeasurementResultList [j];

      i.WriteHtonU16 (item.sourceCellId);
      i.WriteU8 (item.dlHardwareLoadIndicator);
      i.WriteU8 (item.ulHardwareLoadIndicator);
      i.WriteU8 (item.dlN2TnlLoadIndicator);
      i.WriteU8 (item.ulN2TnlLoadIndicator);

      i.WriteHtonU16 (item.dlGbrPrbUsage);
      i.WriteHtonU16 (item.ulGbrPrbUsage);
      i.WriteHtonU16 (item.dlNonGbrPrbUsage);
      i.WriteHtonU16 (item.ulNonGbrPrbUsage);
      i.WriteHtonU16 (item.dlTotalPrbUsage);
      i.WriteHtonU16 (item.ulTotalPrbUsage);

      i.WriteHtonU16 (item.dlCompositeAvailableCapacity.cellCapacityClassValue);
      i.WriteHtonU16 (item.dlCompositeAvailableCapacity.capacityValue);
      i.WriteHtonU16 (item.ulCompositeAvailableCapacity.cellCapacityClassValue);
      i.WriteHtonU16 (item.ulCompositeAvailableCapacity.capacityValue);
    }
}

uint32_t
NgcX2ResourceStatusUpdateHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_enb1MeasurementId = i.ReadNtohU16 ();
  m_enb2MeasurementId = i.ReadNtohU16 ();

  int sz = i.ReadNtohU16 ();
  for (int j = 0; j < sz; j++)
    {
      NgcX2Sap::CellMeasurementResultItem item;

      item.sourceCellId = i.ReadNtohU16 ();
      item.dlHardwareLoadIndicator = (NgcX2Sap::LoadIndicator) i.ReadU8 ();
      item.ulHardwareLoadIndicator = (NgcX2Sap::LoadIndicator) i.ReadU8 ();
      item.dlN2TnlLoadIndicator = (NgcX2Sap::LoadIndicator) i.ReadU8 ();
      item.ulN2TnlLoadIndicator = (NgcX2Sap::LoadIndicator) i.ReadU8 ();

      item.dlGbrPrbUsage = i.ReadNtohU16 ();
      item.ulGbrPrbUsage = i.ReadNtohU16 ();
      item.dlNonGbrPrbUsage = i.ReadNtohU16 ();
      item.ulNonGbrPrbUsage = i.ReadNtohU16 ();
      item.dlTotalPrbUsage = i.ReadNtohU16 ();
      item.ulTotalPrbUsage = i.ReadNtohU16 ();

      item.dlCompositeAvailableCapacity.cellCapacityClassValue = i.ReadNtohU16 ();
      item.dlCompositeAvailableCapacity.capacityValue = i.ReadNtohU16 ();
      item.ulCompositeAvailableCapacity.cellCapacityClassValue = i.ReadNtohU16 ();
      item.ulCompositeAvailableCapacity.capacityValue = i.ReadNtohU16 ();

      m_cellMeasurementResultList.push_back (item);
    }

  m_headerLength = 6 + sz * 26;
  m_numberOfIes = 3;

  return GetSerializedSize ();
}

void
NgcX2ResourceStatusUpdateHeader::Print (std::ostream &os) const
{
  os << "Enb1MeasurementId = " << m_enb1MeasurementId
     << " Enb2MeasurementId = " << m_enb2MeasurementId
     << " NumOfCellMeasurementResultItems = " << m_cellMeasurementResultList.size ();
}

uint16_t
NgcX2ResourceStatusUpdateHeader::GetEnb1MeasurementId () const
{
  return m_enb1MeasurementId;
}

void
NgcX2ResourceStatusUpdateHeader::SetEnb1MeasurementId (uint16_t enb1MeasurementId)
{
  m_enb1MeasurementId = enb1MeasurementId;
}

uint16_t
NgcX2ResourceStatusUpdateHeader::GetEnb2MeasurementId () const
{
  return m_enb2MeasurementId;
}

void
NgcX2ResourceStatusUpdateHeader::SetEnb2MeasurementId (uint16_t enb2MeasurementId)
{
  m_enb2MeasurementId = enb2MeasurementId;
}

std::vector <NgcX2Sap::CellMeasurementResultItem>
NgcX2ResourceStatusUpdateHeader::GetCellMeasurementResultList () const
{
  return m_cellMeasurementResultList;
}

void
NgcX2ResourceStatusUpdateHeader::SetCellMeasurementResultList (std::vector <NgcX2Sap::CellMeasurementResultItem> cellMeasurementResultList)
{
  m_cellMeasurementResultList = cellMeasurementResultList;

  std::vector <NgcX2Sap::CellMeasurementResultItem>::size_type sz = m_cellMeasurementResultList.size ();
  m_headerLength += sz * 26;
}

uint32_t
NgcX2ResourceStatusUpdateHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2ResourceStatusUpdateHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2UeImsiSinrUpdateHeader);

NgcX2UeImsiSinrUpdateHeader::NgcX2UeImsiSinrUpdateHeader ()
  : m_numberOfIes (1 + 1 +1),
    m_headerLength (2 + 2 +2)
{
  m_map.clear ();
}

NgcX2UeImsiSinrUpdateHeader::~NgcX2UeImsiSinrUpdateHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_map.clear ();
}

TypeId
NgcX2UeImsiSinrUpdateHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2UeImsiSinrUpdateHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2UeImsiSinrUpdateHeader> ()
  ;
  return tid;
}

TypeId
NgcX2UeImsiSinrUpdateHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2UeImsiSinrUpdateHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2UeImsiSinrUpdateHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 (m_sourceCellId);
  i.WriteU8 (m_secondCellId); //sjkang1015
  i.WriteU16(m_rnti);
  std::map <uint64_t, double>::size_type sz = m_map.size ();
  i.WriteHtonU16 (sz);              // number of elements in the map

  for (std::map<uint64_t, double>::const_iterator iter = m_map.begin(); iter != m_map.end(); ++iter)
    {
      i.WriteHtonU64 (iter->first); // imsi
      i.WriteHtonU64 (pack754(iter->second)); // sinr
    }
}

uint32_t
NgcX2UeImsiSinrUpdateHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;

  m_sourceCellId = i.ReadU8();
 m_secondCellId = i.ReadU8(); //sjkang
 m_rnti = i.ReadU16();
  m_headerLength += 4;
  m_numberOfIes = 2;

  int sz = i.ReadNtohU16 ();
  for (int j = 0; j < sz; j++)
    {
      uint64_t imsi = i.ReadNtohU64();
      double sinr = unpack754(i.ReadNtohU64());
      m_map[imsi] = sinr;
    }

  m_headerLength += 2 + sz * 16;
  m_numberOfIes += 1 + sz;

  return GetSerializedSize ();
}

void
NgcX2UeImsiSinrUpdateHeader::Print (std::ostream &os) const
{
  os << "SourceCellId " << m_sourceCellId;
  os << "SecondCellId " << m_secondCellId;
  os<<"m_rnti" << m_rnti; //sjkang

  for(std::map<uint64_t, double>::const_iterator iter = m_map.begin(); iter != m_map.end(); ++iter)
  {
    os << " Imsi " << iter->first << " sinr " << 10*std::log10(iter->second);
  }
}
uint16_t
NgcX2UeImsiSinrUpdateHeader::GetRnti() const
{
	return m_rnti;
}

void
NgcX2UeImsiSinrUpdateHeader::SetRnti(uint16_t rnti){ //sjkang
	m_rnti = rnti;
}
uint16_t 
NgcX2UeImsiSinrUpdateHeader::GetSourceCellId () const
{
  return m_sourceCellId;
}
uint16_t
NgcX2UeImsiSinrUpdateHeader::GetSecondCellId () const
{
  return m_secondCellId;
}
void
NgcX2UeImsiSinrUpdateHeader::SetSourceCellId(uint16_t cellId)
{
  m_sourceCellId = cellId;
}
void
NgcX2UeImsiSinrUpdateHeader::SetSecondCellId(uint16_t cellId)
{
 m_secondCellId = cellId;
}
std::map <uint64_t, double>
NgcX2UeImsiSinrUpdateHeader::GetUeImsiSinrMap () const
{
  return m_map;
}

void
NgcX2UeImsiSinrUpdateHeader::SetUeImsiSinrMap (std::map <uint64_t, double> map)
{
  m_map = map;

  std::map <uint64_t, double>::size_type sz = m_map.size ();
  m_headerLength += sz * 16;
  m_numberOfIes += sz;
}

uint32_t
NgcX2UeImsiSinrUpdateHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2UeImsiSinrUpdateHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

uint64_t 
NgcX2UeImsiSinrUpdateHeader::pack754(long double f)
{
  uint16_t bits = 64;
  uint16_t expbits = 11;
  long double fnorm;
  int shift;
  long long sign, exp, significand;
  unsigned significandbits = bits - expbits - 1; // -1 for sign bit

  if (f == 0.0) return 0; // get this special case out of the way

  // check sign and begin normalization
  if (f < 0) { sign = 1; fnorm = -f; }
  else { sign = 0; fnorm = f; }

  // get the normalized form of f and track the exponent
  shift = 0;
  while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
  while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
  fnorm = fnorm - 1.0;

  // calculate the binary form (non-float) of the significand data
  significand = fnorm * ((1LL<<significandbits) + 0.5f);

  // get the biased exponent
  exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

  // return the final answer
  return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

long double 
NgcX2UeImsiSinrUpdateHeader::unpack754(uint64_t i)
{
  uint16_t bits = 64;
  uint16_t expbits = 11;
  long double result;
  long long shift;
  unsigned bias;
  unsigned significandbits = bits - expbits - 1; // -1 for sign bit

  if (i == 0) return 0.0;

  // pull the significand
  result = (i&((1LL<<significandbits)-1)); // mask
  result /= (1LL<<significandbits); // convert back to float
  result += 1.0f; // add the one back on

  // deal with the exponent
  bias = (1<<(expbits-1)) - 1;
  shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
  while(shift > 0) { result *= 2.0; shift--; }
  while(shift < 0) { result /= 2.0; shift++; }

  // sign it
  result *= (i>>(bits-1))&1? -1.0: 1.0;

  return result;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2ConnectionSwitchHeader);

NgcX2ConnectionSwitchHeader::NgcX2ConnectionSwitchHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (2 + 1 + 1),
    m_mmWaveRnti (0xfffa),
    m_drbid (0xfa),
    m_useMmWaveConnection (0)
{

}

NgcX2ConnectionSwitchHeader::~NgcX2ConnectionSwitchHeader ()
{
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_mmWaveRnti          = 0xfffb;
  m_drbid = 0xfb;
  m_useMmWaveConnection = 0;
}

TypeId
NgcX2ConnectionSwitchHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2ConnectionSwitchHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2ConnectionSwitchHeader> ()
  ;
  return tid;
}

TypeId
NgcX2ConnectionSwitchHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2ConnectionSwitchHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcX2ConnectionSwitchHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteHtonU16 (m_mmWaveRnti);
  i.WriteU8 (m_drbid);
  i.WriteU8 (m_useMmWaveConnection);
}

uint32_t
NgcX2ConnectionSwitchHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_mmWaveRnti = i.ReadNtohU16 ();
  m_drbid = i.ReadU8();
  m_useMmWaveConnection = (bool)i.ReadU8 ();
  m_numberOfIes = 3;
  m_headerLength = 4;

  return GetSerializedSize ();
}

void
NgcX2ConnectionSwitchHeader::Print (std::ostream &os) const
{
  os << "m_mmWaveRnti = " << m_mmWaveRnti;
  os << " m_useMmWaveConnection = " << m_useMmWaveConnection;
  os << " m_drbid = " << (uint16_t)m_drbid;
}

uint16_t
NgcX2ConnectionSwitchHeader::GetMmWaveRnti () const
{
  return m_mmWaveRnti;
}

void
NgcX2ConnectionSwitchHeader::SetMmWaveRnti (uint16_t rnti)
{
  m_mmWaveRnti = rnti;
}

bool
NgcX2ConnectionSwitchHeader::GetUseMmWaveConnection () const
{
  return m_useMmWaveConnection;
}

void
NgcX2ConnectionSwitchHeader::SetUseMmWaveConnection (bool useMmWaveConnection)
{
  m_useMmWaveConnection = useMmWaveConnection;
}

uint8_t
NgcX2ConnectionSwitchHeader::GetDrbid () const
{
  return m_drbid;
}

void
NgcX2ConnectionSwitchHeader::SetDrbid (uint8_t bid)
{
  m_drbid = bid;
}

uint32_t
NgcX2ConnectionSwitchHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcX2ConnectionSwitchHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}


} // namespace ns3
