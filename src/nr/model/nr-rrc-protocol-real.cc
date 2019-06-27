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
 * Authors: Nicola Baldo <nbaldo@cttc.es>
 *          Lluis Parcerisa <lparcerisa@cttc.cat>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/nstime.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/simulator.h>

#include "nr-rrc-protocol-real.h"
#include "nr-ue-rrc.h"
#include "nr-enb-rrc.h"
#include "nr-enb-net-device.h"
#include "nr-ue-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrRrcProtocolReal");

const Time RRC_REAL_MSG_DELAY = MicroSeconds (500); 

NS_OBJECT_ENSURE_REGISTERED (NrUeRrcProtocolReal);

NrUeRrcProtocolReal::NrUeRrcProtocolReal ()
  :  m_ueRrcSapProvider (0),
    m_enbRrcSapProvider (0)
{
  m_ueRrcSapUser = new MemberNrUeRrcSapUser<NrUeRrcProtocolReal> (this);
  m_completeSetupParameters.srb0SapUser = new NrRlcSpecificNrRlcSapUser<NrUeRrcProtocolReal> (this);
  m_completeSetupParameters.srb1SapUser = new NrPdcpSpecificNrPdcpSapUser<NrUeRrcProtocolReal> (this);    
  std::cout << " hi-  " <<std::endl;
}
void
NrUeRrcProtocolReal::SendNrAssi(NgcX2Sap::AssistantInformationForSplitting info){

}
NrUeRrcProtocolReal::~NrUeRrcProtocolReal ()
{
}
void
NrUeRrcProtocolReal::DoReceiveNrAssistantInfo(NgcX2Sap::AssistantInformationForSplitting info){

}
void
NrUeRrcProtocolReal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ueRrcSapUser;
  delete m_completeSetupParameters.srb0SapUser;
  delete m_completeSetupParameters.srb1SapUser;
  m_rrc = 0;
}

TypeId
NrUeRrcProtocolReal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrUeRrcProtocolReal")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddConstructor<NrUeRrcProtocolReal> ()
  ;
  return tid;
}

void 
NrUeRrcProtocolReal::SetNrUeRrcSapProvider (NrUeRrcSapProvider* p)
{
  m_ueRrcSapProvider = p;
}

NrUeRrcSapUser* 
NrUeRrcProtocolReal::GetNrUeRrcSapUser ()
{
  return m_ueRrcSapUser;
}

void 
NrUeRrcProtocolReal::SetUeRrc (Ptr<NrUeRrc> rrc)
{
  m_rrc = rrc;
}

void 
NrUeRrcProtocolReal::DoSetup (NrUeRrcSapUser::SetupParameters params)
{
  NS_LOG_FUNCTION (this);

  m_setupParameters.srb0SapProvider = params.srb0SapProvider;
  m_setupParameters.srb1SapProvider = params.srb1SapProvider; 
  m_ueRrcSapProvider->CompleteSetup (m_completeSetupParameters);
}

// jhlim: Send to Enb.
void
NrUeRrcProtocolReal::DoSendRrcIdentityResponse (NrRrcSap::RrcIdentityResponse msg)
{
  m_rnti = m_rrc->GetRnti ();
  //SetEnbRrcSapProvider ();
  Simulator::Schedule (RRC_REAL_MSG_DELAY,
  					  &NrEnbRrcSapProvider::RecvRrcIdentityResponse,
					  m_enbRrcSapProvider,
					  m_rnti,
					  msg);
}
void
NrUeRrcProtocolReal::DoSendRrcRegistrationComplete (NrRrcSap::RrcRegistrationComplete msg)
{
  m_rnti = m_rrc->GetRnti ();
  //SetEnbRrcSapProvider ();
  Simulator::Schedule (RRC_REAL_MSG_DELAY,
  					  &NrEnbRrcSapProvider::RecvRrcRegistrationComplete,
					  m_enbRrcSapProvider,
					  m_rnti,
					  msg);
}
void 
NrUeRrcProtocolReal::DoSendRrcConnectionRequest (NrRrcSap::RrcConnectionRequest msg)
{
  // initialize the RNTI and get the EnbNrRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();
  Simulator::Schedule (RRC_REAL_MSG_DELAY, 
                       &NrEnbRrcSapProvider::RecvRrcConnectionRequest,
                       m_enbRrcSapProvider,
                       m_rnti, 
                       msg);

  // real RRC code
  // Ptr<Packet> packet = Create<Packet> ();

  // RrcConnectionRequestHeader rrcConnectionRequestHeader;
  // rrcConnectionRequestHeader.SetMessage (msg);

  // packet->AddHeader (rrcConnectionRequestHeader);

  // NrRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  // transmitPdcpPduParameters.pdcpPdu = packet;
  // transmitPdcpPduParameters.rnti = m_rnti;
  // transmitPdcpPduParameters.lcid = 0;

  // m_setupParameters.srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
NrUeRrcProtocolReal::DoSendRrcConnectionSetupCompleted (NrRrcSap::RrcConnectionSetupCompleted msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionSetupCompleteHeader rrcConnectionSetupCompleteHeader;
  rrcConnectionSetupCompleteHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionSetupCompleteHeader);

  NrPdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;

  if (m_setupParameters.srb1SapProvider)
    {
      m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
    }
}

void 
NrUeRrcProtocolReal::DoSendRrcConnectionReconfigurationCompleted (NrRrcSap::RrcConnectionReconfigurationCompleted msg)
{
  // re-initialize the RNTI and get the EnbNrRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReconfigurationCompleteHeader rrcConnectionReconfigurationCompleteHeader;
  rrcConnectionReconfigurationCompleteHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReconfigurationCompleteHeader);

  NrPdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;
  NS_LOG_INFO("Tx RRC Connection reconf completed");
  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void 
NrUeRrcProtocolReal::DoSendMeasurementReport (NrRrcSap::MeasurementReport msg)
{
  // re-initialize the RNTI and get the EnbNrRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Ptr<Packet> packet = Create<Packet> ();

  MeasurementReportHeader measurementReportHeader;
  measurementReportHeader.SetMessage (msg);

  packet->AddHeader (measurementReportHeader);

  NrPdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void 
NrUeRrcProtocolReal::DoSendNotifySecondaryCellConnected (uint16_t mmWaveRnti, uint16_t mmWaveCellId)
{
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Ptr<Packet> packet = Create<Packet> ();

  RrcNotifySecondaryConnectedHeader rrcNotifyHeader;
  rrcNotifyHeader.SetMessage (mmWaveCellId, mmWaveRnti);

  packet->AddHeader (rrcNotifyHeader);

  NrPdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}


void 
NrUeRrcProtocolReal::DoSendRrcConnectionReestablishmentRequest (NrRrcSap::RrcConnectionReestablishmentRequest msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReestablishmentRequestHeader rrcConnectionReestablishmentRequestHeader;
  rrcConnectionReestablishmentRequestHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentRequestHeader);

  NrRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = m_rnti;
  transmitPdcpPduParameters.lcid = 0;

  m_setupParameters.srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
NrUeRrcProtocolReal::DoSendRrcConnectionReestablishmentComplete (NrRrcSap::RrcConnectionReestablishmentComplete msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReestablishmentCompleteHeader rrcConnectionReestablishmentCompleteHeader;
  rrcConnectionReestablishmentCompleteHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentCompleteHeader);

  NrPdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}


void 
NrUeRrcProtocolReal::SetEnbRrcSapProvider ()
{
  uint16_t cellId = m_rrc->GetCellId ();

  // walk list of all nodes to get the peer eNB
  Ptr<NrEnbNetDevice> enbDev;
  NodeList::Iterator listEnd = NodeList::End ();
  bool found = false;
  for (NodeList::Iterator i = NodeList::Begin (); 
       (i != listEnd) && (!found); 
       ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; 
           (j < nDevs) && (!found);
           j++)
        {
          enbDev = node->GetDevice (j)->GetObject <NrEnbNetDevice> ();
          if (enbDev == 0)
            {
              continue;
            }
          else
            {
              if (enbDev->GetCellId () == cellId)
                {
                  found = true;
                  break;
                }
            }
        }
    }
  NS_ASSERT_MSG (found, " Unable to find eNB with CellId =" << cellId);
  m_enbRrcSapProvider = enbDev->GetRrc ()->GetNrEnbRrcSapProvider ();
  Ptr<NrEnbRrcProtocolReal> enbRrcProtocolReal = enbDev->GetRrc ()->GetObject<NrEnbRrcProtocolReal> ();
  enbRrcProtocolReal->SetUeRrcSapProvider (m_rnti, m_ueRrcSapProvider);
}

void
NrUeRrcProtocolReal::DoReceivePdcpPdu (Ptr<Packet> p)
{
  // Get type of message received
  RrcDlCcchMessage rrcDlCcchMessage;
  p->PeekHeader (rrcDlCcchMessage);

  // Declare possible headers to receive
  RrcConnectionReestablishmentHeader rrcConnectionReestablishmentHeader;
  RrcConnectionReestablishmentRejectHeader rrcConnectionReestablishmentRejectHeader;
  RrcConnectionSetupHeader rrcConnectionSetupHeader;
  RrcConnectionRejectHeader rrcConnectionRejectHeader;
  RrcConnectToMmWaveHeader rrcConnectToMmWaveHeader;

  // Declare possible messages
  NrRrcSap::RrcConnectionReestablishment rrcConnectionReestablishmentMsg;
  NrRrcSap::RrcConnectionReestablishmentReject rrcConnectionReestablishmentRejectMsg;
  NrRrcSap::RrcConnectionSetup rrcConnectionSetupMsg;
  NrRrcSap::RrcConnectionReject rrcConnectionRejectMsg;
  NS_LOG_FUNCTION (this <<rrcDlCcchMessage.GetMessageType () );
  // Deserialize packet and call member recv function with appropiate structure
  switch ( rrcDlCcchMessage.GetMessageType () )
    {
    case 0:
      // RrcConnectionReestablishment
      p->RemoveHeader (rrcConnectionReestablishmentHeader);
      rrcConnectionReestablishmentMsg = rrcConnectionReestablishmentHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionReestablishment (rrcConnectionReestablishmentMsg);
      break;
    case 1:
      // RrcConnectionReestablishmentReject
      p->RemoveHeader (rrcConnectionReestablishmentRejectHeader);
      rrcConnectionReestablishmentRejectMsg = rrcConnectionReestablishmentRejectHeader.GetMessage ();
      // m_ueRrcSapProvider->RecvRrcConnectionReestablishmentReject (rrcConnectionReestablishmentRejectMsg);
      break;
    case 2:
      // RrcConnectionReject
      p->RemoveHeader (rrcConnectionRejectHeader);
      rrcConnectionRejectMsg = rrcConnectionRejectHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionReject (rrcConnectionRejectMsg);
      break;
    case 3:
      // RrcConnectionSetup
      p->RemoveHeader (rrcConnectionSetupHeader);
      rrcConnectionSetupMsg = rrcConnectionSetupHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionSetup (rrcConnectionSetupMsg);

      break;
    case 4:
      // RrcConnectToMmWave
      p->RemoveHeader (rrcConnectToMmWaveHeader);
      uint16_t mmWaveCellId = rrcConnectToMmWaveHeader.GetMessage ();
      uint16_t mmWaveCellId_2 = rrcConnectToMmWaveHeader.GetMessage_secondMmWaveCellId(); //sjkang
      std::cout << "calllinngg -----------" <<std::endl;
      m_ueRrcSapProvider->RecvRrcConnectToMmWave(mmWaveCellId, mmWaveCellId_2); //sjkang
      break;
    }
}

void
NrUeRrcProtocolReal::DoReceivePdcpSdu (NrPdcpSapUser::ReceivePdcpSduParameters params)
{
  // Get type of message received
  RrcDlDcchMessage rrcDlDcchMessage;
  params.pdcpSdu->PeekHeader (rrcDlDcchMessage);

  // Declare possible headers to receive
  RrcConnectionReconfigurationHeader rrcConnectionReconfigurationHeader;
  RrcConnectionReleaseHeader rrcConnectionReleaseHeader;
  RrcConnectionSwitchHeader rrcSwitchHeader;

  // Declare possible messages to receive
  NrRrcSap::RrcConnectionReconfiguration rrcConnectionReconfigurationMsg;
  NrRrcSap::RrcConnectionRelease rrcConnectionReleaseMsg;
  NrRrcSap::RrcConnectionSwitch rrcConnectionSwitchMsg;
  // Deserialize packet and call member recv function with appropiate structure
  switch ( rrcDlDcchMessage.GetMessageType () )
    {
    case 4:
      params.pdcpSdu->RemoveHeader (rrcConnectionReconfigurationHeader);
      rrcConnectionReconfigurationMsg = rrcConnectionReconfigurationHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionReconfiguration (rrcConnectionReconfigurationMsg);
      break;
    case 5:
      params.pdcpSdu->RemoveHeader (rrcConnectionReleaseHeader);
      rrcConnectionReleaseMsg = rrcConnectionReleaseHeader.GetMessage ();
      //m_ueRrcSapProvider->RecvRrcConnectionRelease (rrcConnectionReleaseMsg);
      break;
    case 6:
      params.pdcpSdu->RemoveHeader (rrcSwitchHeader);
      rrcConnectionSwitchMsg = rrcSwitchHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionSwitch (rrcConnectionSwitchMsg);
      break;
    }
}

NS_OBJECT_ENSURE_REGISTERED (NrEnbRrcProtocolReal);

NrEnbRrcProtocolReal::NrEnbRrcProtocolReal ()
  :  m_enbRrcSapProvider (0)
{
  NS_LOG_FUNCTION (this);
  m_enbRrcSapUser = new MemberNrEnbRrcSapUser<NrEnbRrcProtocolReal> (this);
}

NrEnbRrcProtocolReal::~NrEnbRrcProtocolReal ()
{
  NS_LOG_FUNCTION (this);
}
void
NrEnbRrcProtocolReal::SendNrAssi(NgcX2Sap::AssistantInformationForSplitting info){

}
void
NrEnbRrcProtocolReal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_enbRrcSapUser;
  for (std::map<uint16_t, NrEnbRrcSapProvider::CompleteSetupUeParameters>::iterator 
         it = m_completeSetupUeParametersMap.begin ();
       it != m_completeSetupUeParametersMap.end ();
       ++it)
    {     
      delete it->second.srb0SapUser;
      delete it->second.srb1SapUser;
    }
  m_completeSetupUeParametersMap.clear ();
}

TypeId
NrEnbRrcProtocolReal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrEnbRrcProtocolReal")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddConstructor<NrEnbRrcProtocolReal> ()
  ;
  return tid;
}

void 
NrEnbRrcProtocolReal::SetNrEnbRrcSapProvider (NrEnbRrcSapProvider* p)
{
  m_enbRrcSapProvider = p;
}

NrEnbRrcSapUser* 
NrEnbRrcProtocolReal::GetNrEnbRrcSapUser ()
{
  return m_enbRrcSapUser;
}

void 
NrEnbRrcProtocolReal::SetCellId (uint16_t cellId)
{
  m_cellId = cellId;
}

NrUeRrcSapProvider* 
NrEnbRrcProtocolReal::GetUeRrcSapProvider (uint16_t rnti)
{
  std::map<uint16_t, NrUeRrcSapProvider*>::const_iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  return it->second;
}

void 
NrEnbRrcProtocolReal::SetUeRrcSapProvider (uint16_t rnti, NrUeRrcSapProvider* p)
{
  std::map<uint16_t, NrUeRrcSapProvider*>::iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  it->second = p;
}

void 
NrEnbRrcProtocolReal::DoSetupUe (uint16_t rnti, NrEnbRrcSapUser::SetupUeParameters params)
{
  NS_LOG_FUNCTION (this << rnti);

  // // walk list of all nodes to get the peer UE RRC SAP Provider
  // Ptr<NrUeRrc> ueRrc;
  // NodeList::Iterator listEnd = NodeList::End ();
  // bool found = false;
  // for (NodeList::Iterator i = NodeList::Begin (); (i != listEnd) && (found == false); i++)
  //   {
  //     Ptr<Node> node = *i;
  //     int nDevs = node->GetNDevices ();
  //     for (int j = 0; j < nDevs; j++)
  //       {
  //         Ptr<NrUeNetDevice> ueDev = node->GetDevice (j)->GetObject <NrUeNetDevice> ();
  //         if (!ueDev)
  //           {
  //             continue;
  //           }
  //         else
  //           {
  //             ueRrc = ueDev->GetRrc ();
  //             if ((ueRrc->GetRnti () == rnti) && (ueRrc->GetCellId () == m_cellId))
  //               {
  //              found = true;
  //              break;
  //               }
  //           }
  //       }
  //   }
  // NS_ASSERT_MSG (found , " Unable to find UE with RNTI=" << rnti << " cellId=" << m_cellId);
  // m_enbRrcSapProviderMap[rnti] = ueRrc->GetNrUeRrcSapProvider ();

  // just create empty entry, the UeRrcSapProvider will be set by the
  // ue upon connection request or connection reconfiguration
  // completed 
  m_enbRrcSapProviderMap[rnti] = 0;

  // Store SetupUeParameters
  m_setupUeParametersMap[rnti] = params;

  NrEnbRrcSapProvider::CompleteSetupUeParameters completeSetupUeParameters;
  std::map<uint16_t, NrEnbRrcSapProvider::CompleteSetupUeParameters>::iterator 
    csupIt = m_completeSetupUeParametersMap.find (rnti);
  if (csupIt == m_completeSetupUeParametersMap.end ())
    {
      // Create NrRlcSapUser, NrPdcpSapUser
      NrRlcSapUser* srb0SapUser = new RealProtocolRlcSapUser (this,rnti);
      NrPdcpSapUser* srb1SapUser = new NrPdcpSpecificNrPdcpSapUser<NrEnbRrcProtocolReal> (this);
      completeSetupUeParameters.srb0SapUser = srb0SapUser;
      completeSetupUeParameters.srb1SapUser = srb1SapUser;
      // Store NrRlcSapUser, NrPdcpSapUser
      m_completeSetupUeParametersMap[rnti] = completeSetupUeParameters;      
    }
  else
    {
      completeSetupUeParameters = csupIt->second;
    }
  m_enbRrcSapProvider->CompleteSetupUe (rnti, completeSetupUeParameters);
}

void 
NrEnbRrcProtocolReal::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::map<uint16_t, NrEnbRrcSapProvider::CompleteSetupUeParameters>::iterator 
    it = m_completeSetupUeParametersMap.find (rnti);
  NS_ASSERT (it != m_completeSetupUeParametersMap.end ());
  delete it->second.srb0SapUser;
  delete it->second.srb1SapUser;
  m_completeSetupUeParametersMap.erase (it);
  m_enbRrcSapProviderMap.erase (rnti);
  m_setupUeParametersMap.erase (rnti);
}

void 
NrEnbRrcProtocolReal::DoSendSystemInformation (NrRrcSap::SystemInformation msg)
{
    NS_LOG_FUNCTION (this << m_cellId);
  // walk list of all nodes to get UEs with this cellId
  Ptr<NrUeRrc> ueRrc;
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; ++j)
        {
          Ptr<NrUeNetDevice> ueDev = node->GetDevice (j)->GetObject <NrUeNetDevice> ();
          if (ueDev != 0)
            {
              Ptr<NrUeRrc> ueRrc = ueDev->GetRrc ();
              NS_LOG_LOGIC ("considering UE IMSI " << ueDev->GetImsi () << " that has cellId " << ueRrc->GetCellId ());
              if (ueRrc->GetCellId () == m_cellId)
                {
                  NS_LOG_LOGIC ("sending SI to IMSI " << ueDev->GetImsi ());
                  ueRrc->GetNrUeRrcSapProvider ()->RecvSystemInformation (msg);
                  Simulator::Schedule (RRC_REAL_MSG_DELAY, 
                                       &NrUeRrcSapProvider::RecvSystemInformation,
                                       ueRrc->GetNrUeRrcSapProvider (), 
                                       msg);
                }
            }
        }
    }
}

void 
NrEnbRrcProtocolReal::DoSendRrcConnectionSetup (uint16_t rnti, NrRrcSap::RrcConnectionSetup msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionSetupHeader rrcConnectionSetupHeader;
  rrcConnectionSetupHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionSetupHeader);

  NrRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;

  if (m_setupUeParametersMap.find (rnti) == m_setupUeParametersMap.end () )
    {
      NS_LOG_ERROR("RNTI not found in Enb setup parameters Map!");
    }
  else
    {
      NS_LOG_INFO("Queue RRC connection setup " << packet << " rnti " << rnti << " cellId " << m_cellId);
      m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
    }
}

void 
NrEnbRrcProtocolReal::DoSendRrcConnectionReject (uint16_t rnti, NrRrcSap::RrcConnectionReject msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionRejectHeader rrcConnectionRejectHeader;
  rrcConnectionRejectHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionRejectHeader);

  NrRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}


void 
NrEnbRrcProtocolReal::DoSendRrcConnectionSwitch (uint16_t rnti, NrRrcSap::RrcConnectionSwitch msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionSwitchHeader rrcSwitchHeader;
  rrcSwitchHeader.SetMessage (msg);

  packet->AddHeader (rrcSwitchHeader);

  NrPdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void 
NrEnbRrcProtocolReal::DoSendRrcConnectToMmWave (uint16_t rnti, uint16_t mmWaveId , uint16_t mmWaveId_2)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectToMmWaveHeader connectToMmWaveHeader;
  connectToMmWaveHeader.SetMessage(mmWaveId, mmWaveId_2);

  packet->AddHeader (connectToMmWaveHeader);

  NrRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
NrEnbRrcProtocolReal::DoSendRrcConnectionReconfiguration (uint16_t rnti, NrRrcSap::RrcConnectionReconfiguration msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReconfigurationHeader rrcConnectionReconfigurationHeader;
  rrcConnectionReconfigurationHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReconfigurationHeader);

  NrPdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}
// jhlim: Send to UE.
void
NrEnbRrcProtocolReal::DoSendRrcIdentityRequest (uint16_t rnti, NrRrcSap::RrcIdentityRequest msg)
{
  Ptr<Packet> packet = Create<Packet> ();

 RrcIdentityRequestHeader rrcIdentityRequestHeader;
 rrcIdentityRequestHeader.SetMessage (msg);

 packet->AddHeader (rrcIdentityRequestHeader);

 NrPdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
 transmitPdcpSduParameters.pdcpSdu = packet;
 transmitPdcpSduParameters.rnti = rnti;
 transmitPdcpSduParameters.lcid = 1;

 m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}
void
NrEnbRrcProtocolReal::DoSendRrcRegistrationAccept (uint16_t rnti, NrRrcSap::RrcRegistrationAccept msg)
{
  Ptr<Packet> packet = Create<Packet> ();

 RrcRegistrationAcceptHeader rrcRegistrationAcceptHeader;
 rrcRegistrationAcceptHeader.SetMessage (msg);

 packet->AddHeader (rrcRegistrationAcceptHeader);

 NrPdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
 transmitPdcpSduParameters.pdcpSdu = packet;
 transmitPdcpSduParameters.rnti = rnti;
 transmitPdcpSduParameters.lcid = 1;

 m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}
void 
NrEnbRrcProtocolReal::DoSendRrcConnectionReestablishment (uint16_t rnti, NrRrcSap::RrcConnectionReestablishment msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReestablishmentHeader rrcConnectionReestablishmentHeader;
  rrcConnectionReestablishmentHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentHeader);

  NrRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
NrEnbRrcProtocolReal::DoSendRrcConnectionReestablishmentReject (uint16_t rnti, NrRrcSap::RrcConnectionReestablishmentReject msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReestablishmentRejectHeader rrcConnectionReestablishmentRejectHeader;
  rrcConnectionReestablishmentRejectHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentRejectHeader);

  NrRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
NrEnbRrcProtocolReal::DoSendRrcConnectionRelease (uint16_t rnti, NrRrcSap::RrcConnectionRelease msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  RrcConnectionReleaseHeader rrcConnectionReleaseHeader;
  rrcConnectionReleaseHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReleaseHeader);

  NrPdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = rnti;
  transmitPdcpSduParameters.lcid = 1;

  m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void
NrEnbRrcProtocolReal::DoReceivePdcpPdu (uint16_t rnti, Ptr<Packet> p)
{
  // Get type of message received
  RrcUlCcchMessage rrcUlCcchMessage;
  p->PeekHeader (rrcUlCcchMessage);

  // Declare possible headers to receive
  RrcConnectionReestablishmentRequestHeader rrcConnectionReestablishmentRequestHeader;
  RrcConnectionRequestHeader rrcConnectionRequestHeader;

  // Deserialize packet and call member recv function with appropiate structure
  switch ( rrcUlCcchMessage.GetMessageType () )
    {
    case 0:
      p->RemoveHeader (rrcConnectionReestablishmentRequestHeader);
      NrRrcSap::RrcConnectionReestablishmentRequest rrcConnectionReestablishmentRequestMsg;
      rrcConnectionReestablishmentRequestMsg = rrcConnectionReestablishmentRequestHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionReestablishmentRequest (rnti,rrcConnectionReestablishmentRequestMsg);
      break;
    case 1:
      p->RemoveHeader (rrcConnectionRequestHeader);
      NrRrcSap::RrcConnectionRequest rrcConnectionRequestMsg;
      rrcConnectionRequestMsg = rrcConnectionRequestHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionRequest (rnti,rrcConnectionRequestMsg);
      break;
    }
}

void
NrEnbRrcProtocolReal::DoReceivePdcpSdu (NrPdcpSapUser::ReceivePdcpSduParameters params)
{
  // Get type of message received
  RrcUlDcchMessage rrcUlDcchMessage;
  params.pdcpSdu->PeekHeader (rrcUlDcchMessage);

  // Declare possible headers to receive
  MeasurementReportHeader measurementReportHeader;
  RrcConnectionReconfigurationCompleteHeader rrcConnectionReconfigurationCompleteHeader;
  RrcConnectionReestablishmentCompleteHeader rrcConnectionReestablishmentCompleteHeader;
  RrcConnectionSetupCompleteHeader rrcConnectionSetupCompleteHeader;
  RrcNotifySecondaryConnectedHeader rrcNotifyHeader;

  // Declare possible messages to receive
  NrRrcSap::MeasurementReport measurementReportMsg;
  NrRrcSap::RrcConnectionReconfigurationCompleted rrcConnectionReconfigurationCompleteMsg;
  NrRrcSap::RrcConnectionReestablishmentComplete rrcConnectionReestablishmentCompleteMsg;
  NrRrcSap::RrcConnectionSetupCompleted rrcConnectionSetupCompletedMsg;

  // Deserialize packet and call member recv function with appropiate structure
  switch ( rrcUlDcchMessage.GetMessageType () )
    {
    case 1:
      params.pdcpSdu->RemoveHeader (measurementReportHeader);
      measurementReportMsg = measurementReportHeader.GetMessage ();
      m_enbRrcSapProvider->RecvMeasurementReport (params.rnti,measurementReportMsg);
      break;
    case 2:
      params.pdcpSdu->RemoveHeader (rrcConnectionReconfigurationCompleteHeader);
      rrcConnectionReconfigurationCompleteMsg = rrcConnectionReconfigurationCompleteHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionReconfigurationCompleted (params.rnti,rrcConnectionReconfigurationCompleteMsg);
      break;
    case 3:
      params.pdcpSdu->RemoveHeader (rrcConnectionReestablishmentCompleteHeader);
      rrcConnectionReestablishmentCompleteMsg = rrcConnectionReestablishmentCompleteHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionReestablishmentComplete (params.rnti,rrcConnectionReestablishmentCompleteMsg);
      break;
    case 4:
      params.pdcpSdu->RemoveHeader (rrcConnectionSetupCompleteHeader);
      rrcConnectionSetupCompletedMsg = rrcConnectionSetupCompleteHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionSetupCompleted (params.rnti, rrcConnectionSetupCompletedMsg);
      break;
    case 5:
      params.pdcpSdu->RemoveHeader (rrcNotifyHeader);
      std::pair<uint16_t, uint16_t> rrcNotifyPair;
      rrcNotifyPair = rrcNotifyHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcSecondaryCellInitialAccessSuccessful (params.rnti, rrcNotifyPair.second, rrcNotifyPair.first);
      break;
    }
}

Ptr<Packet> 
NrEnbRrcProtocolReal::DoEncodeHandoverPreparationInformation (NrRrcSap::HandoverPreparationInfo msg)
{
  HandoverPreparationInfoHeader h;
  h.SetMessage (msg);

  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

NrRrcSap::HandoverPreparationInfo 
NrEnbRrcProtocolReal::DoDecodeHandoverPreparationInformation (Ptr<Packet> p)
{
  HandoverPreparationInfoHeader h;
  p->RemoveHeader (h);
  NrRrcSap::HandoverPreparationInfo msg = h.GetMessage ();
  return msg;
}

Ptr<Packet> 
NrEnbRrcProtocolReal::DoEncodeHandoverCommand (NrRrcSap::RrcConnectionReconfiguration msg)
{
  RrcConnectionReconfigurationHeader h;
  h.SetMessage (msg);
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

NrRrcSap::RrcConnectionReconfiguration
NrEnbRrcProtocolReal::DoDecodeHandoverCommand (Ptr<Packet> p)
{
  RrcConnectionReconfigurationHeader h;
  p->RemoveHeader (h);
  NrRrcSap::RrcConnectionReconfiguration msg = h.GetMessage ();
  return msg;
}
void
NrEnbRrcProtocolReal::DoReceiveNrAssistantInfo(NgcX2Sap::AssistantInformationForSplitting info){

}
//////////////////////////////////////////////////////

RealProtocolRlcSapUser::RealProtocolRlcSapUser (NrEnbRrcProtocolReal* pdcp, uint16_t rnti)
  : m_pdcp (pdcp),
    m_rnti (rnti)
{
}

RealProtocolRlcSapUser::RealProtocolRlcSapUser ()
{
}

void
RealProtocolRlcSapUser::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdcpPdu (m_rnti, p);
}
void
RealProtocolRlcSapUser:: SendNrAssi(NgcX2Sap::AssistantInformationForSplitting info){

}

} // namespace ns3
