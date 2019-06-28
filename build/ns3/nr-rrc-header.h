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
 * Author: Lluis Parcerisa <lparcerisa@cttc.cat>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#ifndef RRC_HEADER_H
#define RRC_HEADER_H

#include "ns3/header.h"

#include <bitset>
#include <string>

#include "ns3/nr-rrc-sap.h"
#include "ns3/nr-asn1-header.h"

namespace ns3 {

/**
 * This class extends NrAsn1Header functions, adding serialization/deserialization
 * of some Information elements defined in 3GPP TS 36.331
 */
class RrcNrAsn1Header : public NrAsn1Header
{
public:
  RrcNrAsn1Header ();
  int GetMessageType ();

protected:
  // Inherited from NrAsn1Header
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  uint32_t Deserialize (Buffer::Iterator bIterator) = 0;
  virtual void PreSerialize (void) const = 0;

  // Serialization functions
  void SerializeSrbToAddModList (std::list<NrRrcSap::SrbToAddMod> srbToAddModList) const;
  void SerializeDrbToAddModList (std::list<NrRrcSap::DrbToAddMod> drbToAddModList) const;
  void SerializeLogicalChannelConfig (NrRrcSap::LogicalChannelConfig logicalChannelConfig) const;
  void SerializeRadioResourceConfigDedicated (NrRrcSap::RadioResourceConfigDedicated radioResourceConfigDedicated) const;
  void SerializePhysicalConfigDedicated (NrRrcSap::PhysicalConfigDedicated physicalConfigDedicated) const;
  void SerializeSystemInformationBlockType1 (NrRrcSap::SystemInformationBlockType1 systemInformationBlockType1) const;
  void SerializeSystemInformationBlockType2 (NrRrcSap::SystemInformationBlockType2 systemInformationBlockType2) const;
  void SerializeRadioResourceConfigCommon (NrRrcSap::RadioResourceConfigCommon radioResourceConfigCommon) const;
  void SerializeRadioResourceConfigCommonSib (NrRrcSap::RadioResourceConfigCommonSib radioResourceConfigCommonSib) const;
  void SerializeMeasResults (NrRrcSap::MeasResults measResults) const;
  void SerializePlmnIdentity (uint32_t plmnId) const;
  void SerializeRachConfigCommon (NrRrcSap::RachConfigCommon rachConfigCommon) const;
  void SerializeMeasConfig (NrRrcSap::MeasConfig measConfig) const;
  void SerializeQoffsetRange (int8_t qOffsetRange) const;
  void SerializeThresholdEutra (NrRrcSap::ThresholdEutra thresholdEutra) const;
  
  // Deserialization functions
  Buffer::Iterator DeserializeDrbToAddModList (std::list<NrRrcSap::DrbToAddMod> *drbToAddModLis, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSrbToAddModList (std::list<NrRrcSap::SrbToAddMod> *srbToAddModList, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeLogicalChannelConfig (NrRrcSap::LogicalChannelConfig *logicalChannelConfig, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeRadioResourceConfigDedicated (NrRrcSap::RadioResourceConfigDedicated *radioResourceConfigDedicated, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializePhysicalConfigDedicated (NrRrcSap::PhysicalConfigDedicated *physicalConfigDedicated, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSystemInformationBlockType1 (NrRrcSap::SystemInformationBlockType1 *systemInformationBlockType1, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeSystemInformationBlockType2 (NrRrcSap::SystemInformationBlockType2 *systemInformationBlockType2, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeRadioResourceConfigCommon (NrRrcSap::RadioResourceConfigCommon *radioResourceConfigCommon, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeRadioResourceConfigCommonSib (NrRrcSap::RadioResourceConfigCommonSib *radioResourceConfigCommonSib, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeMeasResults (NrRrcSap::MeasResults *measResults, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializePlmnIdentity (uint32_t *plmnId, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeRachConfigCommon (NrRrcSap::RachConfigCommon * rachConfigCommon, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeMeasConfig (NrRrcSap::MeasConfig * measConfig, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeQoffsetRange (int8_t * qOffsetRange, Buffer::Iterator bIterator);
  Buffer::Iterator DeserializeThresholdEutra (NrRrcSap::ThresholdEutra * thresholdEutra, Buffer::Iterator bIterator);

  void Print (std::ostream &os) const;
  /**
   * This function prints RadioResourceConfigDedicated IE, for debugging purposes.
   * @param os The output stream to use (i.e. std::cout)
   * @param radioResourceConfigDedicated The information element to be printed
   */
  void Print (std::ostream &os, NrRrcSap::RadioResourceConfigDedicated radioResourceConfigDedicated) const;

  /// Stores RRC message type, according to 3GPP TS 36.331
  int m_messageType;
};


/**
 * This class only serves to discriminate which message type has been received
 * in uplink (ue to eNb) for channel DCCH
 */
class RrcUlDcchMessage : public RrcNrAsn1Header
{
public:
  RrcUlDcchMessage ();
  ~RrcUlDcchMessage ();

  // Inherited from RrcNrAsn1Header
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;
  void PreSerialize () const;

protected:
  void SerializeUlDcchMessage (int msgType) const;
  Buffer::Iterator DeserializeUlDcchMessage (Buffer::Iterator bIterator);
};

/**
 * This class only serves to discriminate which message type has been received
 * in downlink (eNb to ue) for channel DCCH
 */
class RrcDlDcchMessage : public RrcNrAsn1Header
{
public:
  RrcDlDcchMessage ();
  ~RrcDlDcchMessage ();

  // Inherited from RrcNrAsn1Header
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;
  void PreSerialize () const;

protected:
  void SerializeDlDcchMessage (int msgType) const;
  Buffer::Iterator DeserializeDlDcchMessage (Buffer::Iterator bIterator);
};

/**
 * This class only serves to discriminate which message type has been received
 * in uplink (ue to eNb) for channel CCCH
 */
class RrcUlCcchMessage : public RrcNrAsn1Header
{
public:
  RrcUlCcchMessage ();
  ~RrcUlCcchMessage ();

  // Inherited from RrcNrAsn1Header 
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;
  void PreSerialize () const;

protected:
  void SerializeUlCcchMessage (int msgType) const;
  Buffer::Iterator DeserializeUlCcchMessage (Buffer::Iterator bIterator);
};

/**
 * This class only serves to discriminate which message type has been received
 * in downlink (eNb to ue) for channel CCCH
 */
class RrcDlCcchMessage : public RrcNrAsn1Header
{
public:
  RrcDlCcchMessage ();
  ~RrcDlCcchMessage ();

  // Inherited from RrcNrAsn1Header 
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;
  void PreSerialize () const;

protected:
  void SerializeDlCcchMessage (int msgType) const;
  Buffer::Iterator DeserializeDlCcchMessage (Buffer::Iterator bIterator);
};

/**
* This class manages the serialization/deserialization of RrcConnectionRequest IE
*/
class RrcConnectionRequestHeader : public RrcUlCcchMessage
{
public:
  RrcConnectionRequestHeader ();
  ~RrcConnectionRequestHeader ();

  // Inherited from RrcNrAsn1Header 
  static TypeId GetTypeId (void);
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
   * Receives a RrcConnectionRequest IE and stores the contents into the class attributes
   * @param msg The information element to parse
   */
  void SetMessage (NrRrcSap::RrcConnectionRequest msg);

  /**
   * Returns a RrcConnectionRequest IE from the values in the class attributes
   * @return A RrcConnectionRequest, as defined in NrRrcSap
   */
  NrRrcSap::RrcConnectionRequest GetMessage () const;

  /**
   * Get AMFID attribute
   * @return m_amfid attribute
   */
  std::bitset<8> GetAmfid () const;

  /**
   * Get M-TMSI attribute
   * @return m_tmsi attribute
   */
  std::bitset<32> GetMtmsi () const;

  std::bitset<1> GetIsMc () const;

private:
  std::bitset<8> m_amfid;
  std::bitset<32> m_mTmsi;
  enum
  {
    EMERGENCY = 0, HIGHPRIORITYACCESS, MT_ACCESS,
    MO_SIGNALLING, MO_DATA, SPARE3, SPARE2, SPARE1
  } m_establishmentCause;
  std::bitset<1> m_spare;
};

class RrcConnectToMmWaveHeader : public RrcDlCcchMessage
{
public:
  RrcConnectToMmWaveHeader();
  ~RrcConnectToMmWaveHeader();

  // Inherited from RrcNrAsn1Header 
  static TypeId GetTypeId (void);
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
   //TODO doc
   */
  void SetMessage (uint16_t mmWaveId, uint16_t mmWaveId_2); //sjkang

  uint16_t GetMessage () const;
  uint16_t GetMessage_secondMmWaveCellId () const; //sjkang
private:
  std::bitset<16> m_mmWaveId;
  std::bitset<16> m_mmWaveId_2; //sjkang
};

class RrcNotifySecondaryConnectedHeader : public RrcUlDcchMessage
{
public:
  RrcNotifySecondaryConnectedHeader();
  ~RrcNotifySecondaryConnectedHeader();

  // Inherited from RrcNrAsn1Header 
  static TypeId GetTypeId (void);
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
   //TODO doc
   */
  void SetMessage (uint16_t mmWaveId, uint16_t mmWaveRnti);

  std::pair<uint16_t, uint16_t> GetMessage () const;

private:
  std::bitset<16> m_mmWaveId;
  std::bitset<16> m_mmWaveRnti;
};

class RrcConnectionSwitchHeader : public RrcDlDcchMessage
{
public:
  RrcConnectionSwitchHeader();
  ~RrcConnectionSwitchHeader();

  // Inherited from RrcNrAsn1Header 
  static TypeId GetTypeId (void);
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
   //TODO doc
   */
  void SetMessage (NrRrcSap::RrcConnectionSwitch msg);

  NrRrcSap::RrcConnectionSwitch GetMessage () const;
  uint8_t GetRrcTransactionIdentifier () const;

private:
  mutable NrRrcSap::RrcConnectionSwitch m_msg;
};

/**
* This class manages the serialization/deserialization of RrcConnectionSetup IE
*/
class RrcConnectionSetupHeader : public RrcDlCcchMessage
{
public:
  RrcConnectionSetupHeader ();
  ~RrcConnectionSetupHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionSetup IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */
  void SetMessage (NrRrcSap::RrcConnectionSetup msg);

  /**
  * Returns a RrcConnectionSetup IE from the values in the class attributes
  * @return A RrcConnectionSetup, as defined in NrRrcSap
  */
  NrRrcSap::RrcConnectionSetup GetMessage () const;

  /**
  * Getter for m_rrcTransactionIdentifier
  * @return m_rrcTransactionIdentifier
  */
  uint8_t GetRrcTransactionIdentifier () const;

  /**
  * Getter for m_radioResourceConfigDedicated
  * @return m_radioResourceConfigDedicated
  */
  NrRrcSap::RadioResourceConfigDedicated GetRadioResourceConfigDedicated () const; 

  /**
  * Gets m_radioResourceConfigDedicated.havePhysicalConfigDedicated
  * @return m_radioResourceConfigDedicated.havePhysicalConfigDedicated
  */
  bool HavePhysicalConfigDedicated () const;

  /**
  * Gets m_radioResourceConfigDedicated.physicalConfigDedicated
  * @return m_radioResourceConfigDedicated.physicalConfigDedicated
  */
  NrRrcSap::PhysicalConfigDedicated GetPhysicalConfigDedicated () const;

  /**
  * Gets m_radioResourceConfigDedicated.srbToAddModList
  * @return m_radioResourceConfigDedicated.srbToAddModList
  */
  std::list<NrRrcSap::SrbToAddMod> GetSrbToAddModList () const;

  /**
  * Gets m_radioResourceConfigDedicated.drbToAddModList
  * @return m_radioResourceConfigDedicated.drbToAddModList
  */
  std::list<NrRrcSap::DrbToAddMod> GetDrbToAddModList () const;

  /**
  * Gets m_radioResourceConfigDedicated.drbToReleaseList
  * @return m_radioResourceConfigDedicated.drbToReleaseList
  */
  std::list<uint8_t> GetDrbToReleaseList () const;

private:
  uint8_t m_rrcTransactionIdentifier;
  mutable NrRrcSap::RadioResourceConfigDedicated m_radioResourceConfigDedicated;
};

/**
* This class manages the serialization/deserialization of RrcConnectionSetupComplete IE
*/
class RrcConnectionSetupCompleteHeader : public RrcUlDcchMessage
{
public:
  RrcConnectionSetupCompleteHeader ();
  ~RrcConnectionSetupCompleteHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionSetupCompleted IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */
  void SetMessage (NrRrcSap::RrcConnectionSetupCompleted msg);

  /**
  * Returns a RrcConnectionSetupCompleted IE from the values in the class attributes
  * @return A RrcConnectionSetupCompleted, as defined in NrRrcSap
  */
  NrRrcSap::RrcConnectionSetupCompleted GetMessage () const;

  /**
  * Getter for m_rrcTransactionIdentifier
  * @return m_rrcTransactionIdentifier
  */
  uint8_t GetRrcTransactionIdentifier () const;

private:
  uint8_t m_rrcTransactionIdentifier;

};

/**
* This class manages the serialization/deserialization of RrcConnectionSetupComplete IE
*/
class RrcConnectionReconfigurationCompleteHeader : public RrcUlDcchMessage
{
public:
  RrcConnectionReconfigurationCompleteHeader ();
  ~RrcConnectionReconfigurationCompleteHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReconfigurationCompleted IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */
  void SetMessage (NrRrcSap::RrcConnectionReconfigurationCompleted msg);

  /**
  * Returns a RrcConnectionReconfigurationCompleted IE from the values in the class attributes
  * @return A RrcConnectionReconfigurationCompleted, as defined in NrRrcSap
  */
  NrRrcSap::RrcConnectionReconfigurationCompleted GetMessage () const;

  /**
  * Getter for m_rrcTransactionIdentifier
  * @return m_rrcTransactionIdentifier
  */ 
  uint8_t GetRrcTransactionIdentifier () const;

private:
  uint8_t m_rrcTransactionIdentifier;

};

/**
* This class manages the serialization/deserialization of RrcConnectionReconfiguration IE
*/
class RrcConnectionReconfigurationHeader : public RrcDlDcchMessage
{
public:
  RrcConnectionReconfigurationHeader ();
  ~RrcConnectionReconfigurationHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReconfiguration IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (NrRrcSap::RrcConnectionReconfiguration msg);

  /**
  * Returns a RrcConnectionReconfiguration IE from the values in the class attributes
  * @return A RrcConnectionReconfiguration, as defined in NrRrcSap
  */
  NrRrcSap::RrcConnectionReconfiguration GetMessage () const; 

  /**
  * Getter for m_haveMeasConfig
  * @return m_haveMeasConfig
  */
  bool GetHaveMeasConfig ();

  /**
  * Getter for m_measConfig
  * @return m_measConfig
  */
  NrRrcSap::MeasConfig GetMeasConfig ();

  /**
  * Getter for m_haveMobilityControlInfo
  * @return m_haveMobilityControlInfo
  */
  bool GetHaveMobilityControlInfo ();

  /**
  * Getter for m_mobilityControlInfo
  * @return m_mobilityControlInfo
  */
  NrRrcSap::MobilityControlInfo GetMobilityControlInfo ();

  /**
  * Getter for m_haveRadioResourceConfigDedicated
  * @return m_haveRadioResourceConfigDedicated
  */
  bool GetHaveRadioResourceConfigDedicated ();

  /**
  * Getter for m_radioResourceConfigDedicated
  * @return m_radioResourceConfigDedicated
  */
  NrRrcSap::RadioResourceConfigDedicated GetRadioResourceConfigDedicated ();

  /**
  * Getter for m_rrcTransactionIdentifier
  * @return m_rrcTransactionIdentifier
  */
  uint8_t GetRrcTransactionIdentifier () const;

  /**
  * Getter for m_radioResourceConfigDedicated
  * @return m_radioResourceConfigDedicated
  */
  NrRrcSap::RadioResourceConfigDedicated GetRadioResourceConfigDedicated () const; 

  /**
  * Gets m_radioResourceConfigDedicated.havePhysicalConfigDedicated
  * @return m_radioResourceConfigDedicated.havePhysicalConfigDedicated
  */
  bool HavePhysicalConfigDedicated () const;

  /**
  * Gets m_radioResourceConfigDedicated.physicalConfigDedicated
  * @return m_radioResourceConfigDedicated.physicalConfigDedicated
  */
  NrRrcSap::PhysicalConfigDedicated GetPhysicalConfigDedicated () const;

  /**
  * Gets m_radioResourceConfigDedicated.srbToAddModList
  * @return m_radioResourceConfigDedicated.srbToAddModList
  */
  std::list<NrRrcSap::SrbToAddMod> GetSrbToAddModList () const;

  /**
  * Gets m_radioResourceConfigDedicated.drbToAddModList
  * @return m_radioResourceConfigDedicated.drbToAddModList
  */
  std::list<NrRrcSap::DrbToAddMod> GetDrbToAddModList () const;

  /**
  * Gets m_radioResourceConfigDedicated.drbToReleaseList
  * @return m_radioResourceConfigDedicated.drbToReleaseList
  */
  std::list<uint8_t> GetDrbToReleaseList () const;

private:
  uint8_t m_rrcTransactionIdentifier;
  bool m_haveMeasConfig;
  NrRrcSap::MeasConfig m_measConfig;
  bool m_haveMobilityControlInfo;
  NrRrcSap::MobilityControlInfo m_mobilityControlInfo;
  bool m_haveRadioResourceConfigDedicated;
  NrRrcSap::RadioResourceConfigDedicated m_radioResourceConfigDedicated;
  uint8_t m_handoverCase; //sjkang
};

/**
* This class manages the serialization/deserialization of HandoverPreparationInfo IE
*/
class HandoverPreparationInfoHeader : public RrcNrAsn1Header
{
public:
  HandoverPreparationInfoHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a HandoverPreparationInfo IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (NrRrcSap::HandoverPreparationInfo msg);

  /**
  * Returns a HandoverPreparationInfo IE from the values in the class attributes
  * @return A HandoverPreparationInfo, as defined in NrRrcSap
  */
  NrRrcSap::HandoverPreparationInfo GetMessage () const;

  /**
  * Getter for m_asConfig
  * @return m_asConfig
  */ 
  NrRrcSap::AsConfig GetAsConfig () const;

private:
  NrRrcSap::AsConfig m_asConfig;
};

// jhlim
class RrcIdentityRequestHeader : public RrcDlDcchMessage
{
public:
  RrcIdentityRequestHeader ();
  ~RrcIdentityRequestHeader ();
/*
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;
*/

  void SetMessage (NrRrcSap::RrcIdentityRequest msg);
//  NrRrcSap::RrcIdentityRequest GetMessage () const;

};
class RrcIdentityResponseHeader : public RrcDlDcchMessage
{
public:
  RrcIdentityResponseHeader ();
  ~RrcIdentityResponseHeader ();
/*
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;
*/

  void SetMessage (NrRrcSap::RrcIdentityResponse msg);
//  NrRrcSap::RrcIdentityRequest GetMessage () const;

};
class RrcRegistrationAcceptHeader : public RrcDlDcchMessage
{
public:
  RrcRegistrationAcceptHeader ();
  ~RrcRegistrationAcceptHeader ();

  void SetMessage (NrRrcSap::RrcRegistrationAccept msg);

};
class RrcRegistrationCompleteHeader : public RrcDlDcchMessage
{
public:
  RrcRegistrationCompleteHeader ();
  ~RrcRegistrationCompleteHeader ();

  void SetMessage (NrRrcSap::RrcRegistrationComplete msg);

};

/**
* This class manages the serialization/deserialization of RRCConnectionReestablishmentRequest IE
*/
class RrcConnectionReestablishmentRequestHeader : public RrcUlCcchMessage
{
public:
  RrcConnectionReestablishmentRequestHeader ();
  ~RrcConnectionReestablishmentRequestHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReestablishmentRequest IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (NrRrcSap::RrcConnectionReestablishmentRequest msg);

  /**
  * Returns a RrcConnectionReestablishmentRequest IE from the values in the class attributes
  * @return A RrcConnectionReestablishmentRequest, as defined in NrRrcSap
  */
  NrRrcSap::RrcConnectionReestablishmentRequest GetMessage () const;

  /**
  * Getter for m_ueIdentity
  * @return m_ueIdentity
  */
  NrRrcSap::ReestabUeIdentity GetUeIdentity () const;

  /**
  * Getter for m_reestablishmentCause
  * @return m_reestablishmentCause
  */
  NrRrcSap::ReestablishmentCause GetReestablishmentCause () const;

private:
  NrRrcSap::ReestabUeIdentity m_ueIdentity;
  NrRrcSap::ReestablishmentCause m_reestablishmentCause;
};

/**
* This class manages the serialization/deserialization of RrcConnectionReestablishment IE
*/
class RrcConnectionReestablishmentHeader : public RrcDlCcchMessage
{
public:
  RrcConnectionReestablishmentHeader ();
  ~RrcConnectionReestablishmentHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReestablishment IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (NrRrcSap::RrcConnectionReestablishment msg);

  /**
  * Returns a RrcConnectionReestablishment IE from the values in the class attributes
  * @return A RrcConnectionReestablishment, as defined in NrRrcSap
  */
  NrRrcSap::RrcConnectionReestablishment GetMessage () const;

  /**
  * Getter for m_rrcTransactionIdentifier attribute
  * @return m_rrcTransactionIdentifier
  */
  uint8_t GetRrcTransactionIdentifier () const;

  /**
  * Getter for m_radioResourceConfigDedicated attribute
  * @return m_radioResourceConfigDedicated
  */
  NrRrcSap::RadioResourceConfigDedicated GetRadioResourceConfigDedicated () const;

private:
  uint8_t m_rrcTransactionIdentifier;
  NrRrcSap::RadioResourceConfigDedicated m_radioResourceConfigDedicated;
};

/**
* This class manages the serialization/deserialization of RrcConnectionReestablishmentComplete IE
*/
class RrcConnectionReestablishmentCompleteHeader : public RrcUlDcchMessage
{
public:
  RrcConnectionReestablishmentCompleteHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReestablishmentComplete IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (NrRrcSap::RrcConnectionReestablishmentComplete msg);

  /**
  * Returns a RrcConnectionReestablishmentComplete IE from the values in the class attributes
  * @return A RrcConnectionReestablishmentComplete, as defined in NrRrcSap
  */
  NrRrcSap::RrcConnectionReestablishmentComplete GetMessage () const;

  /**
  * Getter for m_rrcTransactionIdentifier attribute
  * @return m_rrcTransactionIdentifier
  */
  uint8_t GetRrcTransactionIdentifier () const;

private:
  uint8_t m_rrcTransactionIdentifier;
};

/**
* This class manages the serialization/deserialization of RrcConnectionReestablishmentReject IE
*/
class RrcConnectionReestablishmentRejectHeader : public RrcDlCcchMessage
{
public:
  RrcConnectionReestablishmentRejectHeader ();
  ~RrcConnectionReestablishmentRejectHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReestablishmentReject IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (NrRrcSap::RrcConnectionReestablishmentReject msg);

  /**
  * Returns a RrcConnectionReestablishmentReject IE from the values in the class attributes
  * @return A RrcConnectionReestablishmentReject, as defined in NrRrcSap
  */
  NrRrcSap::RrcConnectionReestablishmentReject GetMessage () const;

private:
  NrRrcSap::RrcConnectionReestablishmentReject m_rrcConnectionReestablishmentReject;
};

/**
* This class manages the serialization/deserialization of RrcConnectionRelease IE
*/
class RrcConnectionReleaseHeader : public RrcDlDcchMessage
{
public:
  RrcConnectionReleaseHeader ();
  ~RrcConnectionReleaseHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionRelease IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (NrRrcSap::RrcConnectionRelease msg);

  /**
  * Returns a RrcConnectionRelease IE from the values in the class attributes
  * @return A RrcConnectionRelease, as defined in NrRrcSap
  */
  NrRrcSap::RrcConnectionRelease GetMessage () const;

private:
  NrRrcSap::RrcConnectionRelease m_rrcConnectionRelease;
};

/**
* This class manages the serialization/deserialization of RrcConnectionReject IE
*/
class RrcConnectionRejectHeader : public RrcDlCcchMessage
{
public:
  RrcConnectionRejectHeader ();
  ~RrcConnectionRejectHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReject IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (NrRrcSap::RrcConnectionReject msg);

  /**
  * Returns a RrcConnectionReject IE from the values in the class attributes
  * @return A RrcConnectionReject, as defined in NrRrcSap
  */
  NrRrcSap::RrcConnectionReject GetMessage () const;

private:
  NrRrcSap::RrcConnectionReject m_rrcConnectionReject;
};

/**
* This class manages the serialization/deserialization of MeasurementReport IE
*/
class MeasurementReportHeader : public RrcUlDcchMessage
{
public:
  MeasurementReportHeader ();
  ~MeasurementReportHeader ();

  // Inherited from RrcNrAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a MeasurementReport IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */
  void SetMessage (NrRrcSap::MeasurementReport msg);

  /**
  * Returns a MeasurementReport IE from the values in the class attributes
  * @return A MeasurementReport, as defined in NrRrcSap
  */
  NrRrcSap::MeasurementReport GetMessage () const;

private:
  NrRrcSap::MeasurementReport m_measurementReport;

};

} // namespace ns3

#endif // RRC_HEADER_H

