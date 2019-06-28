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
 * Author: Nicola Baldo  <nbaldo@cttc.es>
 * Author: Marco Miozzo <mmiozzo@cttc.es>
 */

#ifndef NR_UE_MAC_ENTITY_H
#define NR_UE_MAC_ENTITY_H



#include <map>

#include <ns3/nr-mac-sap.h>
#include <ns3/nr-ue-cmac-sap.h>
#include <ns3/nr-ue-phy-sap.h>
#include <ns3/nstime.h>
#include <ns3/event-id.h>
#include <vector>
#include <ns3/packet.h>
#include <ns3/packet-burst.h>


namespace ns3 {

class UniformRandomVariable;

class NrUeMac :   public Object
{
  friend class UeMemberNrUeCmacSapProvider;
  friend class UeMemberNrMacSapProvider;
  friend class UeMemberNrUePhySapUser;

public:
  static TypeId GetTypeId (void);

  NrUeMac ();
  virtual ~NrUeMac ();
  virtual void DoDispose (void);

  NrMacSapProvider*  GetNrMacSapProvider (void);
  void  SetNrUeCmacSapUser (NrUeCmacSapUser* s);
  NrUeCmacSapProvider*  GetNrUeCmacSapProvider (void);

  /**
  * \brief Get the PHY SAP user
  * \return a pointer to the SAP user of the PHY
  */
  NrUePhySapUser* GetNrUePhySapUser ();

  /**
  * \brief Set the PHY SAP Provider
  * \param s a pointer to the PHY SAP Provider
  */
  void SetNrUePhySapProvider (NrUePhySapProvider* s);
  
  /**
  * \brief Forwarded from NrUePhySapUser: trigger the start from a new frame
  *
  * \param frameNo frame number
  * \param subframeNo subframe number
  */
  void DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo);

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);

private:
  // forwarded from MAC SAP
  void DoTransmitPdu (NrMacSapProvider::TransmitPduParameters params);
  void DoReportBufferStatus (NrMacSapProvider::ReportBufferStatusParameters params);

  // forwarded from UE CMAC SAP
  void DoConfigureRach (NrUeCmacSapProvider::RachConfig rc);
  void DoStartContentionBasedRandomAccessProcedure ();
  void DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t rapId, uint8_t prachMask);
  void DoAddLc (uint8_t lcId, NrUeCmacSapProvider::LogicalChannelConfig lcConfig, NrMacSapUser* msu);
  void DoRemoveLc (uint8_t lcId);
  void DoReset ();

  // forwarded from PHY SAP
  void DoReceivePhyPdu (Ptr<Packet> p);
  void DoReceiveNrControlMessage (Ptr<NrControlMessage> msg);
  
  // internal methods
  void RandomlySelectAndSendRaPreamble ();
  void SendRaPreamble (bool contention);
  void StartWaitingForRaResponse ();
  void RecvRaResponse (NrBuildRarListElement_s raResponse);
  void RaResponseTimeout (bool contention);
  void SendReportBufferStatus (void);
  void RefreshHarqProcessesPacketBuffer (void);

private:

  struct LcInfo
  {
    NrUeCmacSapProvider::LogicalChannelConfig lcConfig;
    NrMacSapUser* macSapUser;
  };

  std::map <uint8_t, LcInfo> m_lcInfoMap;

  NrMacSapProvider* m_macSapProvider;

  NrUeCmacSapUser* m_cmacSapUser;
  NrUeCmacSapProvider* m_cmacSapProvider;

  NrUePhySapProvider* m_uePhySapProvider;
  NrUePhySapUser* m_uePhySapUser;
  
  std::map <uint8_t, NrMacSapProvider::ReportBufferStatusParameters> m_ulBsrReceived; // BSR received from RLC (the last one)
  
  
  Time m_bsrPeriodicity;
  Time m_bsrLast;
  
  bool m_freshUlBsr; // true when a BSR has been received in the last TTI

  uint8_t m_harqProcessId;
  std::vector < Ptr<PacketBurst> > m_miUlHarqProcessesPacket; // Packets under trasmission of the UL HARQ processes
  std::vector < uint8_t > m_miUlHarqProcessesPacketTimer; // timer for packet life in the buffer

  uint16_t m_rnti;

  bool m_rachConfigured;
  NrUeCmacSapProvider::RachConfig m_rachConfig;
  uint8_t m_raPreambleId;
  uint8_t m_preambleTransmissionCounter;
  uint16_t m_backoffParameter;
  EventId m_noRaResponseReceivedEvent;
  Ptr<UniformRandomVariable> m_raPreambleUniformVariable;

  uint32_t m_frameNo;
  uint32_t m_subframeNo;
  uint8_t m_raRnti;
  bool m_waitingForRaResponse;
};

} // namespace ns3

#endif // NR_UE_MAC_ENTITY
