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
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/ngc-gtpu-header.h"
#include "ns3/ngc-x2-tag.h"
#include "ns3/nr-pdcp-tag.h"

#include "ns3/ngc-x2-header.h"
#include "ns3/ngc-x2.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NgcX2");

NrX2IfaceInfo::NrX2IfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket, Ptr<Socket> localUserPlaneSocket)
{
  m_remoteIpAddr = remoteIpAddr;
  m_localCtrlPlaneSocket = localCtrlPlaneSocket;
  m_localUserPlaneSocket = localUserPlaneSocket;
}

NrX2IfaceInfo::~NrX2IfaceInfo (void)
{
  m_localCtrlPlaneSocket = 0;
  m_localUserPlaneSocket = 0;
}

NrX2IfaceInfo& 
NrX2IfaceInfo::operator= (const NrX2IfaceInfo& value)
{
  NS_LOG_FUNCTION (this);
  m_remoteIpAddr = value.m_remoteIpAddr;
  m_localCtrlPlaneSocket = value.m_localCtrlPlaneSocket;
  m_localUserPlaneSocket = value.m_localUserPlaneSocket;
  return *this;
}

///////////////////////////////////////////

NrX2CellInfo::NrX2CellInfo (uint16_t localCellId, uint16_t remoteCellId)
{
  m_localCellId = localCellId;
  m_remoteCellId = remoteCellId;
}

NrX2CellInfo::~NrX2CellInfo (void)
{
  m_localCellId = 0;
  m_remoteCellId = 0;
}

NrX2CellInfo& 
NrX2CellInfo::operator= (const NrX2CellInfo& value)
{
  NS_LOG_FUNCTION (this);
  m_localCellId = value.m_localCellId;
  m_remoteCellId = value.m_remoteCellId;
  return *this;
}

///////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcX2);

NgcX2::NgcX2 ()
  : m_x2cUdpPort (4444),
    m_x2uUdpPort (2152)
{
  NS_LOG_FUNCTION (this);

  m_x2SapProvider = new NgcX2SpecificNgcX2SapProvider<NgcX2> (this);
  m_x2PdcpProvider = new NgcX2PdcpSpecificProvider<NgcX2> (this);
  m_x2RlcProvider = new NgcX2RlcSpecificProvider<NgcX2> (this);
  m_x2PdcpUser =0 ;//sjkang1114
}

NgcX2::~NgcX2 ()
{
  NS_LOG_FUNCTION (this);
}

void
NgcX2::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_x2InterfaceSockets.clear ();
  m_x2InterfaceCellIds.clear ();
  m_x2RlcUserMap.clear ();
  m_x2PdcpUserMap.clear ();
  m_x2RlcUserMap_2.clear (); //sjkang1016
  m_x2PdcpUserMap_2.clear ();//sjakng1016

  delete m_x2SapProvider;
  delete m_x2RlcProvider;
  delete m_x2PdcpProvider;
 // std::cout << "ngc_x2 disposed -->" << std::endl;
//  delete m_x2PdcpUser; ///sjkang
}

TypeId
NgcX2::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&NgcX2::m_rxPdu),
                     "ns3::NgcX2::ReceiveTracedCallback");
  return tid;
}
void
NgcX2::SetNgcX2PdcpUser(NgcX2PdcpUser *s){ //sjkang 1114
	m_x2PdcpUser =s ;
}
void
NgcX2::SetNgcX2SapUser (NgcX2SapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_x2SapUser = s;
}

NgcX2SapProvider*
NgcX2::GetNgcX2SapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_x2SapProvider;
}

// Get and Set interfaces with PDCP and RLC
NgcX2PdcpProvider*
NgcX2::GetNgcX2PdcpProvider ()
{
  NS_LOG_FUNCTION(this);
  return m_x2PdcpProvider;
}

NgcX2RlcProvider*
NgcX2::GetNgcX2RlcProvider ()
{
  return m_x2RlcProvider;
}

void
NgcX2::SetMcNgcX2RlcUser (uint32_t teid, NgcX2RlcUser* rlcUser)
{
  // TODO it may change (for the same teid) on handover between secondary cells, as in NrEnbRrc::RecvRlcSetupRequest
  //NS_ASSERT_MSG(m_x2RlcUserMap.find(teid) == m_x2RlcUserMap.end(), "Teid " << teid
  //  << " is already setup\n");
  NS_LOG_INFO("Add NgcX2RlcUser for teid " << teid);
  m_x2RlcUserMap[teid] = rlcUser;
}

void
NgcX2::SetMcNgcX2PdcpUser (uint32_t teid, NgcX2PdcpUser* pdcpUser)
{
  // TODO it may change (for the same teid) on handover between secondary cells, as in NrEnbRrc::RecvRlcSetupRequest
  //NS_ASSERT_MSG(m_x2PdcpUserMap.find(teid) == m_x2PdcpUserMap.end(), "Teid " << teid
  //  << " is already setup\n");
  NS_LOG_INFO("Add NgcX2PdcpUser for teid " << teid);
  m_x2PdcpUserMap[teid] = pdcpUser;
}
void
NgcX2::SetMcNgcX2RlcUser_2 (uint32_t teid, NgcX2RlcUser* rlcUser) //sjkang1016
{
  // TODO it may change (for the same teid) on handover between secondary cells, as in NrEnbRrc::RecvRlcSetupRequest
  //NS_ASSERT_MSG(m_x2RlcUserMap.find(teid) == m_x2RlcUserMap.end(), "Teid " << teid
  //  << " is already setup\n");
  NS_LOG_INFO("Add NgcX2RlcUser for teid " << teid);
  m_x2RlcUserMap_2[teid] = rlcUser;
}

void
NgcX2::SetMcNgcX2PdcpUser_2 (uint32_t teid, NgcX2PdcpUser* pdcpUser) //sjkang1016
{
  // TODO it may change (for the same teid) on handover between secondary cells, as in NrEnbRrc::RecvRlcSetupRequest
  //NS_ASSERT_MSG(m_x2PdcpUserMap.find(teid) == m_x2PdcpUserMap.end(), "Teid " << teid
  //  << " is already setup\n");
  NS_LOG_INFO("Add NgcX2PdcpUser for teid " << teid);
  m_x2PdcpUserMap_2[teid] = pdcpUser;
}
// Add X2 endpoint
void
NgcX2::AddX2Interface (uint16_t localCellId, Ipv4Address localX2Address, uint16_t remoteCellId, Ipv4Address remoteX2Address)
{
  NS_LOG_FUNCTION (this << localCellId << localX2Address << remoteCellId << remoteX2Address);

  int retval;

  // Get local eNB where this X2 entity belongs to
  Ptr<Node> localEnb = GetObject<Node> ();
//std::cout<< "-------------------------------------------------------------" <<std::endl;
  // Create X2-C socket for the local eNB
  Ptr<Socket> localX2cSocket = Socket::CreateSocket (localEnb, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = localX2cSocket->Bind (InetSocketAddress (localX2Address, m_x2cUdpPort));
  NS_ASSERT (retval == 0);
  localX2cSocket->SetRecvCallback (MakeCallback (&NgcX2::RecvFromX2cSocket, this));

  // Create X2-U socket for the local eNB
  Ptr<Socket> localX2uSocket = Socket::CreateSocket (localEnb, TypeId::LookupByName ("ns3::UdpSocketFactory"));
  retval = localX2uSocket->Bind (InetSocketAddress (localX2Address, m_x2uUdpPort));
  NS_ASSERT (retval == 0);
  localX2uSocket->SetRecvCallback (MakeCallback (&NgcX2::RecvFromX2uSocket, this));


  NS_ASSERT_MSG (m_x2InterfaceSockets.find (remoteCellId) == m_x2InterfaceSockets.end (),
                 "Mapping for remoteCellId = " << remoteCellId << " is already known");
  m_x2InterfaceSockets [remoteCellId] = Create<NrX2IfaceInfo> (remoteX2Address, localX2cSocket, localX2uSocket);
    NS_ASSERT_MSG (m_x2InterfaceCellIds.find (localX2cSocket) == m_x2InterfaceCellIds.end (),
                 "Mapping for control plane localSocket = " << localX2cSocket << " is already known");
  m_x2InterfaceCellIds [localX2cSocket] = Create<NrX2CellInfo> (localCellId, remoteCellId);

  NS_ASSERT_MSG (m_x2InterfaceCellIds.find (localX2uSocket) == m_x2InterfaceCellIds.end (),
                 "Mapping for data plane localSocket = " << localX2uSocket << " is already known");
  m_x2InterfaceCellIds [localX2uSocket] = Create<NrX2CellInfo> (localCellId, remoteCellId);
}

void
NgcX2::DoAddTeidToBeForwarded(uint32_t gtpTeid, uint16_t targetCellId)
{
  NS_LOG_FUNCTION(this << " add an entry to the map of teids to be forwarded: teid " << gtpTeid << " targetCellId " << targetCellId);
  NS_ASSERT_MSG(m_teidToBeForwardedMap.find(gtpTeid) == m_teidToBeForwardedMap.end(), "TEID already in the map");
  m_teidToBeForwardedMap.insert(std::pair<uint32_t, uint16_t> (gtpTeid, targetCellId));
}

void 
NgcX2::DoRemoveTeidToBeForwarded(uint32_t gtpTeid)
{
  NS_LOG_FUNCTION(this << " remove and entry from the map of teids to be forwarded: teid " << gtpTeid);
  NS_ASSERT_MSG(m_teidToBeForwardedMap.find(gtpTeid) != m_teidToBeForwardedMap.end(), "TEID not in the map");
  m_teidToBeForwardedMap.erase(m_teidToBeForwardedMap.find(gtpTeid));
}


void 
NgcX2::RecvFromX2cSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("Recv X2 message: from Socket");
  Ptr<Packet> packet = socket->Recv ();
  NS_LOG_LOGIC ("packetLen = " << packet->GetSize ());

  NS_ASSERT_MSG (m_x2InterfaceCellIds.find (socket) != m_x2InterfaceCellIds.end (),
                 "Missing infos of local and remote CellId");
  Ptr<NrX2CellInfo> cellsInfo = m_x2InterfaceCellIds [socket];

  NgcX2Tag ngcX2Tag;
  Time delay;
  if (packet->PeekPacketTag(ngcX2Tag))
    {
      delay = Simulator::Now() - ngcX2Tag.GetSenderTimestamp ();
      packet->RemovePacketTag(ngcX2Tag);

    }

  m_rxPdu(cellsInfo->m_remoteCellId, cellsInfo->m_localCellId, packet->GetSize (), delay.GetNanoSeconds (), 0);

  NgcX2Header x2Header;
  packet->RemoveHeader (x2Header);

  NS_LOG_LOGIC ("X2 header: " << x2Header);

  uint8_t messageType = x2Header.GetMessageType ();
  uint8_t procedureCode = x2Header.GetProcedureCode ();


  if (procedureCode == NgcX2Header::HandoverPreparation)
    {
      if (messageType == NgcX2Header::InitiatingMessage)
        {
          NS_LOG_LOGIC ("Recv X2 message: HANDOVER REQUEST");

          NgcX2HandoverRequestHeader x2HoReqHeader;
          packet->RemoveHeader (x2HoReqHeader);

          NS_LOG_INFO ("X2 HandoverRequest header: " << x2HoReqHeader);

          NgcX2SapUser::HandoverRequestParams params;
          params.oldEnbUeX2apId = x2HoReqHeader.GetOldEnbUeX2apId ();
          params.cause          = x2HoReqHeader.GetCause ();
          params.sourceCellId   = cellsInfo->m_remoteCellId;
          params.targetCellId   = x2HoReqHeader.GetTargetCellId ();
      //    params.nrCellId = x2HoReqHeader.GetNrCellId(); //sjkang0416
          params.amfUeN2apId    = x2HoReqHeader.GetAmfUeN2apId ();
          params.ueAggregateMaxBitRateDownlink = x2HoReqHeader.GetUeAggregateMaxBitRateDownlink ();
          params.ueAggregateMaxBitRateUplink   = x2HoReqHeader.GetUeAggregateMaxBitRateUplink ();
          params.bearers        = x2HoReqHeader.GetBearers ();
          // RlcRequests for secondary cell HO
          params.rlcRequests    = x2HoReqHeader.GetRlcSetupRequests();
          params.rrcContext     = packet;
          params.isMc           = x2HoReqHeader.GetIsMc ();
          params.isMC_2 			= x2HoReqHeader.GetIsMc_2(); //sjkang0206
          NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
          NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
          NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
          NS_LOG_LOGIC ("amfUeN2apId = " << params.amfUeN2apId);
          NS_LOG_LOGIC ("cellsInfo->m_localCellId = " << cellsInfo->m_localCellId);
          NS_ASSERT_MSG (params.targetCellId == cellsInfo->m_localCellId,
                         "TargetCellId mismatches with localCellId");

          m_x2SapUser->RecvHandoverRequest (params);
        }
      else if (messageType == NgcX2Header::SuccessfulOutcome)
        {
          NS_LOG_INFO ("Recv X2 message: HANDOVER REQUEST ACK");

          NgcX2HandoverRequestAckHeader x2HoReqAckHeader;
          packet->RemoveHeader (x2HoReqAckHeader);

          NS_LOG_INFO ("X2 HandoverRequestAck header: " << x2HoReqAckHeader);

          NgcX2SapUser::HandoverRequestAckParams params;          
          params.oldEnbUeX2apId = x2HoReqAckHeader.GetOldEnbUeX2apId ();
          params.newEnbUeX2apId = x2HoReqAckHeader.GetNewEnbUeX2apId ();
          params.sourceCellId   = cellsInfo->m_localCellId;
          params.targetCellId   = cellsInfo->m_remoteCellId;
          params.admittedBearers = x2HoReqAckHeader.GetAdmittedBearers ();
          params.notAdmittedBearers = x2HoReqAckHeader.GetNotAdmittedBearers ();
          params.rrcContext     = packet;
             NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
          NS_LOG_LOGIC ("newEnbUeX2apId = " << params.newEnbUeX2apId);
          NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
          NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);

          m_x2SapUser->RecvHandoverRequestAck (params);
        }
      else if (messageType == NgcX2Header::SuccesfulOutcomToNr)
             {
               NS_LOG_INFO ("Recv X2 message: HANDOVER REQUEST ACK To NR");

               NgcX2HandoverRequestAckHeader x2HoReqAckHeader;
               packet->RemoveHeader (x2HoReqAckHeader);

               NS_LOG_INFO ("X2 HandoverRequestAck header: " << x2HoReqAckHeader);
                NgcX2SapUser::HandoverRequestAckParams params;
               params.oldEnbUeX2apId = x2HoReqAckHeader.GetOldEnbUeX2apId ();
               params.newEnbUeX2apId = x2HoReqAckHeader.GetNewEnbUeX2apId ();
               params.sourceCellId   = cellsInfo->m_localCellId;
               params.targetCellId   = cellsInfo->m_remoteCellId;
               params.admittedBearers = x2HoReqAckHeader.GetAdmittedBearers ();
               params.notAdmittedBearers = x2HoReqAckHeader.GetNotAdmittedBearers ();
               params.rrcContext     = packet;
               NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
               NS_LOG_LOGIC ("newEnbUeX2apId = " << params.newEnbUeX2apId);
               NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
               NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);

               m_x2SapUser->RecvHandoverRequestAckToNr(params);
             }
      else // messageType == NgcX2Header::UnsuccessfulOutcome
        {
          NS_LOG_LOGIC ("Recv X2 message: HANDOVER PREPARATION FAILURE");

          NgcX2HandoverPreparationFailureHeader x2HoPrepFailHeader;
          packet->RemoveHeader (x2HoPrepFailHeader);

          NS_LOG_INFO ("X2 HandoverPreparationFailure header: " << x2HoPrepFailHeader);

          NgcX2SapUser::HandoverPreparationFailureParams params;          
          params.oldEnbUeX2apId = x2HoPrepFailHeader.GetOldEnbUeX2apId ();
          params.sourceCellId   = cellsInfo->m_localCellId;
          params.targetCellId   = cellsInfo->m_remoteCellId;
          params.cause          = x2HoPrepFailHeader.GetCause ();
          params.criticalityDiagnostics = x2HoPrepFailHeader.GetCriticalityDiagnostics ();

          NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
          NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
          NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
          NS_LOG_LOGIC ("cause = " << params.cause);
          NS_LOG_LOGIC ("criticalityDiagnostics = " << params.criticalityDiagnostics);

          m_x2SapUser->RecvHandoverPreparationFailure (params);
        }
    }
  else if (procedureCode == NgcX2Header::LoadIndication)
    {
      if (messageType == NgcX2Header::InitiatingMessage)
        {
          NS_LOG_LOGIC ("Recv X2 message: LOAD INFORMATION");

          NgcX2LoadInformationHeader x2LoadInfoHeader;
          packet->RemoveHeader (x2LoadInfoHeader);

          NS_LOG_INFO ("X2 LoadInformation header: " << x2LoadInfoHeader);

          NgcX2SapUser::LoadInformationParams params;
          params.cellInformationList = x2LoadInfoHeader.GetCellInformationList ();

          NS_LOG_LOGIC ("cellInformationList size = " << params.cellInformationList.size ());

          m_x2SapUser->RecvLoadInformation (params);
        }
    }
  else if (procedureCode == NgcX2Header::SnStatusTransfer)
    {
      if (messageType == NgcX2Header::InitiatingMessage)
        {
          NS_LOG_LOGIC ("Recv X2 message: SN STATUS TRANSFER");

            NgcX2SnStatusTransferHeader x2SnStatusXferHeader;
            packet->RemoveHeader (x2SnStatusXferHeader);

            NS_LOG_INFO ("X2 SnStatusTransfer header: " << x2SnStatusXferHeader);

            NgcX2SapUser::SnStatusTransferParams params;
            params.oldEnbUeX2apId = x2SnStatusXferHeader.GetOldEnbUeX2apId ();
            params.newEnbUeX2apId = x2SnStatusXferHeader.GetNewEnbUeX2apId ();
            params.sourceCellId   = cellsInfo->m_remoteCellId;
            params.targetCellId   = cellsInfo->m_localCellId;
            params.erabsSubjectToStatusTransferList = x2SnStatusXferHeader.GetErabsSubjectToStatusTransferList ();

            NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
            NS_LOG_LOGIC ("newEnbUeX2apId = " << params.newEnbUeX2apId);
            NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
            NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
            NS_LOG_LOGIC ("erabsList size = " << params.erabsSubjectToStatusTransferList.size ());

            m_x2SapUser->RecvSnStatusTransfer (params);
        }
    }
  else if (procedureCode == NgcX2Header::UeContextRelease)
    {
      if (messageType == NgcX2Header::InitiatingMessage)
        {
          NS_LOG_LOGIC ("Recv X2 message: UE CONTEXT RELEASE");

          NgcX2UeContextReleaseHeader x2UeCtxReleaseHeader;
          packet->RemoveHeader (x2UeCtxReleaseHeader);

          NS_LOG_INFO ("X2 UeContextRelease header: " << x2UeCtxReleaseHeader);

          NgcX2SapUser::UeContextReleaseParams params;
          params.oldEnbUeX2apId = x2UeCtxReleaseHeader.GetOldEnbUeX2apId ();
          params.newEnbUeX2apId = x2UeCtxReleaseHeader.GetNewEnbUeX2apId ();
          params.sourceCellId = cellsInfo->m_remoteCellId;

          NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
          NS_LOG_LOGIC ("newEnbUeX2apId = " << params.newEnbUeX2apId);

          m_x2SapUser->RecvUeContextRelease (params);
        }
    }
  else if (procedureCode == NgcX2Header::ResourceStatusReporting)
    {
      if (messageType == NgcX2Header::InitiatingMessage)
        {
          NS_LOG_LOGIC ("Recv X2 message: RESOURCE STATUS UPDATE");

          NgcX2ResourceStatusUpdateHeader x2ResStatUpdHeader;
          packet->RemoveHeader (x2ResStatUpdHeader);

          NS_LOG_INFO ("X2 ResourceStatusUpdate header: " << x2ResStatUpdHeader);

          NgcX2SapUser::ResourceStatusUpdateParams params;
          params.targetCellId = 0;
          params.enb1MeasurementId = x2ResStatUpdHeader.GetEnb1MeasurementId ();
          params.enb2MeasurementId = x2ResStatUpdHeader.GetEnb2MeasurementId ();
          params.cellMeasurementResultList = x2ResStatUpdHeader.GetCellMeasurementResultList ();

          NS_LOG_LOGIC ("enb1MeasurementId = " << params.enb1MeasurementId);
          NS_LOG_LOGIC ("enb2MeasurementId = " << params.enb2MeasurementId);
          NS_LOG_LOGIC ("cellMeasurementResultList size = " << params.cellMeasurementResultList.size ());

          m_x2SapUser->RecvResourceStatusUpdate (params);
        }
    }
  else if (procedureCode == NgcX2Header::RlcSetupRequest)
    {
      NS_LOG_LOGIC ("Recv X2 message: RLC SETUP REQUEST");

      NgcX2RlcSetupRequestHeader x2RlcHeader;
      packet->RemoveHeader (x2RlcHeader);

      NS_LOG_INFO ("X2 RlcSetupRequest header: " << x2RlcHeader);

      NgcX2SapUser::RlcSetupRequest params;
      params.targetCellId = x2RlcHeader.GetTargetCellId();
      params.sourceCellId = x2RlcHeader.GetSourceCellId ();
      params.mmWaveRnti = x2RlcHeader.GetMmWaveRnti ();
      params.gtpTeid = x2RlcHeader.GetGtpTeid ();
      params.nrRnti = x2RlcHeader.GetNrRnti ();
      params.drbid = x2RlcHeader.GetDrbid ();
      params.lcinfo = x2RlcHeader.GetLcInfo();
      params.rlcConfig = x2RlcHeader.GetRlcConfig();
      params.logicalChannelConfig = x2RlcHeader.GetLogicalChannelConfig();

      NS_LOG_LOGIC ("GtpTeid = " << params.gtpTeid);
      NS_LOG_LOGIC ("MmWaveRnti = " << params.mmWaveRnti);
      NS_LOG_LOGIC ("SourceCellID = " << params.sourceCellId);
      NS_LOG_LOGIC ("TargetCellID = " << params.targetCellId);
      NS_LOG_LOGIC ("Drbid = " << params.drbid);

      m_x2SapUser->RecvRlcSetupRequest (params);
    }
  else if (procedureCode == NgcX2Header::RlcSetupCompleted)
    {
      NS_LOG_LOGIC ("Recv X2 message: RLC SETUP COMPLETED");

      NgcX2RlcSetupCompletedHeader x2RlcHeader;
      packet->RemoveHeader (x2RlcHeader);

      NS_LOG_INFO ("X2 RlcSetupCompleted header: " << x2RlcHeader);

      NgcX2SapUser::UeDataParams params;
      params.targetCellId = x2RlcHeader.GetTargetCellId();
      params.sourceCellId = x2RlcHeader.GetSourceCellId ();
      params.gtpTeid = x2RlcHeader.GetGtpTeid ();

      NS_LOG_LOGIC ("GtpTeid = " << params.gtpTeid);
      NS_LOG_LOGIC ("SourceCellID = " << params.sourceCellId);
      NS_LOG_LOGIC ("TargetCellID = " << params.targetCellId);

      m_x2SapUser->RecvRlcSetupCompleted (params);
    }
  else if(procedureCode == NgcX2Header::UpdateUeSinr)
    {
      NS_LOG_LOGIC ("Recv X2 message: UPDATE UE SINR");
      
      NgcX2UeImsiSinrUpdateHeader x2ueSinrUpdateHeader;
      packet->RemoveHeader(x2ueSinrUpdateHeader);

      NS_LOG_INFO ("X2 SinrUpdateHeader header: " << x2ueSinrUpdateHeader);
      NgcX2SapUser::UeImsiSinrParams params;
      params.ueImsiSinrMap = x2ueSinrUpdateHeader.GetUeImsiSinrMap();
      params.sourceCellId = x2ueSinrUpdateHeader.GetSourceCellId();
      params.secondBestCellId=x2ueSinrUpdateHeader.GetSecondCellId(); //sjkang1015
      params.m_rnti = x2ueSinrUpdateHeader.GetRnti(); //sjkang
      m_x2SapUser->RecvUeSinrUpdate(params);  
      //std::cout<< params.sourceCellId << std::endl;

    }
  else if (procedureCode == NgcX2Header::RequestMcHandover)
    {
      NS_LOG_LOGIC ("Recv X2 message: REQUEST MC HANDOVER");
      
      NgcX2McHandoverHeader x2mcHeader;
      packet->RemoveHeader(x2mcHeader);

      NS_LOG_INFO ("X2 RequestMcHandover header: " << x2mcHeader);

      NgcX2SapUser::SecondaryHandoverParams params;
      params.targetCellId = x2mcHeader.GetTargetCellId();
      params.imsi = x2mcHeader.GetImsi();
      params.oldCellId = x2mcHeader.GetOldCellId();
      params.secondMmWaveCellID = x2mcHeader.GetSecondMmWaveCellId(); //sjkang
      m_x2SapUser->RecvMcHandoverRequest(params);  
    }
  else if (procedureCode == NgcX2Header::NotifyMmWaveNrHandover)
    {
      NS_LOG_LOGIC ("Recv X2 message: NOTIFY MMWAVE HANDOVER TO NR");
      
      NgcX2McHandoverHeader x2mcHeader;
      packet->RemoveHeader(x2mcHeader);

      NS_LOG_INFO ("X2 McHandover header: " << x2mcHeader);

      NgcX2SapUser::SecondaryHandoverParams params;
      params.targetCellId = x2mcHeader.GetTargetCellId(); // the new MmWave cell to which the UE is connected
      params.imsi = x2mcHeader.GetImsi(); 
      params.oldCellId = x2mcHeader.GetOldCellId(); // actually, the NR cell ID

      m_x2SapUser->RecvNrMmWaveHandoverCompleted(params);  
    }
  else if (procedureCode == NgcX2Header::SwitchConnection)
    {
      NS_LOG_LOGIC ("Recv X2 message: SWITCH CONNECTION");
      
      NgcX2ConnectionSwitchHeader x2mcHeader;
      packet->RemoveHeader(x2mcHeader);

      NS_LOG_INFO ("X2 SwitchConnection header: " << x2mcHeader);

      NgcX2SapUser::SwitchConnectionParams params;
      params.mmWaveRnti = x2mcHeader.GetMmWaveRnti();
      params.useMmWaveConnection = x2mcHeader.GetUseMmWaveConnection();
      params.drbid = x2mcHeader.GetDrbid();

      m_x2SapUser->RecvConnectionSwitchToMmWave(params);
    }
  else if (procedureCode == NgcX2Header::SecondaryCellHandoverCompleted)
    {
      NS_LOG_INFO ("Recv X2 message: SECONDARY CELL HANDOVER COMPLETED");

      NgcX2SecondaryCellHandoverCompletedHeader x2hoHeader;
      packet->RemoveHeader(x2hoHeader);

      NgcX2SapUser::SecondaryHandoverCompletedParams params;
      params.mmWaveRnti = x2hoHeader.GetMmWaveRnti();
      params.imsi = x2hoHeader.GetImsi();
      params.oldEnbUeX2apId = x2hoHeader.GetOldEnbUeX2apId();
      params.isMc = x2hoHeader.GetisMc();
      params.isMc_2 = x2hoHeader.GetisMc_2(); //sjkang0710
      params.cellId = cellsInfo->m_remoteCellId;

      m_x2SapUser->RecvSecondaryCellHandoverCompleted(params);
    }
  else if(procedureCode == NgcX2Header::SendingAssistantInformation){ //sjkang1114

    	if (messageType == NgcX2Header::McAssistantInfoForwarding){ //sjkang1114
    		  NS_LOG_INFO ("Recv X2 message: from Socket");
    		NgcX2AssistantInfoHeader ngcX2AssistantHeader;
    		packet->RemoveHeader(ngcX2AssistantHeader);
    		NgcX2SapUser::AssistantInformationForSplitting params;
    		params.Tx_On_Q_Size = ngcX2AssistantHeader.GetTxonQueue();
    		params.Txed_Q_Delay =ngcX2AssistantHeader.GetTxedQueue();
    		params.Re_TX_Q_Size =ngcX2AssistantHeader.GetRetxQueue();
    		params.Tx_On_Q_Delay = ngcX2AssistantHeader.GetTxonQueingDelay();
    		params.Re_Tx_Q_Delay = ngcX2AssistantHeader.GetReTxQueuingDelay();
    		params.sourceCellId = ngcX2AssistantHeader.GetSourceCellId();
    		params.rnti = ngcX2AssistantHeader.GetRnti();
    		params.drbId = ngcX2AssistantHeader.GetDrbId();
    	//	std::cout << "forwarding assistant infor " <<std::endl;
    		//std::cout << m_x2SapProvider << std::endl;
    	   m_x2SapUser ->RecvAssistantInformation(params); //sjkang1115
    	  // m_x2PdcpUser->ReceiveAssistantInformation(params);
    	 	}
    }
  else if (procedureCode == NgcX2Header::BufferDuplication){ //sjkang0802
	  if (messageType == NgcX2Header::DuplicationRlcBuffer){
		//  std::cout << " eNB receives buffer duplication message "<< std::endl;
		  NgcX2DuplicationRlcBufferHeader duplHeader;
		  packet->RemoveHeader(duplHeader);
		  NgcX2SapProvider::SendBufferDuplicationMessage params;
		  params.imsi = duplHeader.GetImsi();
		  params.targetCellId = duplHeader.GetTargetCellID();
		  params.option = duplHeader.GetOption();
		  params.cellIDforBufferForwarding = duplHeader.GetCellIDForBufferForwarding();
		  m_x2SapUser->RecvBufferDuplicationMessage(params); //sjkang
	  }
  }
  else
    {
      NS_ASSERT_MSG (false, "ProcedureCode NOT SUPPORTED!!!");
    }
}


void 
NgcX2::RecvFromX2uSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  NS_LOG_LOGIC ("Recv UE DATA through X2-U interface from Socket");
  Ptr<Packet> packet = socket->Recv ();
  NS_LOG_LOGIC ("packetLen = " << packet->GetSize ());

  NS_ASSERT_MSG (m_x2InterfaceCellIds.find (socket) != m_x2InterfaceCellIds.end (),
                 "Missing infos of local and remote CellId");
  Ptr<NrX2CellInfo> cellsInfo = m_x2InterfaceCellIds [socket];

  NS_LOG_INFO("localCellId = " << cellsInfo->m_localCellId);
  NS_LOG_INFO("remoteCellId = " << cellsInfo->m_remoteCellId);

  NgcX2Tag ngcX2Tag;
  Time delay;
  if (packet->PeekPacketTag(ngcX2Tag))
    {
      delay = Simulator::Now() - ngcX2Tag.GetSenderTimestamp ();
      packet->RemovePacketTag(ngcX2Tag);
    }
  m_rxPdu(cellsInfo->m_localCellId, cellsInfo->m_remoteCellId, packet->GetSize (), delay.GetNanoSeconds (), 1);

  NrGtpuHeader gtpu;
  packet->RemoveHeader (gtpu);
  //SocketAddressTag satag;
  //packet->RemovePacketTag(satag);

  NS_LOG_LOGIC ("GTP-U header: " << gtpu);

  NgcX2SapUser::UeDataParams params;
  params.sourceCellId = cellsInfo->m_remoteCellId;
  params.targetCellId = cellsInfo->m_localCellId;
  params.gtpTeid = gtpu.GetTeid ();
  params.ueData = packet;

  NS_LOG_LOGIC("Received packet on X2 u, size " << packet->GetSize() 
    << " source " << params.sourceCellId << " target " << params.targetCellId << " type " << gtpu.GetMessageType());

  if(m_teidToBeForwardedMap.find(params.gtpTeid) == m_teidToBeForwardedMap.end())
  {
    if(gtpu.GetMessageType() == NgcX2Header::McForwardDownlinkData)
    {
      // add PdcpTag
      PdcpTag pdcpTag (Simulator::Now ());
      params.ueData->AddByteTag (pdcpTag);
      // call rlc interface
      NgcX2RlcUser* user = m_x2RlcUserMap.find(params.gtpTeid)->second;
      NgcX2RlcUser* user_2 = m_x2RlcUserMap_2.find(params.gtpTeid)->second; //sjkang1016

      if(user != 0 && !isAdditionalMmWave) //sjkang1016
      {
        user -> SendMcPdcpSdu(params);
      }else if (user_2 !=0 && isAdditionalMmWave) //sjkang1016
      { //sjkang1016
     	  user_2 ->SendMcPdcpSdu(params);
      }
      else
      {
        NS_LOG_INFO("Not implemented: Forward to the other cell or to NR");
      }
    } 
    else if (gtpu.GetMessageType() == NgcX2Header::McForwardUplinkData)
    {
      // call pdcp interface
      NS_LOG_INFO("Call PDCP interface");

 //     if(!isAdditionalMmWave) //sjkang1016
   //   {
    	//  std::cout << "call to user_2---------------for uplink " <<std::endl;
      m_x2PdcpUserMap[params.gtpTeid] -> ReceiveMcPdcpPdu(params);
    //  }
     // else{
    	//  std::cout << "call to user_2---------------for uplink2 " <<std::endl;
    	//  m_x2PdcpUserMap_2[params.gtpTeid] -> ReceiveMcPdcpPdu(params);
      //}
    }
    else
    {
      m_x2SapUser->RecvUeData (params);
    }
  }
  else // the packet was received during a secondary cell HO, forward to the target cell
  {
    params.sourceCellId = cellsInfo->m_remoteCellId;
    params.targetCellId = m_teidToBeForwardedMap.find(params.gtpTeid)->second;
    NS_LOG_LOGIC("Forward from " << cellsInfo->m_localCellId << " to " << params.targetCellId);
    DoSendMcPdcpPdu(params);
  }
}

//
// Implementation of the X2 SAP Provider
//
void
NgcX2::DoSendHandoverRequest (NgcX2SapProvider::HandoverRequestParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("amfUeN2apId  = " << params.amfUeN2apId);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for targetCellId = " << params.targetCellId <<"\t "<< this);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send X2 message: HANDOVER REQUEST");

  // Build the X2 message
  NgcX2HandoverRequestHeader x2HoReqHeader;
  x2HoReqHeader.SetOldEnbUeX2apId (params.oldEnbUeX2apId);
  x2HoReqHeader.SetCause (params.cause);
  x2HoReqHeader.SetTargetCellId (params.targetCellId);
 //x2HoReqHeader.SetNrCellId(params.nrCellId); //sjkang0416
  x2HoReqHeader.SetAmfUeN2apId (params.amfUeN2apId);
  x2HoReqHeader.SetUeAggregateMaxBitRateDownlink (params.ueAggregateMaxBitRateDownlink);
  x2HoReqHeader.SetUeAggregateMaxBitRateUplink (params.ueAggregateMaxBitRateUplink);
  x2HoReqHeader.SetBearers (params.bearers);
  // For secondary cell handover
  x2HoReqHeader.SetRlcSetupRequests (params.rlcRequests);
  x2HoReqHeader.SetIsMc (params.isMc);
  x2HoReqHeader.SetIsMc_2(params.isMC_2); //sjkang0206
  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::HandoverPreparation);
  x2Header.SetLengthOfIes (x2HoReqHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2HoReqHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 HandoverRequest header: " << x2HoReqHeader);

  // Build the X2 packet
  Ptr<Packet> packet = (params.rrcContext != 0) ? (params.rrcContext) : (Create <Packet> ());
  packet->AddHeader (x2HoReqHeader);
  packet->AddHeader (x2Header);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));
}

void
NgcX2::DoSendRlcSetupRequest (NgcX2SapProvider::RlcSetupRequest params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("teid  = " << params.gtpTeid);
  NS_LOG_LOGIC ("rnti = " << params.mmWaveRnti);
//std::cout<<params.targetCellId << std::endl;
  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for targetCellId = " << params.targetCellId);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send X2 message: RLC SETUP REQUEST");

  // Build the X2 message
  NgcX2RlcSetupRequestHeader x2RlcHeader;
  x2RlcHeader.SetSourceCellId(params.sourceCellId);
  x2RlcHeader.SetTargetCellId(params.targetCellId);
  x2RlcHeader.SetGtpTeid(params.gtpTeid);
  x2RlcHeader.SetMmWaveRnti(params.mmWaveRnti);
  x2RlcHeader.SetNrRnti(params.nrRnti);
  x2RlcHeader.SetDrbid(params.drbid);
  x2RlcHeader.SetLcInfo(params.lcinfo);
  x2RlcHeader.SetRlcConfig(params.rlcConfig);
  x2RlcHeader.SetLogicalChannelConfig(params.logicalChannelConfig);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::RlcSetupRequest);
  x2Header.SetLengthOfIes (x2RlcHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2RlcHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 RlcSetupRequest header: " << x2RlcHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2RlcHeader);
  packet->AddHeader (x2Header);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));
}


void
NgcX2::DoSendRlcSetupCompleted (NgcX2Sap::UeDataParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("teid  = " << params.gtpTeid);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for targetCellId = " << params.targetCellId);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send X2 message: RLC SETUP COMPLETED");

  // Build the X2 message
  NgcX2RlcSetupCompletedHeader x2RlcHeader;
  x2RlcHeader.SetSourceCellId(params.sourceCellId);
  x2RlcHeader.SetTargetCellId(params.targetCellId);
  x2RlcHeader.SetGtpTeid(params.gtpTeid);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::RlcSetupCompleted);
  x2Header.SetLengthOfIes (x2RlcHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2RlcHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 RlcSetupCompleted header: " << x2RlcHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2RlcHeader);
  packet->AddHeader (x2Header);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));
}

void
NgcX2::DoSendHandoverRequestAck (NgcX2SapProvider::HandoverRequestAckParams params)
{
  NS_LOG_FUNCTION (this);
 /// std::cout << "-------------- X2 will send handover request ack to Target eNB -----------" <<std::endl;

  NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
  NS_LOG_LOGIC ("newEnbUeX2apId = " << params.newEnbUeX2apId);
  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.sourceCellId) != m_x2InterfaceSockets.end (),
                 "Socket infos not defined for sourceCellId = " << params.sourceCellId);

  Ptr<Socket> localSocket = m_x2InterfaceSockets [params.sourceCellId]->m_localCtrlPlaneSocket;
  Ipv4Address remoteIpAddr = m_x2InterfaceSockets [params.sourceCellId]->m_remoteIpAddr;

  NS_LOG_LOGIC ("localSocket = " << localSocket);
  NS_LOG_LOGIC ("remoteIpAddr = " << remoteIpAddr);

  NS_LOG_INFO ("Send X2 message: HANDOVER REQUEST ACK");

  // Build the X2 message
  NgcX2HandoverRequestAckHeader x2HoAckHeader;
  x2HoAckHeader.SetOldEnbUeX2apId (params.oldEnbUeX2apId);
  x2HoAckHeader.SetNewEnbUeX2apId (params.newEnbUeX2apId);
  x2HoAckHeader.SetAdmittedBearers (params.admittedBearers);
  x2HoAckHeader.SetNotAdmittedBearers (params.notAdmittedBearers);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::SuccessfulOutcome);
  x2Header.SetProcedureCode (NgcX2Header::HandoverPreparation);
  x2Header.SetLengthOfIes (x2HoAckHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2HoAckHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 HandoverAck header: " << x2HoAckHeader);
  NS_LOG_INFO ("RRC context: " << params.rrcContext);

  // Build the X2 packet
  Ptr<Packet> packet = (params.rrcContext != 0) ? (params.rrcContext) : (Create <Packet> ());
  packet->AddHeader (x2HoAckHeader);
  packet->AddHeader (x2Header);
   std::cout << packet << std::endl;
  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  localSocket->SendTo (packet, 0, InetSocketAddress (remoteIpAddr, m_x2cUdpPort));
}
void
NgcX2::DoSendHandoverRequestAckToNr (NgcX2SapProvider::HandoverRequestAckParams params)
{
  NS_LOG_FUNCTION (this);
 //std::cout << "-------------- X2 will send handover request ack to Nr eNB -----------" <<std::endl;
  NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
  NS_LOG_LOGIC ("newEnbUeX2apId = " << params.newEnbUeX2apId);
  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.sourceCellId) != m_x2InterfaceSockets.end (),
                 "Socket infos not defined for sourceCellId = " << params.sourceCellId);

  Ptr<Socket> localSocket = m_x2InterfaceSockets [params.sourceCellId]->m_localCtrlPlaneSocket;
  Ipv4Address remoteIpAddr = m_x2InterfaceSockets [params.sourceCellId]->m_remoteIpAddr;

  NS_LOG_LOGIC ("localSocket = " << localSocket);
  NS_LOG_LOGIC ("remoteIpAddr = " << remoteIpAddr);

  NS_LOG_INFO ("Send X2 message: HANDOVER REQUEST ACK");

  // Build the X2 message
  NgcX2HandoverRequestAckHeader x2HoAckHeader;
  x2HoAckHeader.SetOldEnbUeX2apId (params.oldEnbUeX2apId);
  x2HoAckHeader.SetNewEnbUeX2apId (params.newEnbUeX2apId);
  x2HoAckHeader.SetAdmittedBearers (params.admittedBearers);
  x2HoAckHeader.SetNotAdmittedBearers (params.notAdmittedBearers);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::SuccesfulOutcomToNr);
  x2Header.SetProcedureCode (NgcX2Header::HandoverPreparation);
  x2Header.SetLengthOfIes (x2HoAckHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2HoAckHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 HandoverAck header: " << x2HoAckHeader);
  NS_LOG_INFO ("RRC context: " << params.rrcContext);

  // Build the X2 packet
  Ptr<Packet> packet = (params.rrcContext != 0) ? (params.rrcContext) : (Create <Packet> ());
  packet->AddHeader (x2HoAckHeader);
  packet->AddHeader (x2Header);

  //NgcX2Tag tag (Simulator::Now());
 // packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  localSocket->SendTo (packet, 0, InetSocketAddress (remoteIpAddr, m_x2cUdpPort));
}

void
NgcX2::DoSendHandoverPreparationFailure (NgcX2SapProvider::HandoverPreparationFailureParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("cause = " << params.cause);
  NS_LOG_LOGIC ("criticalityDiagnostics = " << params.criticalityDiagnostics);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.sourceCellId) != m_x2InterfaceSockets.end (),
                 "Socket infos not defined for sourceCellId = " << params.sourceCellId);

  Ptr<Socket> localSocket = m_x2InterfaceSockets [params.sourceCellId]->m_localCtrlPlaneSocket;
  Ipv4Address remoteIpAddr = m_x2InterfaceSockets [params.sourceCellId]->m_remoteIpAddr;

  NS_LOG_LOGIC ("localSocket = " << localSocket);
  NS_LOG_LOGIC ("remoteIpAddr = " << remoteIpAddr);

  NS_LOG_INFO ("Send X2 message: HANDOVER PREPARATION FAILURE");

  // Build the X2 message
  NgcX2HandoverPreparationFailureHeader x2HoPrepFailHeader;
  x2HoPrepFailHeader.SetOldEnbUeX2apId (params.oldEnbUeX2apId);
  x2HoPrepFailHeader.SetCause (params.cause);
  x2HoPrepFailHeader.SetCriticalityDiagnostics (params.criticalityDiagnostics);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::UnsuccessfulOutcome);
  x2Header.SetProcedureCode (NgcX2Header::HandoverPreparation);
  x2Header.SetLengthOfIes (x2HoPrepFailHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2HoPrepFailHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 HandoverPrepFail header: " << x2HoPrepFailHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2HoPrepFailHeader);
  packet->AddHeader (x2Header);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  localSocket->SendTo (packet, 0, InetSocketAddress (remoteIpAddr, m_x2cUdpPort));
}

void
NgcX2::DoNotifyCoordinatorHandoverFailed(NgcX2SapProvider::HandoverFailedParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("coordinator cellId " << params.coordinatorId);
  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("imsi = " << params.imsi);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.coordinatorId) != m_x2InterfaceSockets.end (),
                 "Socket infos not defined for coordinatorId = " << params.coordinatorId);

  Ptr<Socket> localSocket = m_x2InterfaceSockets [params.coordinatorId]->m_localCtrlPlaneSocket;
  Ipv4Address remoteIpAddr = m_x2InterfaceSockets [params.coordinatorId]->m_remoteIpAddr;

  NS_LOG_LOGIC ("localSocket = " << localSocket);
  NS_LOG_LOGIC ("remoteIpAddr = " << remoteIpAddr);

  NS_LOG_INFO ("Send X2 message: NOTIFY HANDOVER FAILED");

  // Build the X2 message
  NgcX2NotifyCoordinatorHandoverFailedHeader x2failHeader;
  x2failHeader.SetSourceCellId (params.sourceCellId);
  x2failHeader.SetTargetCellId (params.targetCellId);
  x2failHeader.SetImsi (params.imsi);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::UnsuccessfulOutcome);
  x2Header.SetProcedureCode (NgcX2Header::NotifyCoordinatorHandoverFailed);
  x2Header.SetLengthOfIes (x2failHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2failHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 HandoverPrepFail header: " << x2failHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2failHeader);
  packet->AddHeader (x2Header);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  localSocket->SendTo (packet, 0, InetSocketAddress (remoteIpAddr, m_x2cUdpPort));
}


void
NgcX2::DoSendSnStatusTransfer (NgcX2SapProvider::SnStatusTransferParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
  NS_LOG_LOGIC ("newEnbUeX2apId = " << params.newEnbUeX2apId);
  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("erabsList size = " << params.erabsSubjectToStatusTransferList.size ());

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                 "Socket infos not defined for targetCellId = " << params.targetCellId);

  Ptr<Socket> localSocket = m_x2InterfaceSockets [params.targetCellId]->m_localCtrlPlaneSocket;
  Ipv4Address remoteIpAddr = m_x2InterfaceSockets [params.targetCellId]->m_remoteIpAddr;

  NS_LOG_LOGIC ("localSocket = " << localSocket);
  NS_LOG_LOGIC ("remoteIpAddr = " << remoteIpAddr);

  NS_LOG_INFO ("Send X2 message: SN STATUS TRANSFER");

  // Build the X2 message
  NgcX2SnStatusTransferHeader x2SnStatusXferHeader;
  x2SnStatusXferHeader.SetOldEnbUeX2apId (params.oldEnbUeX2apId);
  x2SnStatusXferHeader.SetNewEnbUeX2apId (params.newEnbUeX2apId);
  x2SnStatusXferHeader.SetErabsSubjectToStatusTransferList (params.erabsSubjectToStatusTransferList);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::SnStatusTransfer);
  x2Header.SetLengthOfIes (x2SnStatusXferHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2SnStatusXferHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 SnStatusTransfer header: " << x2SnStatusXferHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2SnStatusXferHeader);
  packet->AddHeader (x2Header);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  localSocket->SendTo (packet, 0, InetSocketAddress (remoteIpAddr, m_x2cUdpPort));
}


void
NgcX2::DoSendUeContextRelease (NgcX2SapProvider::UeContextReleaseParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
  NS_LOG_LOGIC ("newEnbUeX2apId = " << params.newEnbUeX2apId);
  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.sourceCellId) != m_x2InterfaceSockets.end (),
                 "Socket infos not defined for sourceCellId = " << params.sourceCellId);

  Ptr<Socket> localSocket = m_x2InterfaceSockets [params.sourceCellId]->m_localCtrlPlaneSocket;
  Ipv4Address remoteIpAddr = m_x2InterfaceSockets [params.sourceCellId]->m_remoteIpAddr;

  NS_LOG_LOGIC ("localSocket = " << localSocket);
  NS_LOG_LOGIC ("remoteIpAddr = " << remoteIpAddr);

  NS_LOG_INFO ("Send X2 message: UE CONTEXT RELEASE");

  // Build the X2 message
  NgcX2UeContextReleaseHeader x2UeCtxReleaseHeader;
  x2UeCtxReleaseHeader.SetOldEnbUeX2apId (params.oldEnbUeX2apId);
  x2UeCtxReleaseHeader.SetNewEnbUeX2apId (params.newEnbUeX2apId);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::UeContextRelease);
  x2Header.SetLengthOfIes (x2UeCtxReleaseHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2UeCtxReleaseHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 UeContextRelease header: " << x2UeCtxReleaseHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2UeCtxReleaseHeader);
  packet->AddHeader (x2Header);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  localSocket->SendTo (packet, 0, InetSocketAddress (remoteIpAddr, m_x2cUdpPort));
}


void
NgcX2::DoSendLoadInformation (NgcX2SapProvider::LoadInformationParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("cellInformationList size = " << params.cellInformationList.size ());

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for targetCellId = " << params.targetCellId);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send X2 message: LOAD INFORMATION");

  // Build the X2 message
  NgcX2LoadInformationHeader x2LoadInfoHeader;
  x2LoadInfoHeader.SetCellInformationList (params.cellInformationList);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::LoadIndication);
  x2Header.SetLengthOfIes (x2LoadInfoHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2LoadInfoHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 LoadInformation header: " << x2LoadInfoHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2LoadInfoHeader);
  packet->AddHeader (x2Header);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));

}


void
NgcX2::DoSendResourceStatusUpdate (NgcX2SapProvider::ResourceStatusUpdateParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("enb1MeasurementId = " << params.enb1MeasurementId);
  NS_LOG_LOGIC ("enb2MeasurementId = " << params.enb2MeasurementId);
  NS_LOG_LOGIC ("cellMeasurementResultList size = " << params.cellMeasurementResultList.size ());

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for targetCellId = " << params.targetCellId);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send X2 message: RESOURCE STATUS UPDATE");

  // Build the X2 message
  NgcX2ResourceStatusUpdateHeader x2ResourceStatUpdHeader;
  x2ResourceStatUpdHeader.SetEnb1MeasurementId (params.enb1MeasurementId);
  x2ResourceStatUpdHeader.SetEnb2MeasurementId (params.enb2MeasurementId);
  x2ResourceStatUpdHeader.SetCellMeasurementResultList (params.cellMeasurementResultList);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::ResourceStatusReporting);
  x2Header.SetLengthOfIes (x2ResourceStatUpdHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2ResourceStatUpdHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 ResourceStatusUpdate header: " << x2ResourceStatUpdHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2ResourceStatUpdHeader);
  packet->AddHeader (x2Header);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  // Send the X2 message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));

}


void
NgcX2::DoSendUeData (NgcX2SapProvider::UeDataParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("gtpTeid = " << params.gtpTeid);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for targetCellId = " << params.targetCellId << "\t"<< this);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localUserPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NrGtpuHeader gtpu;
  gtpu.SetTeid (params.gtpTeid);
  gtpu.SetLength (params.ueData->GetSize () + gtpu.GetSerializedSize () - 8); /// \todo This should be done in NrGtpuHeader
  NS_LOG_INFO ("GTP-U header: " << gtpu);

  Ptr<Packet> packet = params.ueData;
  packet->AddHeader (gtpu);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("Forward UE DATA through X2 interface");
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2uUdpPort));
}

void
NgcX2::DoSendMcPdcpPdu(NgcX2Sap::UeDataParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("gtpTeid = " << params.gtpTeid);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for targetCellId = " << params.targetCellId);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localUserPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  // add a message type to the gtpu header, so that it is possible to distinguish at receiver
  NrGtpuHeader gtpu;
  gtpu.SetTeid (params.gtpTeid);
  gtpu.SetMessageType(NgcX2Header::McForwardDownlinkData);
  gtpu.SetLength (params.ueData->GetSize () + gtpu.GetSerializedSize () - 8); /// \todo This should be done in NrGtpuHeader
  NS_LOG_INFO ("GTP-U header: " << gtpu);

  Ptr<Packet> packet = params.ueData;
  packet->AddHeader (gtpu);

  NgcX2Tag tag (Simulator::Now()); //sjkang1221
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("Forward MC UE DATA through X2 interface");
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2uUdpPort));  /// should be transmitted to NgcX2:RecvFromX2Socket
}

void
NgcX2::DoReceiveMcPdcpSdu(NgcX2Sap::UeDataParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);
  NS_LOG_LOGIC ("gtpTeid = " << params.gtpTeid);

  //NS_LOG_UNCOND( m_x2InterfaceSockets.find(params.targetCellId)== m_x2InterfaceSockets.end() );
  //NS_LOG_UNCOND("forward data to other Enb");
  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for targetCellId = " << params.targetCellId<<this);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localUserPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  // add a message type to the gtpu header, so that it is possible to distinguish at receiver
  NrGtpuHeader gtpu;
  gtpu.SetTeid (params.gtpTeid);
  gtpu.SetMessageType(NgcX2Header::McForwardUplinkData);
  gtpu.SetLength (params.ueData->GetSize () + gtpu.GetSerializedSize () - 8); /// \todo This should be done in NrGtpuHeader
  NS_LOG_INFO ("GTP-U header: " << gtpu);

  Ptr<Packet> packet = params.ueData;
  packet->AddHeader (gtpu);

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  NS_LOG_INFO ("Forward MC UE DATA through X2 interface");
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2uUdpPort));  
}
void
NgcX2::DoReceiveAssistantInformation(NgcX2Sap::AssistantInformationForSplitting params){ //sjkang
NS_LOG_INFO("received assistant information at X2" << params.sourceCellId << "\t" << params.targetCellId);
NS_LOG_FUNCTION (this);

 NS_LOG_INFO ("sourceCellId = " << params.sourceCellId);
 NS_LOG_INFO("targetCellId = " << params.targetCellId);
 //NS_LOG_INFO ("gtpTeid = " << params.gtpTeid);

// NS_LOG_UNCOND( m_x2InterfaceSockets.find(params.targetCellId)== m_x2InterfaceSockets.end() );

 NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                "Missing infos for targetCellId = " << params.targetCellId);
 Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
 Ptr<Socket> sourceSocket = socketInfo->m_localUserPlaneSocket;
 Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

 NS_LOG_INFO ("sourceSocket = " << sourceSocket);
 NS_LOG_INFO ("targetIpAddr = " << targetIpAddr);
NS_LOG_INFO (this);
 NgcX2AssistantInfoHeader assistantInfoHeader;
 assistantInfoHeader.SetReTxQueingDelay(params.Re_Tx_Q_Delay);
 assistantInfoHeader.SetTxonQueingDelay(params.Tx_On_Q_Delay);
 assistantInfoHeader.SetRetxQueue(params.Re_TX_Q_Size);
 assistantInfoHeader.SetTxedQueue(params.Txed_Q_Size);
 assistantInfoHeader.SetTxonQueue(params.Tx_On_Q_Size);
 assistantInfoHeader.SetSourceCellId(params.sourceCellId);
 assistantInfoHeader.SetDrbId(params.drbId);
 assistantInfoHeader.SetRnti(params.rnti);
 NgcX2Header ngcX2Header;
 ngcX2Header.SetMessageType(NgcX2Header::McAssistantInfoForwarding);
 ngcX2Header.SetProcedureCode(NgcX2Header::SendingAssistantInformation);
 ngcX2Header.SetLengthOfIes(assistantInfoHeader.GetSerializedSize());
 ngcX2Header.SetNumberOfIes(assistantInfoHeader.GetNumbefOfIes());

 Ptr<Packet> packet =Create<Packet>();
 packet->AddHeader(assistantInfoHeader);
 packet->AddHeader(ngcX2Header);

 NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));
}
void
NgcX2::DoSendUeSinrUpdate(NgcX2Sap::UeImsiSinrParams params)
{
 // NS_LOG_FUNCTION(this);

  NS_LOG_LOGIC ("sourceCellId = " << params.sourceCellId);
  NS_LOG_LOGIC ("targetCellId = " << params.targetCellId);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.targetCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for targetCellId = " << params.targetCellId);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localUserPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;
//std::cout << this <<"\t"<<  targetIpAddr << " from  DoSendUeSinrUpdaat "<<params.targetCellId<<std::endl;
  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  // Build the X2 message
  NgcX2UeImsiSinrUpdateHeader x2imsiSinrHeader;
  x2imsiSinrHeader.SetUeImsiSinrMap (params.ueImsiSinrMap);
  x2imsiSinrHeader.SetSourceCellId (params.sourceCellId);
  x2imsiSinrHeader.SetSecondCellId(params.secondBestCellId); //sjkang
  x2imsiSinrHeader.SetRnti(params.m_rnti); //sjkang
  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::UpdateUeSinr);
  x2Header.SetLengthOfIes (x2imsiSinrHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2imsiSinrHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 UeImsiSinrUpdate header: " << x2imsiSinrHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2imsiSinrHeader);
  packet->AddHeader (x2Header);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);
//std::cout << targetIpAddr<< std::endl;
  // Send the X2 message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));
}


void
NgcX2::DoSendMcHandoverRequest (NgcX2SapProvider::SecondaryHandoverParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("oldCellId = " << params.oldCellId);
  NS_LOG_LOGIC ("imsi = " << params.imsi);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.oldCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for oldCellId = " << params.oldCellId);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.oldCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send X2 message: REQUEST MC HANDOVER");

  // Build the X2 message
  NgcX2McHandoverHeader x2mcHeader;
  x2mcHeader.SetTargetCellId(params.targetCellId);
  x2mcHeader.SetImsi(params.imsi);
  x2mcHeader.SetOldCellId(params.oldCellId);
  x2mcHeader.SetSecondMmWaveCellId(params.secondMmWaveCellID);
  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::RequestMcHandover);
  x2Header.SetLengthOfIes (x2mcHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2mcHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 RequestMcHandover header: " << x2mcHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2mcHeader);
  packet->AddHeader (x2Header);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  // Send the X2 message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));
}

void
NgcX2::DoNotifyNrMmWaveHandoverCompleted (NgcX2SapProvider::SecondaryHandoverParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("nrCellId = " << params.oldCellId);
  NS_LOG_LOGIC ("imsi = " << params.imsi);
  NS_LOG_LOGIC ("MmWave cellId = " << params.targetCellId);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.oldCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for oldCellId = " << params.oldCellId);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.oldCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send X2 message: NOTIFY MMWAVE HANDOVER TO NR");

  // Build the X2 message
  NgcX2McHandoverHeader x2mcHeader;
  x2mcHeader.SetTargetCellId(params.targetCellId);
  x2mcHeader.SetImsi(params.imsi);
  x2mcHeader.SetOldCellId(params.oldCellId);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::NotifyMmWaveNrHandover);
  x2Header.SetLengthOfIes (x2mcHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2mcHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 RequestMcHandover header: " << x2mcHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2mcHeader);
  packet->AddHeader (x2Header);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  // Send the X2 message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));
}

void
NgcX2::DoSendSwitchConnectionToMmWave(NgcX2SapProvider::SwitchConnectionParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("MmWaveCellId = " << params.mmWaveCellId);
  NS_LOG_LOGIC ("MmWaveRnti = " << params.mmWaveRnti);
  NS_LOG_LOGIC ("UseMmWaveConnection " << params.useMmWaveConnection);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.mmWaveCellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for mmWaveCellId = " << params.mmWaveCellId);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.mmWaveCellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send X2 message: SEND CONNECTION SWITCH MESSAGE");

  // Build the X2 message
  NgcX2ConnectionSwitchHeader x2mcHeader;
  x2mcHeader.SetMmWaveRnti(params.mmWaveRnti);
  x2mcHeader.SetUseMmWaveConnection(params.useMmWaveConnection);
  x2mcHeader.SetDrbid(params.drbid);

  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::InitiatingMessage);
  x2Header.SetProcedureCode (NgcX2Header::SwitchConnection);
  x2Header.SetLengthOfIes (x2mcHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2mcHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 RequestMcHandover header: " << x2mcHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2mcHeader);
  packet->AddHeader (x2Header);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  // Send the X2 message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));
}

void 
NgcX2::DoSendSecondaryCellHandoverCompleted(NgcX2SapProvider::SecondaryHandoverCompletedParams params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_LOGIC ("MmWaveRnti = " << params.mmWaveRnti);
  NS_LOG_LOGIC ("Imsi = " << params.imsi);
  NS_LOG_LOGIC ("oldEnbUeX2apId = " << params.oldEnbUeX2apId);
  NS_LOG_LOGIC ("Dst cellId = " << params.cellId);

  NS_ASSERT_MSG (m_x2InterfaceSockets.find (params.cellId) != m_x2InterfaceSockets.end (),
                 "Missing infos for cellId = " << params.cellId);
  Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.cellId];
  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;

  NS_LOG_LOGIC ("sourceSocket = " << sourceSocket);
  NS_LOG_LOGIC ("targetIpAddr = " << targetIpAddr);

  NS_LOG_INFO ("Send X2 message: SEND SECONDARY CELL HANDOVER COMPLETED MESSAGE");

  // Build the X2 message
  NgcX2SecondaryCellHandoverCompletedHeader x2hoHeader;
  x2hoHeader.SetMmWaveRnti(params.mmWaveRnti);
  x2hoHeader.SetImsi(params.imsi);
  x2hoHeader.SetOldEnbUeX2apId(params.oldEnbUeX2apId);
  x2hoHeader.SetisMc (params.isMc); //sjkang0710
  x2hoHeader.SetisMc_2(params.isMc_2); //sjkang0710
  NgcX2Header x2Header;
  x2Header.SetMessageType (NgcX2Header::SuccessfulOutcome);
  x2Header.SetProcedureCode (NgcX2Header::SecondaryCellHandoverCompleted);
  x2Header.SetLengthOfIes (x2hoHeader.GetLengthOfIes ());
  x2Header.SetNumberOfIes (x2hoHeader.GetNumberOfIes ());

  NS_LOG_INFO ("X2 header: " << x2Header);
  NS_LOG_INFO ("X2 SecondaryCellHandoverCompleted header: " << x2hoHeader);

  // Build the X2 packet
  Ptr<Packet> packet = Create <Packet> ();
  packet->AddHeader (x2hoHeader);
  packet->AddHeader (x2Header);
  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

  NgcX2Tag tag (Simulator::Now());
  packet->AddPacketTag (tag);

  // Send the X2 message through the socket
  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));
}
void
NgcX2::DoDuplicateRlcBuffer(NgcX2SapProvider::SendBufferDuplicationMessage params){
	 Ptr<NrX2IfaceInfo> socketInfo = m_x2InterfaceSockets [params.targetCellId];
	  Ptr<Socket> sourceSocket = socketInfo->m_localCtrlPlaneSocket;
	  Ipv4Address targetIpAddr = socketInfo->m_remoteIpAddr;



	  // Build the X2 message
	  NgcX2DuplicationRlcBufferHeader duplHeader;
	  duplHeader.SetImsi(params.imsi);
	  duplHeader.SetTargetCellID(params.targetCellId);
	  duplHeader.SetCellIDForBufferForwarding(params.cellIDforBufferForwarding);
	  duplHeader.SetOption(params.option);

	  	  NgcX2Header x2Header;
	  x2Header.SetProcedureCode (NgcX2Header::BufferDuplication);
	  x2Header.SetMessageType (NgcX2Header::DuplicationRlcBuffer);
	  x2Header.SetLengthOfIes (duplHeader.GetLengthOfIes ());
	  x2Header.SetNumberOfIes (duplHeader.GetNumberOfIes ());


	  // Build the X2 packet
	  Ptr<Packet> packet = Create <Packet> ();
	  packet->AddHeader (duplHeader);
	  packet->AddHeader (x2Header);
	  NS_LOG_INFO ("packetLen = " << packet->GetSize ());

	  NgcX2Tag tag (Simulator::Now());
	  packet->AddPacketTag (tag);
    // Send the X2 message through the socket
	  sourceSocket->SendTo (packet, 0, InetSocketAddress (targetIpAddr, m_x2cUdpPort));
}

} // namespace ns3
