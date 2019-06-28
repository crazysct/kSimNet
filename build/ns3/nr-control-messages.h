/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 */

#ifndef NR_CONTROL_MESSAGES_H
#define NR_CONTROL_MESSAGES_H

#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>
#include <ns3/nr-ff-mac-common.h>
#include <ns3/nr-rrc-sap.h>
#include <list>

namespace ns3 {

class NrNetDevice;


/**
 * \ingroup nr
 *
 * The NrControlMessage provides a basic implementations for
 * control messages (such as PDCCH allocation map, CQI feedbacks)
 * that are exchanged among eNodeB and UEs.
 */
class NrControlMessage : public SimpleRefCount<NrControlMessage>
{
public:
  /**
   * The type of the message
   * NOTE: The messages sent by UE are filtered by the
   *  NrEnbPhy::ReceiveNrControlMessageList in order to remove the ones 
   *  that has been already handoff by the eNB for avoiding propagation of
   *  spurious messages. When new messaged have to been added, consider to
   *  update the switch statement implementing teh filtering.
   */
  enum MessageType
  {
    DL_DCI, UL_DCI, // Downlink/Uplink Data Control Indicator
    DL_CQI, UL_CQI, // Downlink/Uplink Channel Quality Indicator
    BSR, // Buffer Status Report
    DL_HARQ, // UL HARQ feedback
    RACH_PREAMBLE, // Random Access Preamble
    RAR, // Random Access Response
    MIB, // Master Information Block
    SIB1, // System Information Block Type 1
  };

  NrControlMessage (void);
  virtual ~NrControlMessage (void);

  /**
   * \brief Set the type of the message
   * \param type the type of the message
   */
  void SetMessageType (MessageType type);
  /**
   * \brief Get the type of the message
   * \return the type of the message
   */
  MessageType GetMessageType (void);

private:
  MessageType m_type;
};


// -----------------------------------------------------------------------

/**
 * \ingroup nr
 * The Downlink Data Control Indicator messages defines the RB allocation for the
 * users in the downlink
 */
class DlDciNrControlMessage : public NrControlMessage
{
public:
  DlDciNrControlMessage (void);
  virtual ~DlDciNrControlMessage (void);

  /**
  * \brief add a DCI into the message
  * \param dci the dci
  */
  void SetDci (NrDlDciListElement_s dci);

  /**
  * \brief Get dic informations
  * \return dci messages
  */
  NrDlDciListElement_s GetDci (void);

private:
  NrDlDciListElement_s m_dci;
};


// ---------------------------------------------------------------------------

/**
 * \ingroup nr
 * The Uplink Data Control Indicator messages defines the RB allocation for the
 * users in the uplink
 */
class UlDciNrControlMessage : public NrControlMessage
{
public:
  UlDciNrControlMessage (void);
  virtual ~UlDciNrControlMessage (void);

  /**
  * \brief add a DCI into the message
  * \param dci the dci
  */
  void SetDci (NrUlDciListElement_s dci);

  /**
  * \brief Get dic informations
  * \return dci messages
  */
  NrUlDciListElement_s GetDci (void);

private:
  NrUlDciListElement_s m_dci;
};


// ---------------------------------------------------------------------------

/**
 * \ingroup nr
 * The downlink CqiNrControlMessage defines an ideal list of
 * feedback about the channel quality sent by the UE to the eNodeB.
 */
class DlCqiNrControlMessage : public NrControlMessage
{
public:
  DlCqiNrControlMessage (void);
  virtual ~DlCqiNrControlMessage (void);

  /**
  * \brief add a DL-CQI feedback record into the message.
  * \param dlcqi the DL cqi feedback
  */
  void SetDlCqi (NrCqiListElement_s dlcqi);

  /**
  * \brief Get DL cqi informations
  * \return dlcqi messages
  */
  NrCqiListElement_s GetDlCqi (void);

private:
  NrCqiListElement_s m_dlCqi;
};


// ---------------------------------------------------------------------------

/**
 * \ingroup nr
 * The uplink BsrNrControlMessage defines the specific
 * extension of the CE element for reporting the buffer status report
 */
class BsrNrControlMessage : public NrControlMessage
{
public:
  BsrNrControlMessage (void);
  virtual ~BsrNrControlMessage (void);

  /**
  * \brief add a BSR feedback record into the message.
  * \param bsr the BSR feedback
  */
  void SetBsr (NrMacCeListElement_s bsr);

  /**
  * \brief Get BSR informations
  * \return BSR message
  */
  NrMacCeListElement_s GetBsr (void);

private:
  NrMacCeListElement_s m_bsr;

};


// ---------------------------------------------------------------------------

/**
 * \ingroup nr
 * The downlink DlHarqFeedbackNrControlMessage defines the specific
 * messages for transmitting the DL HARQ feedback through PUCCH
 */
class DlHarqFeedbackNrControlMessage : public NrControlMessage
{
public:
  DlHarqFeedbackNrControlMessage (void);
  virtual ~DlHarqFeedbackNrControlMessage (void);

  /**
  * \brief add a DL HARQ feedback record into the message.
  * \param m the DL HARQ feedback
  */
  void SetDlHarqFeedback (NrDlInfoListElement_s m);

  /**
  * \brief Get DL HARQ informations
  * \return DL HARQ message
  */
  NrDlInfoListElement_s GetDlHarqFeedback (void);

private:
  NrDlInfoListElement_s m_dlInfoListElement;

};


// ---------------------------------------------------------------------------

/**
 * \ingroup nr
 *
 * abstract model for the Random Access Preamble
 */
class RachPreambleNrControlMessage : public NrControlMessage
{
public:
  RachPreambleNrControlMessage (void);
  
  /** 
   * Set the Random Access Preamble Identifier (RAPID), see 3GPP TS 36.321 6.2.2
   *
   * \param rapid the RAPID
   */
  void SetRapId (uint32_t rapid);
  
  /** 
   * 
   * \return the RAPID
   */
  uint32_t GetRapId () const;

private:
  uint32_t m_rapId;

};


// ---------------------------------------------------------------------------

/**
 * \ingroup nr
 *
 * abstract model for the MAC Random Access Response message
 */
class RarNrControlMessage : public NrControlMessage
{
public:
  RarNrControlMessage (void);

  /** 
   * 
   * \param raRnti the RA-RNTI, see 3GPP TS 36.321 5.1.4
   */
  void SetRaRnti (uint16_t raRnti);

  /** 
   * 
   * \return  the RA-RNTI, see 3GPP TS 36.321 5.1.4
   */
  uint16_t GetRaRnti () const;

  /**
   * a MAC RAR and the corresponding RAPID subheader 
   * 
   */
  struct Rar
  {
    uint8_t rapId;
    NrBuildRarListElement_s rarPayload;
  };

  /** 
   * add a RAR to the MAC PDU, see 3GPP TS 36.321 6.2.3
   * 
   * \param rar the rar
   */
  void AddRar (Rar rar);

  /** 
   * 
   * \return a const iterator to the beginning of the RAR list
   */
  std::list<Rar>::const_iterator RarListBegin () const;
  
  /** 
   * 
   * \return a const iterator to the end of the RAR list
   */
  std::list<Rar>::const_iterator RarListEnd () const;

private:
  std::list<Rar> m_rarList;
  uint16_t m_raRnti;

};


// ---------------------------------------------------------------------------

/**
 * \ingroup nr
 * \brief Abstract model for broadcasting the Master Information Block (MIB)
 *        within the control channel (BCCH).
 *
 * MIB is transmitted by eNodeB RRC and received by UE RRC at every radio frame,
 * i.e., every 10 milliseconds.
 *
 * \sa NrEnbRrc::ConfigureCell, NrEnbPhy::StartFrame,
 *     NrUeRrc::DoRecvMasterInformationBlock
 */
class MibNrControlMessage : public NrControlMessage
{
public:
  /**
   * \brief Create a new instance of MIB control message.
   */
  MibNrControlMessage (void);

  /**
   * \brief Replace the MIB content of this control message.
   * \param mib the desired MIB content
   */
  void SetMib (NrRrcSap::MasterInformationBlock mib);

  /**
   * \brief Retrieve the MIB content from this control message.
   * \return the current MIB content that this control message holds
   */
  NrRrcSap::MasterInformationBlock GetMib () const;

private:
  NrRrcSap::MasterInformationBlock m_mib;

}; // end of class MibNrControlMessage


// ---------------------------------------------------------------------------

/**
 * \ingroup nr
 * \brief Abstract model for broadcasting the System Information Block Type 1
 *        (SIB1) within the control channel (BCCH).
 *
 * SIB1 is transmitted by eNodeB RRC and received by UE RRC at the 6th subframe
 * of every odd-numbered radio frame, i.e., every 20 milliseconds.
 *
 * \sa NrEnbRrc::SetSystemInformationBlockType1, NrEnbPhy::StartSubFrame,
 *     NrUeRrc::DoRecvSystemInformationBlockType1
 */
class Sib1NrControlMessage : public NrControlMessage
{
public:
  /**
   * \brief Create a new instance of SIB1 control message.
   */
  Sib1NrControlMessage (void);

  /**
   * \brief Replace the SIB1 content of this control message.
   * \param sib1 the desired SIB1 content
   */
  void SetSib1 (NrRrcSap::SystemInformationBlockType1 sib1);

  /**
   * \brief Retrieve the SIB1 content from this control message.
   * \return the current SIB1 content that this control message holds
   */
  NrRrcSap::SystemInformationBlockType1 GetSib1 () const;

private:
  NrRrcSap::SystemInformationBlockType1 m_sib1;

}; // end of class Sib1NrControlMessage


} // namespace ns3

#endif  // NR_CONTROL_MESSAGES_H
