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
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */


#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ns3/nr-rlc.h"
#include "ns3/nr-rlc-tag.h"
#include "nr-mac-sap.h"
#include "ns3/nr-rlc-sap.h"
// #include "ff-mac-sched-sap.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrRlc");


///////////////////////////////////////


NrRlcSpecificNrMacSapUser::NrRlcSpecificNrMacSapUser (NrRlc* rlc)
  : m_rlc (rlc)
{
}

NrRlcSpecificNrMacSapUser::NrRlcSpecificNrMacSapUser ()
{
}

void
NrRlcSpecificNrMacSapUser::NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId)
{
  m_rlc->DoNotifyTxOpportunity (bytes, layer, harqId);
}

void
NrRlcSpecificNrMacSapUser::NotifyHarqDeliveryFailure ()
{
  m_rlc->DoNotifyHarqDeliveryFailure ();
}

void
NrRlcSpecificNrMacSapUser::NotifyHarqDeliveryFailure (uint8_t harqId)
{
  m_rlc->DoNotifyHarqDeliveryFailure (harqId);
}

void
NrRlcSpecificNrMacSapUser::ReceivePdu (Ptr<Packet> p)
{
  m_rlc->DoReceivePdu (p);
}


///////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NrRlc);

NrRlc::NrRlc ()
  : m_rlcSapUser (0),
	m_rlc_Assistant_User(0),
    m_macSapProvider (0),
    m_rnti (0),
    m_lcid (0),
    isMc(false) // TODO refactor this!!

{
  NS_LOG_FUNCTION (this);
  m_rlcSapProvider = new NrRlcSpecificNrRlcSapProvider<NrRlc> (this);
  m_ngcX2RlcUser = new NgcX2RlcSpecificUser<NrRlc> (this);
  m_macSapUser = new NrRlcSpecificNrMacSapUser (this);
 // m_drbId=0;
//  std::cout << "sjkang1114" << "\t " << this <<"\t"<<"this is rlc address" << std::endl;
}

NrRlc::~NrRlc ()
{
  NS_LOG_FUNCTION (this);
}

TypeId NrRlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrRlc")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddTraceSource ("TxPDU",
                     "PDU transmission notified to the MAC.",
                     MakeTraceSourceAccessor (&NrRlc::m_txPdu),
                     "ns3::NrRlc::NotifyTxTracedCallback")
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&NrRlc::m_rxPdu),
                     "ns3::NrRlc::ReceiveTracedCallback")
   .AddTraceSource ("TxCompletedCallback",
                     "PDU acked.",
                     MakeTraceSourceAccessor (&NrRlc::m_txCompletedCallback),
                     "ns3::NrRlc::RetransmissionCountCallback")
    ;
  return tid;
}

void
NrRlc::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete (m_rlcSapProvider);
  delete (m_macSapUser);
}

void
NrRlc::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << (uint32_t) rnti);
  m_rnti = rnti;
}
void
NrRlc::SetDrbId(uint8_t drbid){ //sjkang1115
	m_drbId = drbid;//sjkang1115

}
void
NrRlc::SetStreamForQueueStatistics(std::ofstream *stream){
	measuringQusizeQueueDelayStream = stream; //sjkang1116
}
void
NrRlc::SetLcId (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << (uint32_t) lcId);
  m_lcid = lcId;
}

void
NrRlc::SetNrRlcSapUser (NrRlcSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_rlcSapUser = s;
}
void
NrRlc::SetNrRlcAssistantSapUser(NrRlcSapUser *s){
	//std::cout << "setting "<< s <<std::endl;
	m_rlc_Assistant_User = s;
}
NrRlcSapProvider*
NrRlc::GetNrRlcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_rlcSapProvider;
}

void
NrRlc::SetNrMacSapProvider (NrMacSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_macSapProvider = s;
}

NrMacSapUser*
NrRlc::GetNrMacSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_macSapUser;
}

void
NrRlc::DoNotifyHarqDeliveryFailure (uint8_t harqId)
{
	NS_LOG_FUNCTION (this);
}

void
NrRlc::SetUeDataParams(NgcX2Sap::UeDataParams params)
{
  isMc = true;
  m_ueDataParams = params;
}

void 
NrRlc::SetNgcX2RlcProvider (NgcX2RlcProvider * s)
{
  m_ngcX2RlcProvider = s;
 //std::cout << "setting ngcX2RlcProvider in nrRlc layer sjkang1114   --->" <<this << std::endl;
}

NgcX2RlcUser* 
NrRlc::GetNgcX2RlcUser ()
{
  return m_ngcX2RlcUser;
}
void
NrRlc:: DoRequestAssistantInfo() {//sjkang
}

////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NrRlcSm);

NrRlcSm::NrRlcSm ()
{
  NS_LOG_FUNCTION (this);
}

NrRlcSm::~NrRlcSm ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrRlcSm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrRlcSm")
    .SetParent<NrRlc> ()
    .SetGroupName("Nr")
    .AddConstructor<NrRlcSm> ()
    ;
  return tid;
}

void
NrRlcSm::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  ReportBufferStatus ();
}

void
NrRlcSm::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  NrRlc::DoDispose ();
}

void
NrRlcSm::DoTransmitPdcpPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
}

void
NrRlcSm::DoReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  // RLC Performance evaluation
  RlcTag rlcTag;
  Time delay;
  if (p->FindFirstMatchingByteTag(rlcTag))
    {
      delay = Simulator::Now() - rlcTag.GetSenderTimestamp ();
    }
  NS_LOG_LOGIC (" RNTI=" << m_rnti 
                << " LCID=" << (uint32_t) m_lcid 
                << " size=" << p->GetSize () 
                << " delay=" << delay.GetNanoSeconds ());
  m_rxPdu(m_rnti, m_lcid, p->GetSize (), delay.GetNanoSeconds () );
}

void
NrRlcSm::DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId)
{
  NS_LOG_FUNCTION (this << bytes);
  NrMacSapProvider::TransmitPduParameters params;
  params.pdu = Create<Packet> (bytes);
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  params.layer = layer;
  params.harqProcessId = harqId;

  // RLC Performance evaluation
  RlcTag tag (Simulator::Now());
  params.pdu->AddByteTag (tag);
  NS_LOG_LOGIC (" RNTI=" << m_rnti 
                << " LCID=" << (uint32_t) m_lcid 
                << " size=" << bytes);
  m_txPdu(m_rnti, m_lcid, bytes);

  m_macSapProvider->TransmitPdu (params);
  ReportBufferStatus ();
}

void
NrRlcSm::DoNotifyHarqDeliveryFailure ()
{
  NS_LOG_FUNCTION (this);
}

void
NrRlcSm::ReportBufferStatus ()
{
  NS_LOG_FUNCTION (this);
  NrMacSapProvider::ReportBufferStatusParameters p;
  p.rnti = m_rnti;
  p.lcid = m_lcid;
  p.txQueueSize = 1000000;  // mmWave module: Arbitrarily changed full-buffer BSR to report 1MB available each subframe
  p.txQueueHolDelay = 10;
  p.retxQueueSize = 0;
  p.retxQueueHolDelay = 0;
  p.statusPduSize = 0;
  m_macSapProvider->ReportBufferStatus (p);
}

void 
NrRlcSm::DoSendMcPdcpSdu(NgcX2Sap::UeDataParams params)
{
  NS_LOG_FUNCTION(this);
  NS_FATAL_ERROR("Not supported");
}


void
NrRlcSm::CalculatePathThroughput (std::ofstream *streamPathThroughput) // woody
{
  NS_FATAL_ERROR ("Not implemented yet");
}

void
NrRlcSm::DoRequestAssistantInfo(){

}
//////////////////////////////////////////

// NrRlcTm::~NrRlcTm ()
// {
// }

//////////////////////////////////////////

// NrRlcUm::~NrRlcUm ()
// {
// }

//////////////////////////////////////////

// NrRlcAm::~NrRlcAm ()
// {
// }


} // namespace ns3
