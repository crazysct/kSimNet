/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef NGC_X2_SAP_H
#define NGC_X2_SAP_H

#include "ns3/packet.h"
#include "ns3/ipv4-address.h"
#include <ns3/nr-rrc-sap.h>
#include <ns3/nr-enb-cmac-sap.h>
#include <bitset>
#include <map>

namespace ns3 {


class Node;

/**
 * The X2 SAP defines the service between the X2 entity and the RRC entity.
 * 
 * The X2 SAP follows the specification 3GPP TS 36.423: "X2 application protocol (X2AP)"
 * 
 * The service primitives corresponds to the X2AP procedures and messages and 
 * the service parameters corresponds to the Information Elements
 * 
 * Note: Any reference in this file refers to the 3GPP TS 36.423 specification
 */

/**
 * \brief Common structures for NgcX2SapProvider and NgcX2SapUser
 */
class NgcX2Sap
{
public:
  virtual ~NgcX2Sap ();
  
  /**
   * E-RABs to be setup item as
   * it is used in the HANDOVER REQUEST message.
   * See section 9.1.1.1 for further info about the parameters
   */
  struct ErabToBeSetupItem
  {
    uint16_t    erabId;
    EpsBearer   erabLevelQosParameters;
    bool        dlForwarding;
    Ipv4Address transportLayerAddress;
    uint32_t    gtpTeid;

    ErabToBeSetupItem ();
  };

  /**
   * E-RABs admitted item as
   * it is used in the HANDOVER REQUEST ACKNOWLEDGE message.
   * See section 9.1.1.2 for further info about the parameters
   */
  struct ErabAdmittedItem
  {
    uint16_t    erabId;
    uint32_t    ulGtpTeid;
    uint32_t    dlGtpTeid;
  };

  /**
   * E-RABs not admitted item as
   * it is used in the HANDOVER REQUEST ACKNOWLEDGE message.
   * See section 9.1.1.2 for further info about the parameters
   */
  struct ErabNotAdmittedItem
  {
    uint16_t    erabId;
    uint16_t    cause;
  };

  /**
   * E-RABs subject to status transfer item as
   * it is used in the SN STATUS TRANSFER message.
   * See section 9.1.1.4 for further info about the parameters
   */
  static const uint16_t m_maxPdcpSn = 4096;
  struct ErabsSubjectToStatusTransferItem
  {
    uint16_t            erabId;
    std::bitset<m_maxPdcpSn> receiveStatusOfUlPdcpSdus;
    uint16_t            ulPdcpSn;
    uint32_t            ulHfn;
    uint16_t            dlPdcpSn;
    uint32_t            dlHfn;
  };

  /**
   * UL Interference OverloadIndication as
   * it is used in the LOAD INFORMATION message.
   * See section 9.2.17 for further info about the values
   */
  enum UlInterferenceOverloadIndicationItem
  {
    HighInterference,
    MediumInterference,
    LowInterference
  };

  /**
   * UL High Interference Information as
   * it is used in the LOAD INFORMATION message.
   * See section 9.1.2.1 for further info about the parameters
   */
  struct UlHighInterferenceInformationItem
  {
    uint16_t    targetCellId;
    std::vector <bool> ulHighInterferenceIndicationList;
  };

  /**
   * Relative Narrowband Tx Power (RNTP) as
   * it is used in the LOAD INFORMATION message.
   * See section 9.2.19 for further info about the parameters
   * 
   * Note: You can use INT16_MIN value for -infinite value
   *       in the rntpThreshold field
   */
  struct RelativeNarrowbandTxBand
  {
    std::vector <bool> rntpPerPrbList;
    int16_t     rntpThreshold;
    uint16_t    antennaPorts;
    uint16_t    pB;
    uint16_t    pdcchInterferenceImpact;
  };

  /**
   * Cell Information Item as
   * it is used in the LOAD INFORMATION message.
   * See section 9.1.2.1 for further info about the parameters
   */
  struct CellInformationItem
  {
    uint16_t    sourceCellId;
    std::vector <UlInterferenceOverloadIndicationItem> ulInterferenceOverloadIndicationList;
    std::vector <UlHighInterferenceInformationItem> ulHighInterferenceInformationList;
    RelativeNarrowbandTxBand relativeNarrowbandTxBand;
  };

  /**
   * Load Indicator as
   * it is used in the RESOURCE STATUS UPDATE message.
   * See section 9.2.36 for further info about the value
   */
  enum LoadIndicator
  {
    LowLoad,
    MediumLoad,
    HighLoad,
    Overload
  };

  /**
   * Composite Available Capacity as
   * it is used in the RESOURCE STATUS UPDATE message.
   * See section 9.2.45 for further info about the parameters
   */
  struct CompositeAvailCapacity
  {
    uint16_t    cellCapacityClassValue;
    uint16_t    capacityValue;
  };

  /**
   * Cell Measurement Result Item as
   * it is used in the RESOURCE STATUS UPDATE message.
   * See section 9.1.2.14 for further info about the parameters
   */
  struct CellMeasurementResultItem
  {
    uint16_t        sourceCellId;

    LoadIndicator   dlHardwareLoadIndicator;
    LoadIndicator   ulHardwareLoadIndicator;

    LoadIndicator   dlN2TnlLoadIndicator;
    LoadIndicator   ulN2TnlLoadIndicator;

    uint16_t        dlGbrPrbUsage;
    uint16_t        ulGbrPrbUsage;
    uint16_t        dlNonGbrPrbUsage;
    uint16_t        ulNonGbrPrbUsage;
    uint16_t        dlTotalPrbUsage;
    uint16_t        ulTotalPrbUsage;

    CompositeAvailCapacity  dlCompositeAvailableCapacity;
    CompositeAvailCapacity  ulCompositeAvailableCapacity;
  };


  enum IdCause
  {
    HandoverDesirableForRadioReason,
    TimeCriticalHandover
  };

  /**
   * \brief Parameters of the RlcSetupRequest to handle MC connectivity
   *
   * Forward UE params during the MC setup
   */
  struct RlcSetupRequest
  {
    uint16_t    sourceCellId;
    uint16_t    targetCellId;
    uint32_t    gtpTeid;
    uint16_t    mmWaveRnti;
    uint16_t    nrRnti;
    uint8_t     drbid;
    NrEnbCmacSapProvider::LcInfo lcinfo;
    NrRrcSap::RlcConfig rlcConfig;
    NrRrcSap::LogicalChannelConfig logicalChannelConfig;
    TypeId      rlcType;
  };



  /**
   * \brief Parameters of the HANDOVER REQUEST message.
   *
   * See section 9.1.1.1 for further info about the parameters
   */
  struct HandoverRequestParams
  {
    uint16_t            oldEnbUeX2apId;
    uint16_t            cause;
    uint16_t            sourceCellId;
    uint16_t            targetCellId;
    uint16_t 				nrCellId; //sjkang
    uint32_t            amfUeN2apId;
    uint64_t            ueAggregateMaxBitRateDownlink;
    uint64_t            ueAggregateMaxBitRateUplink;
    bool                isMc;
    bool					isMC_2;
   // uint16_t 			   imsi;
    std::vector <ErabToBeSetupItem> bearers;
    // list of RlcSetupRequest for secondary cell handovers (otherwise empty)
    std::vector <RlcSetupRequest> rlcRequests;
    Ptr<Packet>         rrcContext;
  };

  /**
   * \brief Parameters of the HANDOVER REQUEST ACKNOWLEDGE message.
   *
   * See section 9.1.1.2 for further info about the parameters
   */
  struct HandoverRequestAckParams
  {
    uint16_t            oldEnbUeX2apId;
    uint16_t            newEnbUeX2apId;
    uint16_t            sourceCellId;
    uint16_t            targetCellId;
    std::vector <ErabAdmittedItem> admittedBearers;
    std::vector <ErabNotAdmittedItem> notAdmittedBearers;
    Ptr<Packet>         rrcContext;
  };

  /**
   * \brief Parameters of the HANDOVER PREPARATION FAILURE message.
   *
   * See section 9.1.1.3 for further info about the parameters
   */
  struct HandoverPreparationFailureParams
  {
    uint16_t            oldEnbUeX2apId;
    uint16_t            sourceCellId;
    uint16_t            targetCellId;
    uint16_t            cause;
    uint16_t            criticalityDiagnostics;
  };

  /**
   * \brief Parameters of the SN STATUS TRANSFER message.
   *
   * See section 9.1.1.4 for further info about the parameters
   */
  struct SnStatusTransferParams
  {
    uint16_t            oldEnbUeX2apId;
    uint16_t            newEnbUeX2apId;
    uint16_t            sourceCellId;
    uint16_t            targetCellId;
    std::vector <ErabsSubjectToStatusTransferItem> erabsSubjectToStatusTransferList;
  };

  /**
   * \brief Parameters of the UE CONTEXT RELEASE message.
   *
   * See section 9.1.1.5 for further info about the parameters
   */
  struct UeContextReleaseParams
  {
    uint16_t            oldEnbUeX2apId;
    uint16_t            newEnbUeX2apId;
    uint16_t            sourceCellId;
    uint16_t            targetCellId;
  };

  /**
   * \brief Parameters of the LOAD INFORMATION message.
   *
   * See section 9.1.2.1 for further info about the parameters
   */
  struct LoadInformationParams
  {
    uint16_t            targetCellId;
    std::vector <CellInformationItem> cellInformationList;
  };

  /**
   * \brief Parameters of the RESOURCE STATUS UPDATE message.
   *
   * See section 9.1.2.14 for further info about the parameters
   */
  struct ResourceStatusUpdateParams
  {
    uint16_t            targetCellId;
    uint16_t            enb1MeasurementId;
    uint16_t            enb2MeasurementId;
    std::vector <CellMeasurementResultItem> cellMeasurementResultList;
  };

  /**
   * \brief Parameters of the UE DATA primitive
   *
   * Forward UE data during the handover procedure from source eNB (sourceCellId)
   * to target eNB (targetCellId) using a GTP-U tunnel (gtpTeid)
   */
  struct UeDataParams
  {
    uint16_t    sourceCellId;
    uint16_t    targetCellId;
    uint32_t    gtpTeid;
    Ptr<Packet> ueData;
  };

  struct SecondaryHandoverParams
  {
    uint64_t imsi;
    uint16_t oldCellId;
    uint16_t targetCellId;
    uint16_t secondMmWaveCellID; //sjkang
  };
struct SendBufferDuplicationMessage
{
	    uint64_t imsi;
	    uint16_t targetCellId;
	    uint16_t cellIDforBufferForwarding;
	    bool 	option;
};
  struct SecondaryHandoverCompletedParams
  {
    uint64_t imsi;
    uint16_t mmWaveRnti;
    uint16_t cellId;
    uint16_t oldEnbUeX2apId;
    bool 	  isMc;
    bool    isMc_2;
  };

  struct UeImsiSinrParams
  {
    uint16_t    sourceCellId;
    uint16_t    targetCellId;
    uint16_t    secondBestCellId=0;
    uint16_t 	m_rnti;//sjkang
    std::map<uint64_t, double> ueImsiSinrMap; 
  };

  struct HandoverFailedParams
  {
    uint64_t imsi;
    uint16_t coordinatorId;
    uint16_t sourceCellId;
    uint16_t targetCellId;
  };

  struct SwitchConnectionParams
  {
    uint32_t mmWaveRnti;
    uint16_t mmWaveCellId;
    uint8_t drbid;
    bool useMmWaveConnection;
  }; 

  struct AssistantInformationForSplitting{ //sjkang
	  uint16_t targetCellId;
	  uint16_t drbId;
	  uint16_t sourceCellId;
	  uint16_t rnti;
  	  uint32_t Re_TX_Q_Size=0;
  	  uint32_t Tx_On_Q_Size=0;
  	  uint32_t Txed_Q_Size=0;
  	  uint32_t Re_Tx_Q_Delay=0;
  	  uint32_t Tx_On_Q_Delay=0;
  	  uint32_t Txed_Q_Delay=0;
  };
};


/**
 * MC primitives. Part of X2 entity, called by PDCP
 */
class NgcX2PdcpProvider : public NgcX2Sap
{
public:
  virtual ~NgcX2PdcpProvider ();

  /*
   * Service primitives
   */
  // X2 sends a Pdcp PDU in downlink to the MmWave eNB for transmission to the UE
  virtual void SendMcPdcpPdu (UeDataParams params) = 0;
  virtual void ReceiveAssistantInformation (AssistantInformationForSplitting info)=0;
};


/**
 * MC primitives. Part of PDCP entity, called by X2
 */
class NgcX2PdcpUser : public NgcX2Sap
{
public:
  virtual ~NgcX2PdcpUser ();

  /*
   * Service primitives
   */
  // Receive a PDCP PDU in uplink from the MmWave eNB for transmission to CN
  virtual void ReceiveMcPdcpPdu (UeDataParams params) = 0;
 virtual void ReceiveAssistantInformation(AssistantInformationForSplitting Info)=0; //sjkang
};


/**
 * MC primitives. Part of X2 entity, called by RLC
 */
class NgcX2RlcProvider : public NgcX2Sap
{
public:
  virtual ~NgcX2RlcProvider ();

  /*
   * Service primitives
   */
  // Receive a PDCP SDU from RLC for uplink transmission to PDCP in NR eNB
  virtual void ReceiveMcPdcpSdu (UeDataParams params) = 0;
  virtual void ReceiveAssistantInformation(AssistantInformationForSplitting info)=0; //sjkang
};


/**
 * MC primitives. Part of RLC entity, called by X2
 */
class NgcX2RlcUser : public NgcX2Sap
{
public:
  virtual ~NgcX2RlcUser ();

  /*
   * Service primitives
   */
  // X2 sends a PDCP SDU to RLC for downlink transmission to the UE
  virtual void SendMcPdcpSdu (UeDataParams params) = 0;
};

/**
 * These service primitives of this part of the X2 SAP
 * are provided by the X2 entity and issued by RRC entity
 */
class NgcX2SapProvider : public NgcX2Sap
{
public:
  virtual ~NgcX2SapProvider ();

  /**
   * Service primitives
   */

  virtual void SendHandoverRequest (HandoverRequestParams params) = 0;

  virtual void SendHandoverRequestAck (HandoverRequestAckParams params) = 0;
  virtual void SendHandoverRequestAckToNr(HandoverRequestAckParams params)=0; //sjkang0416
  virtual void SendHandoverPreparationFailure (HandoverPreparationFailureParams params) = 0;

  virtual void SendSnStatusTransfer (SnStatusTransferParams params) = 0;

  virtual void SendUeContextRelease (UeContextReleaseParams params) = 0;

  virtual void SendLoadInformation (LoadInformationParams params) = 0;

  virtual void SendResourceStatusUpdate (ResourceStatusUpdateParams params) = 0;

  virtual void SendUeData (UeDataParams params) = 0;

  virtual void SetNgcX2PdcpUser (uint32_t teid, NgcX2PdcpUser * s) = 0;

  virtual void SetNgcX2RlcUser (uint32_t teid, NgcX2RlcUser * s) = 0;

  virtual void SetNgcX2PdcpUser_2 (uint32_t teid, NgcX2PdcpUser * s) = 0;//sjkang1016

   virtual void SetNgcX2RlcUser_2 (uint32_t teid, NgcX2RlcUser * s) = 0; //sjkang1016

  virtual void SendRlcSetupRequest (RlcSetupRequest params) = 0;

  virtual void SendRlcSetupCompleted (UeDataParams params) = 0;

  virtual void SendUeSinrUpdate (UeImsiSinrParams params) = 0;

  virtual void NotifyNrMmWaveHandoverCompleted (SecondaryHandoverParams params) = 0;

  virtual void NotifyCoordinatorHandoverFailed (HandoverFailedParams params) = 0;

  // send the switch command to the mmwave eNB
  virtual void SendSwitchConnectionToMmWave (SwitchConnectionParams params) = 0;

  // for secondary cell HO
  // the coordinator requests to a mmWave eNB to start an handover
  virtual void SendMcHandoverRequest (SecondaryHandoverParams params) = 0;
  virtual void DuplicateRlcBuffer(SendBufferDuplicationMessage params) = 0; //sjkang
//  virtual void ForwardRlcBuffer(SendBufferDuplicationMessage params) =0; //sjkang
  // notify the coordinator that the secondary cell handover is completed
  virtual void SendSecondaryCellHandoverCompleted (SecondaryHandoverCompletedParams params) = 0;
  // notify the NgcX2 class that packets for a certain TEID must be forwarded to the targetCell
  virtual void AddTeidToBeForwarded (uint32_t gtpTeid, uint16_t targetCellId) = 0;
  // notify the NgcX2 class that packets for a certain TEID must not be forwarded anymore
  virtual void RemoveTeidToBeForwarded (uint32_t gtpTeid) = 0;
  // to forward the packets in the RLC buffers in the source cell as if they were generated by a PDCP
  virtual void ForwardRlcPdu (UeDataParams params) = 0;

};


/**
 * These service primitives of this part of the X2 SAP
 * are provided by the RRC entity and issued by the X2 entity
 */
class NgcX2SapUser : public NgcX2Sap
{
public:
  virtual ~NgcX2SapUser ();

  /**
   * Service primitives
   */

  virtual void RecvHandoverRequest (HandoverRequestParams params) = 0;
  virtual void RecvAssistantInformation(AssistantInformationForSplitting info)=0; //sjkang
  virtual void RecvBufferDuplicationMessage(SendBufferDuplicationMessage message) = 0; //sjkang
  virtual void RecvHandoverRequestAck (HandoverRequestAckParams params) = 0;
  virtual void RecvHandoverRequestAckToNr (HandoverRequestAckParams params) = 0;

  virtual void RecvHandoverPreparationFailure (HandoverPreparationFailureParams params) = 0;

  virtual void RecvSnStatusTransfer (SnStatusTransferParams params) = 0;

  virtual void RecvUeContextRelease (UeContextReleaseParams params) = 0;

  virtual void RecvLoadInformation (LoadInformationParams params) = 0;
  
  virtual void RecvResourceStatusUpdate (ResourceStatusUpdateParams params) = 0;

  virtual void RecvRlcSetupRequest (RlcSetupRequest params) = 0;

  virtual void RecvRlcSetupCompleted (UeDataParams params) = 0;

  virtual void RecvUeData (UeDataParams params) = 0;

  virtual void RecvUeSinrUpdate(UeImsiSinrParams params) = 0;

  virtual void RecvMcHandoverRequest (SecondaryHandoverParams params) = 0;

  virtual void RecvNrMmWaveHandoverCompleted (SecondaryHandoverParams params) = 0;

  virtual void RecvConnectionSwitchToMmWave (SwitchConnectionParams params) = 0;

  virtual void RecvSecondaryCellHandoverCompleted (SecondaryHandoverCompletedParams params) = 0;

};

///////////////////////////////////////

template <class C>
class NgcX2SpecificNgcX2SapProvider : public NgcX2SapProvider
{
public:
  NgcX2SpecificNgcX2SapProvider (C* x2);

  //
  // Interface implemented from NgcX2SapProvider
  //

  virtual void SendHandoverRequest (HandoverRequestParams params);

  virtual void SendHandoverRequestAck (HandoverRequestAckParams params);
  virtual void SendHandoverRequestAckToNr (HandoverRequestAckParams params);

  virtual void SendHandoverPreparationFailure (HandoverPreparationFailureParams params);

  virtual void SendSnStatusTransfer (SnStatusTransferParams params);

  virtual void SendUeContextRelease (UeContextReleaseParams params);

  virtual void SendLoadInformation (LoadInformationParams params);

  virtual void SendResourceStatusUpdate (ResourceStatusUpdateParams params);

  virtual void SendUeData (UeDataParams params);

  virtual void SetNgcX2PdcpUser (uint32_t teid, NgcX2PdcpUser * s);

  virtual void SetNgcX2RlcUser (uint32_t teid, NgcX2RlcUser * s);

  virtual void SetNgcX2PdcpUser_2 (uint32_t teid, NgcX2PdcpUser * s); //sjkang1016

   virtual void SetNgcX2RlcUser_2 (uint32_t teid, NgcX2RlcUser * s); //sjkang1016


  virtual void SendRlcSetupRequest (RlcSetupRequest params);

  virtual void SendRlcSetupCompleted (UeDataParams params);

  virtual void SendUeSinrUpdate (UeImsiSinrParams params);

  virtual void SendMcHandoverRequest (SecondaryHandoverParams params);
  virtual void DuplicateRlcBuffer(SendBufferDuplicationMessage params); //sjkang
 // virtual void ForwardRlcBuffer(SendBufferDuplicationMessage params); //sjkang
  virtual void NotifyNrMmWaveHandoverCompleted (SecondaryHandoverParams params);

  virtual void NotifyCoordinatorHandoverFailed (HandoverFailedParams params);

  virtual void SendSwitchConnectionToMmWave (SwitchConnectionParams params);

  virtual void SendSecondaryCellHandoverCompleted (SecondaryHandoverCompletedParams params);

  virtual void AddTeidToBeForwarded (uint32_t gtpTeid, uint16_t targetCellId);
  
  virtual void RemoveTeidToBeForwarded (uint32_t gtpTeid);

  virtual void ForwardRlcPdu (UeDataParams params);

private:
  NgcX2SpecificNgcX2SapProvider ();
  C* m_x2;
};

template <class C>
NgcX2SpecificNgcX2SapProvider<C>::NgcX2SpecificNgcX2SapProvider (C* x2)
  : m_x2 (x2)
{
}

template <class C>
NgcX2SpecificNgcX2SapProvider<C>::NgcX2SpecificNgcX2SapProvider ()
{
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendHandoverRequest (HandoverRequestParams params)
{
  m_x2->DoSendHandoverRequest (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendHandoverRequestAck (HandoverRequestAckParams params)
{
  m_x2->DoSendHandoverRequestAck (params);
}
template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendHandoverRequestAckToNr (HandoverRequestAckParams params) //sjkang0416
{
  m_x2->DoSendHandoverRequestAckToNr (params);
}
template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendHandoverPreparationFailure (HandoverPreparationFailureParams params)
{
  m_x2->DoSendHandoverPreparationFailure (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendSnStatusTransfer (SnStatusTransferParams params)
{
  m_x2->DoSendSnStatusTransfer (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendUeContextRelease (UeContextReleaseParams params)
{
  m_x2->DoSendUeContextRelease (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendLoadInformation (LoadInformationParams params)
{
  m_x2->DoSendLoadInformation (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendResourceStatusUpdate (ResourceStatusUpdateParams params)
{
  m_x2->DoSendResourceStatusUpdate (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendUeData (UeDataParams params)
{
  m_x2->DoSendUeData (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SetNgcX2RlcUser (uint32_t teid, NgcX2RlcUser * s)
{
  m_x2->SetMcNgcX2RlcUser (teid, s);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SetNgcX2PdcpUser (uint32_t teid, NgcX2PdcpUser * s)
{
  m_x2->SetMcNgcX2PdcpUser (teid, s);
}
template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SetNgcX2RlcUser_2 (uint32_t teid, NgcX2RlcUser * s) //sjkang1016
{
  m_x2->SetMcNgcX2RlcUser_2 (teid, s);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SetNgcX2PdcpUser_2 (uint32_t teid, NgcX2PdcpUser * s) //sjkang1016
{
  m_x2->SetMcNgcX2PdcpUser_2 (teid, s);
}
template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendRlcSetupRequest (RlcSetupRequest params)
{
  m_x2->DoSendRlcSetupRequest (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendRlcSetupCompleted (UeDataParams params)
{
  m_x2->DoSendRlcSetupCompleted (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendUeSinrUpdate (UeImsiSinrParams params)
{
  m_x2->DoSendUeSinrUpdate (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendMcHandoverRequest (SecondaryHandoverParams params)
{
  m_x2->DoSendMcHandoverRequest (params);
}
template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::DuplicateRlcBuffer(SendBufferDuplicationMessage params){ //sjkang
	m_x2->DoDuplicateRlcBuffer(params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::NotifyNrMmWaveHandoverCompleted (SecondaryHandoverParams params)
{
  m_x2->DoNotifyNrMmWaveHandoverCompleted(params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::NotifyCoordinatorHandoverFailed (HandoverFailedParams params)
{
  m_x2->DoNotifyCoordinatorHandoverFailed(params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendSwitchConnectionToMmWave (SwitchConnectionParams params)
{
  m_x2->DoSendSwitchConnectionToMmWave(params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::SendSecondaryCellHandoverCompleted (SecondaryHandoverCompletedParams params)
{
  m_x2->DoSendSecondaryCellHandoverCompleted(params);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::AddTeidToBeForwarded (uint32_t gtpTeid, uint16_t targetCellId)
{
  m_x2->DoAddTeidToBeForwarded(gtpTeid, targetCellId);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::RemoveTeidToBeForwarded (uint32_t gtpTeid)
{
  m_x2->DoRemoveTeidToBeForwarded(gtpTeid);
}

template <class C>
void
NgcX2SpecificNgcX2SapProvider<C>::ForwardRlcPdu(UeDataParams params)
{
  m_x2->DoSendMcPdcpPdu(params);
}

///////////////////////////////////////

template <class C>
class NgcX2SpecificNgcX2SapUser : public NgcX2SapUser
{
public:
  NgcX2SpecificNgcX2SapUser (C* rrc);

  //
  // Interface implemented from NgcX2SapUser
  //

  virtual void RecvHandoverRequest (HandoverRequestParams params);
  virtual void RecvAssistantInformation(AssistantInformationForSplitting info); //sjkang
  virtual void RecvBufferDuplicationMessage (SendBufferDuplicationMessage message); //sjkang
  virtual void RecvHandoverRequestAck (HandoverRequestAckParams params);
  virtual void RecvHandoverRequestAckToNr (HandoverRequestAckParams params); //sjkang

  virtual void RecvHandoverPreparationFailure (HandoverPreparationFailureParams params);

  virtual void RecvSnStatusTransfer (SnStatusTransferParams params);

  virtual void RecvUeContextRelease (UeContextReleaseParams params);

  virtual void RecvLoadInformation (LoadInformationParams params);

  virtual void RecvResourceStatusUpdate (ResourceStatusUpdateParams params);

  virtual void RecvRlcSetupRequest (RlcSetupRequest params);

  virtual void RecvRlcSetupCompleted (UeDataParams params);

  virtual void RecvUeData (UeDataParams params);

  virtual void RecvUeSinrUpdate (UeImsiSinrParams params);

  virtual void RecvMcHandoverRequest (SecondaryHandoverParams params);

  virtual void RecvNrMmWaveHandoverCompleted (SecondaryHandoverParams params);

  virtual void RecvConnectionSwitchToMmWave (SwitchConnectionParams params);

  virtual void RecvSecondaryCellHandoverCompleted (SecondaryHandoverCompletedParams params);


private:
  NgcX2SpecificNgcX2SapUser ();
  C* m_rrc;
};

template <class C>
NgcX2SpecificNgcX2SapUser<C>::NgcX2SpecificNgcX2SapUser (C* rrc)
  : m_rrc (rrc)
{
}

template <class C>
NgcX2SpecificNgcX2SapUser<C>::NgcX2SpecificNgcX2SapUser ()
{
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvHandoverRequest (HandoverRequestParams params)
{
  m_rrc->DoRecvHandoverRequest (params);
}
template <class C> //sjkang
void
NgcX2SpecificNgcX2SapUser<C>::RecvAssistantInformation (AssistantInformationForSplitting params)
{
  m_rrc->DoRecvAssistantInfo (params); //sjkang
}
template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvBufferDuplicationMessage(SendBufferDuplicationMessage message){
	m_rrc->DoRecvBufferDuplicationMessage(message);
}
template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvHandoverRequestAck (HandoverRequestAckParams params)
{
  m_rrc->DoRecvHandoverRequestAck (params);
}
template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvHandoverRequestAckToNr (HandoverRequestAckParams params)
{
  m_rrc->DoRecvHandoverRequestAckToNr (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvHandoverPreparationFailure (HandoverPreparationFailureParams params)
{
  m_rrc->DoRecvHandoverPreparationFailure (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvSnStatusTransfer (SnStatusTransferParams params)
{
  m_rrc->DoRecvSnStatusTransfer (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvUeContextRelease (UeContextReleaseParams params)
{
  m_rrc->DoRecvUeContextRelease (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvLoadInformation (LoadInformationParams params)
{
  m_rrc->DoRecvLoadInformation (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvResourceStatusUpdate (ResourceStatusUpdateParams params)
{
  m_rrc->DoRecvResourceStatusUpdate (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvRlcSetupRequest (RlcSetupRequest params)
{
  m_rrc->DoRecvRlcSetupRequest (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvRlcSetupCompleted (UeDataParams params)
{
  m_rrc->DoRecvRlcSetupCompleted (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvUeData (UeDataParams params)
{
  m_rrc->DoRecvUeData (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvUeSinrUpdate (UeImsiSinrParams params)
{
  m_rrc->DoRecvUeSinrUpdate (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvMcHandoverRequest (SecondaryHandoverParams params)
{
  m_rrc->DoRecvMcHandoverRequest (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvNrMmWaveHandoverCompleted (SecondaryHandoverParams params)
{
  m_rrc->DoRecvNrMmWaveHandoverCompleted (params);
}

template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvConnectionSwitchToMmWave (SwitchConnectionParams params)
{
  m_rrc->DoRecvConnectionSwitchToMmWave (params);
}


template <class C>
void
NgcX2SpecificNgcX2SapUser<C>::RecvSecondaryCellHandoverCompleted (SecondaryHandoverCompletedParams params)
{
  m_rrc->DoRecvSecondaryCellHandoverCompleted(params);
}

/////////////////////////////////////////////
template <class C>
class NgcX2PdcpSpecificProvider : public NgcX2PdcpProvider
{
public:
  NgcX2PdcpSpecificProvider (C* x2);

  // Inherited
  virtual void SendMcPdcpPdu (UeDataParams params);
  virtual void ReceiveAssistantInformation(AssistantInformationForSplitting info); //sjkang

private:
  NgcX2PdcpSpecificProvider ();
  C* m_x2;
};

template <class C>
NgcX2PdcpSpecificProvider<C>::NgcX2PdcpSpecificProvider (C* x2)
  : m_x2 (x2)
{
}

template <class C>
NgcX2PdcpSpecificProvider<C>::NgcX2PdcpSpecificProvider ()
{
}

template <class C>
void
NgcX2PdcpSpecificProvider<C>::SendMcPdcpPdu(UeDataParams params)
{
  m_x2->DoSendMcPdcpPdu(params);
}
template <class C>
void
NgcX2PdcpSpecificProvider<C>::ReceiveAssistantInformation(AssistantInformationForSplitting info) //sjkang
{
  m_x2->DoReceiveAssistantInformation(info);
}
/////////////////////////////////////////////
template <class C>
class NgcX2RlcSpecificProvider : public NgcX2RlcProvider
{
public:
  NgcX2RlcSpecificProvider (C* x2);

  // Inherited
  virtual void ReceiveMcPdcpSdu (UeDataParams params);
  virtual void ReceiveAssistantInformation(AssistantInformationForSplitting info); //sjkang
private:
  NgcX2RlcSpecificProvider ();
  C* m_x2;
};

template <class C>
NgcX2RlcSpecificProvider<C>::NgcX2RlcSpecificProvider (C* x2)
  : m_x2 (x2)
{
}

template <class C>
NgcX2RlcSpecificProvider<C>::NgcX2RlcSpecificProvider ()
{
}

template <class C>
void
NgcX2RlcSpecificProvider<C>::ReceiveMcPdcpSdu(UeDataParams params)
{
  m_x2->DoReceiveMcPdcpSdu(params);
}

template <class C>
void
NgcX2RlcSpecificProvider<C>::ReceiveAssistantInformation(AssistantInformationForSplitting params) //sjkang
{
  m_x2->DoReceiveAssistantInformation(params);
}
/////////////////////////////////////////////
template <class C>
class NgcX2PdcpSpecificUser : public NgcX2PdcpUser
{
public:
  NgcX2PdcpSpecificUser (C* pdcp);

  // Inherited
  virtual void ReceiveMcPdcpPdu (UeDataParams params);
  virtual void ReceiveAssistantInformation(AssistantInformationForSplitting info); //sjkang1114

private:
  NgcX2PdcpSpecificUser ();
  C* m_pdcp;
};

template <class C>
NgcX2PdcpSpecificUser<C>::NgcX2PdcpSpecificUser (C* pdcp)
  : m_pdcp (pdcp)
{
}

template <class C>
NgcX2PdcpSpecificUser<C>::NgcX2PdcpSpecificUser ()
{
}

template <class C>
void
NgcX2PdcpSpecificUser<C>::ReceiveMcPdcpPdu(UeDataParams params)
{
  m_pdcp->DoReceiveMcPdcpPdu(params);
}
template <class C>
void
NgcX2PdcpSpecificUser<C>::ReceiveAssistantInformation(AssistantInformationForSplitting params)
{
  m_pdcp->DoReceiveAssistantInformation(params);
}

/////////////////////////////////////////////
template <class C>
class NgcX2RlcSpecificUser : public NgcX2RlcUser
{
public:
  NgcX2RlcSpecificUser (C* rlc);

  // Inherited
  virtual void SendMcPdcpSdu (UeDataParams params);

private:
  NgcX2RlcSpecificUser ();
  C* m_rlc;
};

template <class C>
NgcX2RlcSpecificUser<C>::NgcX2RlcSpecificUser (C* rlc)
  : m_rlc (rlc)
{
}

template <class C>
NgcX2RlcSpecificUser<C>::NgcX2RlcSpecificUser ()
{
}

template <class C>
void
NgcX2RlcSpecificUser<C>::SendMcPdcpSdu(UeDataParams params)
{
  m_rlc->DoSendMcPdcpSdu(params);
}

} // namespace ns3

#endif // NGC_X2_SAP_H
