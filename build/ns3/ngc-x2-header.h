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

#ifndef NGC_X2_HEADER_H
#define NGC_X2_HEADER_H

#include "ns3/ngc-x2-sap.h"
#include "ns3/header.h"

#include <vector>


namespace ns3 {


class NgcX2Header : public Header
{
public:
  NgcX2Header ();
  virtual ~NgcX2Header ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint8_t GetMessageType () const;
  void SetMessageType (uint8_t messageType);

  uint8_t GetProcedureCode () const;
  void SetProcedureCode (uint8_t procedureCode);

  void SetLengthOfIes (uint32_t lengthOfIes);
  void SetNumberOfIes (uint32_t numberOfIes);


  enum ProcedureCode_t {
    HandoverPreparation     = 0,
    LoadIndication          = 2,
    SnStatusTransfer        = 4,
    UeContextRelease        = 5,
    ResourceStatusReporting = 10,
    RlcSetupRequest         = 11, // added for MC functionalities
    RlcSetupCompleted       = 12,
    NotifyMcConnection      = 13,
    UpdateUeSinr            = 14,
    RequestMcHandover       = 15,
    NotifyMmWaveNrHandover = 16,
    NotifyCoordinatorHandoverFailed = 17,
    SwitchConnection        = 18,
    SecondaryCellHandoverCompleted = 19,
	SendingAssistantInformation =20,
	BufferDuplication = 21


  };

  enum TypeOfMessage_t {
    InitiatingMessage       = 0,
    SuccessfulOutcome       = 1,
    UnsuccessfulOutcome     = 2,
    McForwardDownlinkData   = 3, // added for MC functionalities
    McForwardUplinkData     = 4,
	McAssistantInfoForwarding =5,  // for sending assistant information by sjkang1114
	SuccesfulOutcomToNr = 6 , // for sending ack to NR eNB by sjkang0416
	DuplicationRlcBuffer = 7 ///for sending NLOS eNB to duplicate RLC buffer and send it to LOS eNB


  };

private:
  uint8_t m_messageType;
  uint8_t m_procedureCode;

  uint32_t m_lengthOfIes;
  uint32_t m_numberOfIes;
};
class NgcX2AssistantInfoHeader : public Header{ //sjkang1114
public:
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	NgcX2AssistantInfoHeader();
     ~NgcX2AssistantInfoHeader();
	virtual void Print (std::ostream &os) const;
	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (Buffer::Iterator start) const;
	virtual uint32_t Deserialize (Buffer::Iterator start);

public:
	uint32_t GetTxonQueue() const;
	void SetTxonQueue(uint32_t TxOnQueue);
	uint32_t GetTxedQueue() const;
	void SetTxedQueue(uint32_t TxedQueue);
	uint32_t GetRetxQueue() const;
	void SetRetxQueue(uint32_t retxQ);
	uint32_t GetTxonQueingDelay() const;
	void SetTxonQueingDelay(uint32_t queingdelay);
	uint32_t GetReTxQueuingDelay() const;
	void SetReTxQueingDelay (uint32_t retxqueingdelay);
	uint8_t GetSourceCellId() const;
	void SetSourceCellId(uint8_t cellId);
	 uint16_t GetNumbefOfIes();
	 void SetRnti(uint8_t rnti);
	 uint8_t GetRnti();
	 void SetDrbId(uint8_t drbid);
	 uint8_t GetDrbId();
private:
	uint32_t TxonQueueSize;
	uint32_t TxedQueueSize;
	uint32_t RetxQueueSize;
	uint32_t TxQueingDelay;
	uint32_t ReTxQueingDelay;
	uint8_t sourceCellId;
	uint8_t drbId;
	uint8_t rnti;
	uint32_t headerLength;
};

class NgcX2HandoverRequestHeader : public Header
{
public:
  NgcX2HandoverRequestHeader ();
  virtual ~NgcX2HandoverRequestHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint16_t GetOldEnbUeX2apId () const;
  void SetOldEnbUeX2apId (uint16_t x2apId);

  uint16_t GetCause () const;
  void SetCause (uint16_t cause);

  bool GetIsMc () const;
  void SetIsMc (bool isMc);
  bool GetIsMc_2 () const;  //sjkang
   void SetIsMc_2 (bool isMc_2); //sjkang

  uint16_t GetTargetCellId () const;
  void SetTargetCellId (uint16_t targetCellId);


  uint32_t GetAmfUeN2apId () const;
  void SetAmfUeN2apId (uint32_t amfUeN2apId);

  std::vector <NgcX2Sap::ErabToBeSetupItem> GetBearers () const;
  void SetBearers (std::vector <NgcX2Sap::ErabToBeSetupItem> bearers);

  std::vector <NgcX2Sap::RlcSetupRequest> GetRlcSetupRequests () const;
  void SetRlcSetupRequests (std::vector <NgcX2Sap::RlcSetupRequest> rlcRequests);

  uint64_t GetUeAggregateMaxBitRateDownlink () const;
  void SetUeAggregateMaxBitRateDownlink (uint64_t bitRate);

  uint64_t GetUeAggregateMaxBitRateUplink () const;
  void SetUeAggregateMaxBitRateUplink (uint64_t bitRate);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_oldEnbUeX2apId;
  uint16_t          m_cause;
  uint16_t          m_targetCellId;
 // uint16_t				m_nrCellId;
  uint32_t          m_amfUeN2apId;
  uint64_t          m_ueAggregateMaxBitRateDownlink;
  uint64_t          m_ueAggregateMaxBitRateUplink;
  std::vector <NgcX2Sap::ErabToBeSetupItem> m_erabsToBeSetupList;
  std::vector <NgcX2Sap::RlcSetupRequest> m_rlcRequestsList;
  bool              m_isMc;
  bool					m_isMc_2; //sjkang
};


class NgcX2RlcSetupRequestHeader : public Header
{
public:
  NgcX2RlcSetupRequestHeader ();
  virtual ~NgcX2RlcSetupRequestHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  uint16_t GetSourceCellId () const;
  void SetSourceCellId (uint16_t sourceCellId);

  uint16_t GetTargetCellId () const;
  void SetTargetCellId (uint16_t targetCellId);

  uint32_t GetGtpTeid () const;
  void SetGtpTeid (uint32_t gtpTeid);

  uint16_t GetMmWaveRnti () const;
  void SetMmWaveRnti (uint16_t rnti);

  uint16_t GetNrRnti () const;
  void SetNrRnti (uint16_t rnti);

  uint8_t GetDrbid () const;
  void SetDrbid (uint8_t drbid);

  NrEnbCmacSapProvider::LcInfo GetLcInfo() const;
  void SetLcInfo(NrEnbCmacSapProvider::LcInfo lcInfo);

  NrRrcSap::RlcConfig GetRlcConfig() const;
  void SetRlcConfig(NrRrcSap::RlcConfig rlcConfig);

  NrRrcSap::LogicalChannelConfig GetLogicalChannelConfig();
  void SetLogicalChannelConfig(NrRrcSap::LogicalChannelConfig conf);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_sourceCellId;
  uint16_t          m_targetCellId;
  uint32_t          m_gtpTeid;
  uint16_t          m_mmWaveRnti;
  uint16_t          m_nrRnti;
  uint8_t           m_drbid;
  NrEnbCmacSapProvider::LcInfo m_lcInfo;
  NrRrcSap::RlcConfig m_rlcConfig;
  NrRrcSap::LogicalChannelConfig m_lcConfig;
};

class NgcX2RlcSetupCompletedHeader : public Header
{
public:
  NgcX2RlcSetupCompletedHeader ();
  virtual ~NgcX2RlcSetupCompletedHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  uint16_t GetSourceCellId () const;
  void SetSourceCellId (uint16_t sourceCellId);

  uint16_t GetTargetCellId () const;
  void SetTargetCellId (uint16_t targetCellId);

  uint32_t GetGtpTeid () const;
  void SetGtpTeid (uint32_t gtpTeid);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_sourceCellId;
  uint16_t          m_targetCellId;
  uint32_t          m_gtpTeid;
};

class NgcX2McHandoverHeader : public Header
{
public:
  NgcX2McHandoverHeader ();
  virtual ~NgcX2McHandoverHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  uint16_t GetTargetCellId () const;
  void SetTargetCellId (uint16_t targetCellId);

  uint16_t GetSecondMmWaveCellId() const;
  void SetSecondMmWaveCellId(uint16_t); //sjkang

  uint16_t GetOldCellId () const;
  void SetOldCellId (uint16_t oldCellId);

  uint64_t GetImsi () const;
  void SetImsi (uint64_t imsi);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_targetCellId;
  uint16_t				m_secondMmWaveCellId; //sjkang
  uint16_t          m_oldCellId;
  uint64_t          m_imsi;
};

class NgcX2SecondaryCellHandoverCompletedHeader : public Header
{
public:
  NgcX2SecondaryCellHandoverCompletedHeader ();
  virtual ~NgcX2SecondaryCellHandoverCompletedHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  uint16_t GetMmWaveRnti () const;
  void SetMmWaveRnti (uint16_t mmWaveRnti);

  uint16_t GetOldEnbUeX2apId () const;
  void SetOldEnbUeX2apId (uint16_t oldEnbUeX2apId);

  uint64_t GetImsi () const;
  void SetImsi (uint64_t imsi);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;
  void SetisMc (bool) ;
  bool GetisMc() const;
  void SetisMc_2 (bool) ;
   bool GetisMc_2() const;
private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_mmWaveRnti;
  uint16_t          m_oldEnbUeX2apId;
  uint64_t          m_imsi;
  bool					m_isMc;
  bool 				m_isMc_2; //sjkang0710
};
class NgcX2DuplicationRlcBufferHeader:public Header //sjkang
{
public:
	NgcX2DuplicationRlcBufferHeader();
	virtual ~NgcX2DuplicationRlcBufferHeader();
	static TypeId GetTypeId (void);
	virtual TypeId GetInstanceTypeId (void) const;
	virtual uint32_t GetSerializedSize (void) const;
	virtual void Serialize (Buffer::Iterator start) const;
	virtual uint32_t Deserialize (Buffer::Iterator start);
	virtual void Print (std::ostream &os) const;

	  uint16_t GetImsi () const;
	  void SetImsi (uint16_t imsi);
	  uint16_t GetTargetCellID ()const;
	  void SetTargetCellID(uint16_t); //sjkang
	  uint16_t GetCellIDForBufferForwarding() const;
	  void SetCellIDForBufferForwarding(uint16_t);
	  bool GetOption() const;
	  void SetOption(bool); //sjkang
	  uint32_t GetLengthOfIes () const;
	  uint32_t GetNumberOfIes () const;
private:
	  uint32_t          m_numberOfIes;
	  uint32_t          m_headerLength;

	  uint16_t m_imsi;
	  uint16_t m_targetCellID;
	  uint16_t m_cellIDforBufferForwarding;
	  bool 	m_option;

};
class NgcX2NotifyCoordinatorHandoverFailedHeader : public Header
{
public:
  NgcX2NotifyCoordinatorHandoverFailedHeader ();
  virtual ~NgcX2NotifyCoordinatorHandoverFailedHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  uint16_t GetTargetCellId () const;
  void SetTargetCellId (uint16_t targetCellId);

  uint16_t GetSourceCellId () const;
  void SetSourceCellId (uint16_t oldCellId);

  uint64_t GetImsi () const;
  void SetImsi (uint64_t imsi);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_targetCellId;
  uint16_t          m_sourceCellId;
  uint64_t          m_imsi;
};

class NgcX2HandoverRequestAckHeader : public Header
{
public:
  NgcX2HandoverRequestAckHeader ();
  virtual ~NgcX2HandoverRequestAckHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint16_t GetOldEnbUeX2apId () const;
  void SetOldEnbUeX2apId (uint16_t x2apId);

  uint16_t GetNewEnbUeX2apId () const;
  void SetNewEnbUeX2apId (uint16_t x2apId);

  std::vector <NgcX2Sap::ErabAdmittedItem> GetAdmittedBearers () const;
  void SetAdmittedBearers (std::vector <NgcX2Sap::ErabAdmittedItem> bearers);

  std::vector <NgcX2Sap::ErabNotAdmittedItem> GetNotAdmittedBearers () const;
  void SetNotAdmittedBearers (std::vector <NgcX2Sap::ErabNotAdmittedItem> bearers);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_oldEnbUeX2apId;
  uint16_t          m_newEnbUeX2apId;
  std::vector <NgcX2Sap::ErabAdmittedItem>     m_erabsAdmittedList;
  std::vector <NgcX2Sap::ErabNotAdmittedItem>  m_erabsNotAdmittedList;
};

class NgcX2HandoverPreparationFailureHeader : public Header
{
public:
  NgcX2HandoverPreparationFailureHeader ();
  virtual ~NgcX2HandoverPreparationFailureHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint16_t GetOldEnbUeX2apId () const;
  void SetOldEnbUeX2apId (uint16_t x2apId);

  uint16_t GetCause () const;
  void SetCause (uint16_t cause);

  uint16_t GetCriticalityDiagnostics () const;
  void SetCriticalityDiagnostics (uint16_t criticalityDiagnostics);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_oldEnbUeX2apId;
  uint16_t          m_cause;
  uint16_t          m_criticalityDiagnostics;
};


class NgcX2SnStatusTransferHeader : public Header
{
public:
  NgcX2SnStatusTransferHeader ();
  virtual ~NgcX2SnStatusTransferHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint16_t GetOldEnbUeX2apId () const;
  void SetOldEnbUeX2apId (uint16_t x2apId);

  uint16_t GetNewEnbUeX2apId () const;
  void SetNewEnbUeX2apId (uint16_t x2apId);

  std::vector <NgcX2Sap::ErabsSubjectToStatusTransferItem> GetErabsSubjectToStatusTransferList () const;
  void SetErabsSubjectToStatusTransferList (std::vector <NgcX2Sap::ErabsSubjectToStatusTransferItem> erabs);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_oldEnbUeX2apId;
  uint16_t          m_newEnbUeX2apId;
  std::vector <NgcX2Sap::ErabsSubjectToStatusTransferItem> m_erabsSubjectToStatusTransferList;
};


class NgcX2UeContextReleaseHeader : public Header
{
public:
  NgcX2UeContextReleaseHeader ();
  virtual ~NgcX2UeContextReleaseHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint16_t GetOldEnbUeX2apId () const;
  void SetOldEnbUeX2apId (uint16_t x2apId);

  uint16_t GetNewEnbUeX2apId () const;
  void SetNewEnbUeX2apId (uint16_t x2apId);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_oldEnbUeX2apId;
  uint16_t          m_newEnbUeX2apId;
};


class NgcX2LoadInformationHeader : public Header
{
public:
  NgcX2LoadInformationHeader ();
  virtual ~NgcX2LoadInformationHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  std::vector <NgcX2Sap::CellInformationItem> GetCellInformationList () const;
  void SetCellInformationList (std::vector <NgcX2Sap::CellInformationItem> cellInformationList);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  std::vector <NgcX2Sap::CellInformationItem> m_cellInformationList;
};


class NgcX2ResourceStatusUpdateHeader : public Header
{
public:
  NgcX2ResourceStatusUpdateHeader ();
  virtual ~NgcX2ResourceStatusUpdateHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint16_t GetEnb1MeasurementId () const;
  void SetEnb1MeasurementId (uint16_t enb1MeasurementId);

  uint16_t GetEnb2MeasurementId () const;
  void SetEnb2MeasurementId (uint16_t enb2MeasurementId);

  std::vector <NgcX2Sap::CellMeasurementResultItem> GetCellMeasurementResultList () const;
  void SetCellMeasurementResultList (std::vector <NgcX2Sap::CellMeasurementResultItem> cellMeasurementResultList);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t          m_enb1MeasurementId;
  uint16_t          m_enb2MeasurementId;
  std::vector <NgcX2Sap::CellMeasurementResultItem> m_cellMeasurementResultList;
};

class NgcX2UeImsiSinrUpdateHeader : public Header
{
public:
  NgcX2UeImsiSinrUpdateHeader ();
  virtual ~NgcX2UeImsiSinrUpdateHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  std::map <uint64_t, double> GetUeImsiSinrMap () const;
  void SetUeImsiSinrMap (std::map<uint64_t, double> map);

  uint16_t GetSourceCellId () const;
  void SetSourceCellId (uint16_t sourceCellId);
  uint16_t GetSecondCellId () const; //sjkang1015
   void SetSecondCellId (uint16_t secondCellId); //sjkang1015
  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;
  void SetRnti(uint16_t) ; //sjkang
  uint16_t GetRnti() const; //sjkang
private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  // from http://beej.us/guide/bgnet/examples/ieee754.c, to convert 
  // uint64_t to double and viceversa according to IEEE754 format
  static uint64_t pack754(long double f);
  static long double unpack754(uint64_t i);

  std::map <uint64_t, double> m_map;
  uint16_t m_sourceCellId;
  uint16_t m_secondCellId; //sjkang1015
  uint16_t m_rnti; //sjkang
};

class NgcX2ConnectionSwitchHeader : public Header
{
public:
  NgcX2ConnectionSwitchHeader ();
  virtual ~NgcX2ConnectionSwitchHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint16_t GetMmWaveRnti () const;
  void SetMmWaveRnti (uint16_t rnti);

  bool GetUseMmWaveConnection () const;
  void SetUseMmWaveConnection (bool useMmWaveConnection);

  uint8_t GetDrbid () const;
  void SetDrbid (uint8_t bid);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;

  uint16_t m_mmWaveRnti;
  uint8_t m_drbid;
  bool     m_useMmWaveConnection;
};

} // namespace ns3

#endif // NGC_X2_HEADER_H
