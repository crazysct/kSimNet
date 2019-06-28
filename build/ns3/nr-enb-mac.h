/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Marco Miozzo  <marco.miozzo@cttc.es>
 *         Nicola Baldo  <nbaldo@cttc.es>
 */

#ifndef NR_ENB_MAC_H
#define NR_ENB_MAC_H


#include <map>
#include <vector>
#include <ns3/nr-common.h>
#include <ns3/nr-mac-sap.h>
#include <ns3/nr-enb-cmac-sap.h>
#include <ns3/nr-ff-mac-csched-sap.h>
#include <ns3/nr-ff-mac-sched-sap.h>
#include <ns3/nr-enb-phy-sap.h>
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include <ns3/packet.h>
#include <ns3/packet-burst.h>

namespace ns3 {

class DlCqiNrControlMessage;
class UlCqiNrControlMessage;
class PdcchMapNrControlMessage;

typedef std::vector <std::vector < Ptr<PacketBurst> > > DlHarqProcessesBuffer_t;

/**
 * This class implements the MAC layer of the eNodeB device
 */
class NrEnbMac :   public Object
{
  friend class EnbMacMemberNrEnbCmacSapProvider;
  friend class EnbMacMemberNrMacSapProvider<NrEnbMac>;
  friend class EnbMacMemberNrFfMacSchedSapUser;
  friend class EnbMacMemberNrFfMacCschedSapUser;
  friend class EnbMacMemberNrEnbPhySapUser;

public:
  static TypeId GetTypeId (void);

  NrEnbMac (void);
  virtual ~NrEnbMac (void);
  virtual void DoDispose (void);


  /**
   * \brief Set the scheduler SAP provider
   * \param s a pointer SAP provider of the FF packet scheduler
   */
  void SetNrFfMacSchedSapProvider (NrFfMacSchedSapProvider* s);
  /**
   * \brief Get the scheduler SAP user
   * \return a pointer to the SAP user of the scheduler
   */
  NrFfMacSchedSapUser* GetNrFfMacSchedSapUser (void);
  /**
   * \brief Set the control scheduler SAP provider
   * \param s a pointer to the control scheduler SAP provider
   */
  void SetNrFfMacCschedSapProvider (NrFfMacCschedSapProvider* s);
  /**
   * \brief Get the control scheduler SAP user
   * \return a pointer to the control scheduler SAP user
   */
  NrFfMacCschedSapUser* GetNrFfMacCschedSapUser (void);



  /**
   * \brief Set the MAC SAP user
   * \param s a pointer to the MAC SAP user
   */
  void SetNrMacSapUser (NrMacSapUser* s);
  /**
   * \brief Get the MAC SAP provider
   * \return a pointer to the SAP provider of the MAC
   */
  NrMacSapProvider* GetNrMacSapProvider (void);
  /**
   * \brief Set the control MAC SAP user
   * \param s a pointer to the control MAC SAP user
   */
  void SetNrEnbCmacSapUser (NrEnbCmacSapUser* s);
  /**
   * \brief Get the control MAC SAP provider
   * \return a pointer to the control MAC SAP provider
   */
  NrEnbCmacSapProvider* GetNrEnbCmacSapProvider (void);


  /**
  * \brief Get the eNB-PHY SAP User
  * \return a pointer to the SAP User of the PHY
  */
  NrEnbPhySapUser* GetNrEnbPhySapUser ();

  /**
  * \brief Set the PHY SAP Provider
  * \param s a pointer to the PHY SAP provider
  */
  void SetNrEnbPhySapProvider (NrEnbPhySapProvider* s);

  /**
   * TracedCallback signature for DL scheduling events.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] mcs0 The MCS for transport block.. 
   * \param [in] tbs0Size
   * \param [in] mcs1 The MCS for transport block.
   * \param [in] tbs1Size
   */
  typedef void (* DlSchedulingTracedCallback)
    (uint32_t frame, uint32_t subframe,  uint16_t rnti,
     uint8_t mcs0, uint16_t tbs0Size,
     uint8_t mcs1, uint16_t tbs1Size);

  /**
   *  TracedCallback signature for UL scheduling events.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] mcs  The MCS for transport block
   * \param [in] tbsSize
   */
  typedef void (* UlSchedulingTracedCallback)
    (uint32_t frame, uint32_t subframe, uint16_t rnti,
     uint8_t mcs, uint16_t tbsSize);
  
private:

  /**
  * \brief Receive a DL CQI ideal control message
  * \param msg the DL CQI message
  */
  void ReceiveDlCqiNrControlMessage  (Ptr<DlCqiNrControlMessage> msg);

  void DoReceiveNrControlMessage (Ptr<NrControlMessage> msg);

  /**
  * \brief Receive a CE element containing the buffer status report
  * \param bsr the BSR message
  */
  void ReceiveBsrMessage  (NrMacCeListElement_s bsr);

 
  void DoUlCqiReport (NrFfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi);



  // forwarded from NrEnbCmacSapProvider
  void DoConfigureMac (uint8_t ulBandwidth, uint8_t dlBandwidth);
  void DoAddUe (uint16_t rnti);
  void DoRemoveUe (uint16_t rnti);
  void DoAddLc (NrEnbCmacSapProvider::LcInfo lcinfo, NrMacSapUser* msu);
  void DoReconfigureLc (NrEnbCmacSapProvider::LcInfo lcinfo);
  void DoReleaseLc (uint16_t  rnti, uint8_t lcid);
  void DoUeUpdateConfigurationReq (NrEnbCmacSapProvider::UeConfig params);
  NrEnbCmacSapProvider::RachConfig DoGetRachConfig ();
  NrEnbCmacSapProvider::AllocateNcRaPreambleReturnValue DoAllocateNcRaPreamble (uint16_t rnti);

  // forwarded from NrMacSapProvider
  void DoTransmitPdu (NrMacSapProvider::TransmitPduParameters);
  void DoReportBufferStatus (NrMacSapProvider::ReportBufferStatusParameters);


  // forwarded from FfMacCchedSapUser
  void DoCschedCellConfigCnf (NrFfMacCschedSapUser::CschedCellConfigCnfParameters params);
  void DoCschedUeConfigCnf (NrFfMacCschedSapUser::CschedUeConfigCnfParameters params);
  void DoCschedLcConfigCnf (NrFfMacCschedSapUser::CschedLcConfigCnfParameters params);
  void DoCschedLcReleaseCnf (NrFfMacCschedSapUser::CschedLcReleaseCnfParameters params);
  void DoCschedUeReleaseCnf (NrFfMacCschedSapUser::CschedUeReleaseCnfParameters params);
  void DoCschedUeConfigUpdateInd (NrFfMacCschedSapUser::CschedUeConfigUpdateIndParameters params);
  void DoCschedCellConfigUpdateInd (NrFfMacCschedSapUser::CschedCellConfigUpdateIndParameters params);

  // forwarded from NrFfMacSchedSapUser
  void DoSchedDlConfigInd (NrFfMacSchedSapUser::SchedDlConfigIndParameters ind);
  void DoSchedUlConfigInd (NrFfMacSchedSapUser::SchedUlConfigIndParameters params);

  // forwarded from NrEnbPhySapUser
  void DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo);
  void DoReceiveRachPreamble (uint8_t prachId);

public:
  // legacy public for use the Phy callback
  void DoReceivePhyPdu (Ptr<Packet> p);

private:
  void DoUlInfoListElementHarqFeeback (NrUlInfoListElement_s params);
  void DoDlInfoListElementHarqFeeback (NrDlInfoListElement_s params);

  //            rnti,             lcid, SAP of the RLC instance
  std::map <uint16_t, std::map<uint8_t, NrMacSapUser*> > m_rlcAttached;

  std::vector <NrCqiListElement_s> m_dlCqiReceived; // DL-CQI received
  std::vector <NrFfMacSchedSapProvider::SchedUlCqiInfoReqParameters> m_ulCqiReceived; // UL-CQI received
  std::vector <NrMacCeListElement_s> m_ulCeReceived; // CE received (BSR up to now)

  std::vector <NrDlInfoListElement_s> m_dlInfoListReceived; // DL HARQ feedback received

  std::vector <NrUlInfoListElement_s> m_ulInfoListReceived; // UL HARQ feedback received


  /*
  * Map of UE's info element (see 4.3.12 of FF MAC Scheduler API)
  */
//   std::map <uint16_t,NrUlInfoListElement_s> m_ulInfoListElements; 



  NrMacSapProvider* m_macSapProvider;
  NrEnbCmacSapProvider* m_cmacSapProvider;
  NrMacSapUser* m_macSapUser;
  NrEnbCmacSapUser* m_cmacSapUser;


  NrFfMacSchedSapProvider* m_schedSapProvider;
  NrFfMacCschedSapProvider* m_cschedSapProvider;
  NrFfMacSchedSapUser* m_schedSapUser;
  NrFfMacCschedSapUser* m_cschedSapUser;

  // PHY-SAP
  NrEnbPhySapProvider* m_enbPhySapProvider;
  NrEnbPhySapUser* m_enbPhySapUser;

  uint32_t m_frameNo;
  uint32_t m_subframeNo;
  /**
   * Trace information regarding DL scheduling
   * Frame number, Subframe number, RNTI, MCS of TB1, size of TB1,
   * MCS of TB2 (0 if not present), size of TB2 (0 if not present)
   */
  TracedCallback<uint32_t, uint32_t, uint16_t,
                 uint8_t, uint16_t, uint8_t, uint16_t> m_dlScheduling;
  /**
   * Trace information regarding UL scheduling
   * Frame number, Subframe number, RNTI, MCS of TB, size of TB
   */
  TracedCallback<uint32_t, uint32_t, uint16_t,
                 uint8_t, uint16_t> m_ulScheduling;
  
  uint8_t m_macChTtiDelay; // delay of MAC, PHY and channel in terms of TTIs


  std::map <uint16_t, DlHarqProcessesBuffer_t> m_miDlHarqProcessesPackets; // Packet under trasmission of the DL HARQ process
  
  uint8_t m_numberOfRaPreambles;
  uint8_t m_preambleTransMax;
  uint8_t m_raResponseWindowSize;

  /**
   * info associated with a preamble allocated for non-contention based RA
   * 
   */
  struct NcRaPreambleInfo
  {   
    uint16_t rnti; ///< rnti previously allocated for this non-contention based RA procedure
    Time expiryTime; ///< value the expiration time of this allocation (so that stale preambles can be reused)
  };

  /**
   * map storing as key the random acccess preamble IDs allocated for
   * non-contention based access, and as value the associated info
   * 
   */
  std::map<uint8_t, NcRaPreambleInfo> m_allocatedNcRaPreambleMap;
 
  std::map<uint8_t, uint32_t> m_receivedRachPreambleCount;

  std::map<uint8_t, uint32_t> m_rapIdRntiMap;
};

} // end namespace ns3

#endif /* NR_ENB_MAC_ENTITY_H */
