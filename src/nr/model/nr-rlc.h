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

#ifndef NR_RLC_H
#define NR_RLC_H

#include <ns3/simple-ref-count.h>
#include <ns3/packet.h>
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/nstime.h"
#include <ns3/ngc-x2-sap.h>

#include "ns3/object.h"

#include "ns3/nr-rlc-sap.h"
#include "ns3/nr-mac-sap.h"
#include <fstream>//sjkang

namespace ns3 {


// class NrRlcSapProvider;
// class NrRlcSapUser;
// 
// class NrMacSapProvider;
// class NrMacSapUser;

class NrRlc;

class NrRlcSpecificNrMacSapUser : public NrMacSapUser
{

public:
  NrRlcSpecificNrMacSapUser (NrRlc* rlc);

  // Interface implemented from NrMacSapUser
  virtual void NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId);
  virtual void NotifyHarqDeliveryFailure ();
  virtual void NotifyHarqDeliveryFailure (uint8_t harqId);
  virtual void ReceivePdu (Ptr<Packet> p);

private:
  NrRlcSpecificNrMacSapUser ();
  NrRlc* m_rlc;
};


/**
 * This abstract base class defines the API to interact with the Radio Link Control
 * (NR_RLC) in NR, see 3GPP TS 36.322
 *
 */
class NrRlc : public Object 
{
  friend class NrRlcSpecificNrMacSapUser;
  friend class NgcX2RlcSpecificUser<NrRlc>;
  friend class NrRlcSpecificNrRlcSapProvider<NrRlc>;
public:
  NrRlc ();
  virtual ~NrRlc ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   *
   *
   * \param rnti
   */
  void SetRnti (uint16_t rnti);
  void SetDrbId(uint8_t drbId); //sjkang1115
  void SetStreamForQueueStatistics(std::ofstream * stream); //sjkang1116

  /**
   *
   *
   * \param lcId
   */
  void SetLcId (uint8_t lcId);

  /**
   *
   *
   * \param s the RLC SAP user to be used by this NR_RLC
   */
  void SetNrRlcSapUser (NrRlcSapUser * s);
  void SetNrRlcAssistantSapUser(NrRlcSapUser *s); //sjkang0104

  /**
   *
   *
   * \return the RLC SAP Provider interface offered to the PDCP by this NR_RLC
   */
  NrRlcSapProvider* GetNrRlcSapProvider ();

  /**
   * Set the param needed for X2 tunneling
   * \param the UeDataParams defined in RRC
   */
  void SetUeDataParams(NgcX2Sap::UeDataParams params);

  /**
   * \param s the NgcX2Rlc Provider to the Ngc X2 interface
   */
  void SetNgcX2RlcProvider (NgcX2RlcProvider * s);

  /**
   * \return the NgcX2Rlc User, given to X2 to access Rlc SendMcPdcpPdu method
   */
  NgcX2RlcUser* GetNgcX2RlcUser ();

  /**
   *
   *
   * \param s the MAC SAP Provider to be used by this NR_RLC
   */
  void SetNrMacSapProvider (NrMacSapProvider * s);

  /**
   *
   *
   * \return the MAC SAP User interface offered to the MAC by this NR_RLC
   */
  NrMacSapUser* GetNrMacSapUser ();


  /**
   * TracedCallback signature for NotifyTxOpportunity events.
   *
   * \param [in] rnti C-RNTI scheduled.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] bytes The number of bytes to transmit
   */
  typedef void (* NotifyTxTracedCallback)
    (uint16_t rnti, uint8_t lcid, uint32_t bytes);

  /**
   * TracedCallback signature for
   *
   * \param [in] rnti C-RNTI scheduled.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] bytes The packet size.
   * \param [in] delay Delay since sender timestamp, in ns.
   */
  typedef void (* ReceiveTracedCallback)
    (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay);

  /**
   * TracedCallback signature for
   *
   * \param [in] rnti C-RNTI scheduled.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] bytes The packet size.
   * \param [in] the number of RLC AM retransmissions for that packet
   */
  typedef void (* RetransmissionCountCallback)
    (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint32_t numRetx);

  /// \todo MRE What is the sense to duplicate all the interfaces here???
  // NB to avoid the use of multiple inheritance
  virtual void CalculatePathThroughput(std::ofstream *stream)=0; //sjkang

protected:
  // Interface forwarded by NrRlcSapProvider
  virtual void DoTransmitPdcpPdu (Ptr<Packet> p) = 0;

  NrRlcSapUser* m_rlcSapUser;
  NrRlcSapUser* m_rlc_Assistant_User; //sjkang0104
  NrRlcSapProvider* m_rlcSapProvider;

  // Interface forwarded by NrMacSapUser
  virtual void DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId) = 0;
  virtual void DoNotifyHarqDeliveryFailure () = 0;
  virtual void DoNotifyHarqDeliveryFailure (uint8_t harqId);
  virtual void DoReceivePdu (Ptr<Packet> p) = 0;

  virtual void DoSendMcPdcpSdu(NgcX2Sap::UeDataParams params) = 0;

  virtual void DoRequestAssistantInfo();//sjkang

  NrMacSapUser* m_macSapUser;
  NrMacSapProvider* m_macSapProvider;

  uint16_t m_rnti;
  uint8_t m_lcid;
  uint8_t m_drbId;//sjkang1115
  std::ofstream *measuringQusizeQueueDelayStream; //sjkang1116
 // uint16_t m_rnti; //sjkang1115
  /**
   * Used to inform of a PDU delivery to the MAC SAP provider
   */
  TracedCallback<uint16_t, uint8_t, uint32_t> m_txPdu;
  /**
   * Used to inform of a PDU reception from the MAC SAP user
   */
  TracedCallback<uint16_t, uint8_t, uint32_t, uint64_t> m_rxPdu;

  TracedCallback<uint16_t, uint8_t, uint32_t, uint32_t> m_txCompletedCallback; // callback used to broadcast the number of retx for each RLC packet

  // MC functionalities
  // UeDataParams needed to forward data to MmWave
  NgcX2Sap::UeDataParams m_ueDataParams;
  bool isMc;
  NgcX2RlcProvider* m_ngcX2RlcProvider;
  NgcX2RlcUser* m_ngcX2RlcUser;

};



/**
 * NR_RLC Saturation Mode (SM): simulation-specific mode used for
 * experiments that do not need to consider the layers above the NR_RLC.
 * The NR_RLC SM, unlike the standard NR_RLC modes, it does not provide
 * data delivery services to upper layers; rather, it just generates a
 * new NR_RLC PDU whenever the MAC notifies a transmission opportunity.
 *
 */
class NrRlcSm : public NrRlc
{
public:
  NrRlcSm ();
  virtual ~NrRlcSm ();
  static TypeId GetTypeId (void);
  virtual void DoInitialize ();
  virtual void DoDispose ();

  virtual void DoTransmitPdcpPdu (Ptr<Packet> p);
  virtual void DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId);
  virtual void DoNotifyHarqDeliveryFailure ();
  virtual void DoReceivePdu (Ptr<Packet> p);
  virtual void DoSendMcPdcpSdu (NgcX2Sap::UeDataParams params);
  virtual void CalculatePathThroughput(std::ofstream *stream); //sjkang
  virtual void DoRequestAssistantInfo();//sjkang


private:
  void ReportBufferStatus ();

};




// /**
//  * Implements NR_RLC Transparent Mode (TM), see  3GPP TS 36.322
//  *
//  */
// class NrRlcTm : public NrRlc
// {
// public:
//   virtual ~NrRlcTm ();

// };


// /**
//  * Implements NR_RLC Unacknowledged Mode (UM), see  3GPP TS 36.322
//  *
//  */
// class NrRlcUm : public NrRlc
// {
// public:
//   virtual ~NrRlcUm ();

// };

// /**
//  * Implements NR_RLC Acknowledged Mode (AM), see  3GPP TS 36.322
//  *
//  */

// class NrRlcAm : public NrRlc
// {
// public:
//   virtual ~NrRlcAm ();
// };





} // namespace ns3

#endif // NR_RLC_H
