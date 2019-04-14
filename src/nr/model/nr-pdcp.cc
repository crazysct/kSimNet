/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 */

#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ns3/nr-pdcp.h"
#include "ns3/nr-pdcp-header.h"
#include "ns3/nr-pdcp-sap.h"
#include "ns3/nr-pdcp-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrPdcp");

class NrPdcpSpecificNrRlcSapUser : public NrRlcSapUser
{
public:
  NrPdcpSpecificNrRlcSapUser (NrPdcp* pdcp);

  // Interface provided to lower RLC entity (implemented from NrRlcSapUser)
  virtual void ReceivePdcpPdu (Ptr<Packet> p);
 virtual void  SendNrAssi(NgcX2Sap::AssistantInformationForSplitting info);
private:
  NrPdcpSpecificNrRlcSapUser ();
  NrPdcp* m_pdcp;
};

NrPdcpSpecificNrRlcSapUser::NrPdcpSpecificNrRlcSapUser (NrPdcp* pdcp)
  : m_pdcp (pdcp)
{
}

NrPdcpSpecificNrRlcSapUser::NrPdcpSpecificNrRlcSapUser ()
{
}

void
NrPdcpSpecificNrRlcSapUser::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdu (p);
}
void
NrPdcpSpecificNrRlcSapUser:: SendNrAssi(NgcX2Sap::AssistantInformationForSplitting info){

}
///////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NrPdcp);

NrPdcp::NrPdcp ()
  : m_pdcpSapUser (0),
    m_rlcSapProvider (0),
    m_rnti (0),
    m_lcid (0),
    m_txSequenceNumber (0),
    m_rxSequenceNumber (0)
{
  NS_LOG_FUNCTION (this);
  m_pdcpSapProvider = new NrPdcpSpecificNrPdcpSapProvider<NrPdcp> (this);
  m_rlcSapUser = new NrPdcpSpecificNrRlcSapUser (this);
  //m_rlc_AssistSapUser = new NrPdcpSpecificNrRlcSapUser (this);

}

NrPdcp::~NrPdcp ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrPdcp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrPdcp")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddTraceSource ("TxPDU",
                     "PDU transmission notified to the RLC.",
                     MakeTraceSourceAccessor (&NrPdcp::m_txPdu),
                     "ns3::NrPdcp::PduTxTracedCallback")
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&NrPdcp::m_rxPdu),
                     "ns3::NrPdcp::PduRxTracedCallback")
    ;
  return tid;
}

void
NrPdcp::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete (m_pdcpSapProvider);
  delete (m_rlcSapUser);
}


void
NrPdcp::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << (uint32_t) rnti);
  m_rnti = rnti;
}

void
NrPdcp::SetLcId (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << (uint32_t) lcId);
  m_lcid = lcId;
}

void
NrPdcp::SetNrPdcpSapUser (NrPdcpSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_pdcpSapUser = s;
}

NrPdcpSapProvider*
NrPdcp::GetNrPdcpSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_pdcpSapProvider;
}

void
NrPdcp::SetNrRlcSapProvider (NrRlcSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_rlcSapProvider = s;
}

NrRlcSapUser*
NrPdcp::GetNrRlcSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_rlcSapUser;
}

NrPdcp::Status 
NrPdcp::GetStatus ()
{
  Status s;
  s.txSn = m_txSequenceNumber;
  s.rxSn = m_rxSequenceNumber;
  return s;
}

void 
NrPdcp::SetStatus (Status s)
{
  m_txSequenceNumber = s.txSn;
  m_rxSequenceNumber = s.rxSn;
}

////////////////////////////////////////

void
NrPdcp::DoTransmitPdcpSdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  NrPdcpHeader pdcpHeader;
  pdcpHeader.SetSequenceNumber (m_txSequenceNumber);
//std::cout << "Nr pdcp is used " <<std::endl;
  m_txSequenceNumber++;
  if (m_txSequenceNumber > m_maxPdcpSn)
    {
      m_txSequenceNumber = 0;
    }

  pdcpHeader.SetDcBit (NrPdcpHeader::DATA_PDU);

  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);
  p->AddHeader (pdcpHeader);

  // Sender timestamp
  PdcpTag pdcpTag (Simulator::Now ());
  p->AddByteTag (pdcpTag);
  m_txPdu (m_rnti, m_lcid, p->GetSize ());

  NrRlcSapProvider::TransmitPdcpPduParameters params;
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  params.pdcpPdu = p;

  m_rlcSapProvider->TransmitPdcpPdu (params);
}

void
NrPdcp::DoReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  // Receiver timestamp
  PdcpTag pdcpTag;
  Time delay;
  if (p->FindFirstMatchingByteTag (pdcpTag))
    {
      delay = Simulator::Now() - pdcpTag.GetSenderTimestamp ();
    }
  m_rxPdu(m_rnti, m_lcid, p->GetSize (), delay.GetNanoSeconds ());

  NrPdcpHeader pdcpHeader;
  p->RemoveHeader (pdcpHeader);
  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);

  m_rxSequenceNumber = pdcpHeader.GetSequenceNumber () + 1;
  if (m_rxSequenceNumber > m_maxPdcpSn)
    {
      m_rxSequenceNumber = 0;
    }

  NrPdcpSapUser::ReceivePdcpSduParameters params;
  params.pdcpSdu = p;
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  m_pdcpSapUser->ReceivePdcpSdu (params);
}
void
NrPdcp::DoReceiveNrAssistantInfo(NgcX2Sap::AssistantInformationForSplitting){ //sjkang
}

} // namespace ns3
