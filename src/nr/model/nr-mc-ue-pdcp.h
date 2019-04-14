/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab. 
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
 * Author of base class: Manuel Requena <manuel.requena@cttc.es>
 * Extension to DC devices by Michele Polese <michele.polese@gmail.com>
 */

#ifndef MC_UE_PDCP_H
#define MC_UE_PDCP_H

#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

#include "ns3/object.h"

#include <ns3/nr-pdcp-sap.h>
#include <ns3/nr-rlc-sap.h>
#include <ns3/nr-pdcp.h>
#include "ns3/network-module.h"
namespace ns3 {

/**
 * MC UE PDCP entity. It has 2 interfaces to the 2 RLC layers
 * Note: there is a single IMSI and lcid (no problem in having the same
 * in the 2 eNBs), but 2 rnti.
 */
class NrMcUePdcp : public NrPdcp 
{
  friend class NrMcUePdcpSpecificNrRlcSapUser;
  friend class NrPdcpSpecificNrPdcpSapProvider<NrMcUePdcp>;

public:
  NrMcUePdcp ();
  virtual ~NrMcUePdcp ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * Set the RNTI of the UE in the MmWave eNB
   *
   * \param rnti
   */
  void SetMmWaveRnti (uint16_t rnti1, uint16_t rnti2);

  /**
   * Set the RNTI of the UE in the NR eNB
   *
   * \param rnti
   */
  void SetRnti (uint16_t rnti);
  void BufferingAndReordering(Ptr<Packet>p); // this is reordering function
  void BufferingAndReordering_New(Ptr<Packet>p); // this is reordering function

  void printData(std::string filename, uint16_t SN);
  void t_ReordringTimer_Expired();
  void t_ReorderingTimer_Expired_New();

  /**
   * Set the ldid
   *
   * \param lcId
   */
  void SetLcId (uint8_t lcId);

  /**
   *
   *
   * \param s the PDCP SAP user to be used by this MC_PDCP
   */
  void SetNrPdcpSapUser (NrPdcpSapUser * s);

  /**
   *
   *
   * \return the PDCP SAP Provider interface offered to the RRC by this MC_PDCP
   */
  NrPdcpSapProvider* GetNrPdcpSapProvider ();

  /**
   *
   *
   * \param s the Nr RLC SAP Provider to be used by this MC_PDCP
   */
  void SetNrRlcSapProvider (NrRlcSapProvider * s);

  /**
   *
   *
   * \return the RLC SAP User interface offered to the RLC by this MC_PDCP
   */
  NrRlcSapUser* GetNrRlcSapUser ();

  /**
   *
   *
   * \param s the MmWave RLC SAP Provider to be used by this MC_PDCP
   */
  void SetMmWaveRlcSapProvider (NrRlcSapProvider * s);
  void SetMmWaveRlcSapProvider_2 (NrRlcSapProvider * s); //sjkang1110

  static const uint16_t MAX_PDCP_SN = 32768;

  /**
   * Status variables of the PDCP
   * 
   */
  struct Status
  {
    uint16_t txSn; ///< TX sequence number
    uint16_t rxSn; ///< RX sequence number
  };
  struct BufferedPackets{
 	  uint16_t sequenceNumber;
 	  NrPdcpSapUser::ReceivePdcpSduParameters params;
 	  uint16_t RX_HFN;
   };

  /** 
   * 
   * \return the current status of the PDCP
   */
  Status GetStatus ();

  /**
   * Set the status of the PDCP
   * 
   * \param s 
   */
  void SetStatus (Status s);

  /**
   * TracedCallback for PDU transmission event.
   *
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] size Packet size.
   */
  typedef void (* PduTxTracedCallback)
    (uint16_t rnti, uint8_t lcid, uint32_t size);

  /**
   * TracedCallback signature for PDU receive event.
   *
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] size Packet size.
   * \param [in] delay Delay since packet sent, in ns..
   */
  typedef void (* PduRxTracedCallback)
    (const uint16_t rnti, const uint8_t lcid,
     const uint32_t size, const uint64_t delay);

  /**
   * Switch between NR and MmWave
   */
  void SwitchConnection(bool useMmWaveConnection);

  void CalculatePdcpThroughput(std::ofstream *stream ); //sjkang

  void SetStreams ( std::ofstream * stream); //sjkang
  void MeasureSN_Difference(); //sjkang1116

protected:
  // Interface provided to upper RRC entity
  virtual void DoTransmitPdcpSdu (Ptr<Packet> p);
  virtual void DoTransmitPdcpControlPacket(Ptr<Packet> p, uint16_t cellId);//sjkang
  virtual void sendControlMessage();
  NrPdcpSapUser* m_pdcpSapUser;
  NrPdcpSapProvider* m_pdcpSapProvider;

  // Interface provided to lower RLC entity
  virtual void DoReceivePdu (Ptr<Packet> p);

  NrRlcSapUser* m_rlcSapUser;
  NrRlcSapProvider* m_rlcSapProvider;
  NrRlcSapProvider* m_mmWaveRlcSapProvider;
  NrRlcSapProvider* m_mmWaveRlcSapProvider_2; //sjkang1110

  uint16_t m_rnti;
  uint8_t m_lcid;
  uint16_t m_mmWaveRnti_73, m_mmWaveRnti_28;


  /**
   * Used to inform of a PDU delivery to the RLC SAP provider.
   * The parameters are RNTI, LCID and bytes delivered
   */
  TracedCallback<uint16_t, uint8_t, uint32_t> m_txPdu;
  /**
   * Used to inform of a PDU reception from the RLC SAP user.
   * The parameters are RNTI, LCID, bytes delivered and delivery delay in nanoseconds. 
   */
  TracedCallback<uint16_t, uint8_t, uint32_t, uint64_t> m_rxPdu;
  EventId t_ReorderingTimer;
private:
  /**
   * State variables. See section 7.1 in TS 36.323
   */
  uint16_t m_txSequenceNumber;
  uint16_t m_rxSequenceNumber;

  /**
   * Constants. See section 7.2 in TS 36.323
   */
  static const uint16_t m_maxPdcpSn = 32767;

  bool m_useMmWaveConnection;
  bool m_alwaysNrUplink;
  int  Last_Submitted_PDCP_RX_SN;
  /////sjkang for enabling reordering
  int *temp ;
 Time  expiredTime;
 int receivedPDCP_SN;
    int  Reordering_PDCP_RX_COUNT;
    int Next_PDCP_RX_SN;
    std::map <int,NrPdcpSapUser::ReceivePdcpSduParameters> PdcpBuffer;

    uint64_t discardedPacketSize=0;
    uint32_t numberOfDiscaredPackets=0;
    static const int reorderingWindow =16384;//8192;//2048;
    std::map<uint16_t, uint16_t>RX_HFNbySN;
    int present_RX_HFN;
    uint32_t 	SumOfPacketSize=0;
    uint32_t	 orderdedSumOfPacket=0;
    uint16_t	numberOfReorderingTimeout=0;
    bool check;
    bool check_2;

///////// for measuring SN difference

    int tempBuffer[100]; //sjkang0810
    uint64_t *temp_n;
     Time tempTime; //sjkang0810
     Time previousTime;
     uint16_t counter=0;
     //for calculating Pdcp Tput
     uint64_t TotalPacketSize;
     uint64_t  TotalPacketSize_ordered;
     double TotalTime;

   std::ofstream * m_SN_DifferenceStream; //sjkang1116
    //uint16_t temp_SN; //sjkang1116
    uint64_t cellIdToSN_1, cellIdToSN_2; //sjkang1116
    uint16_t cellId_1, cellId_2; //sjkang1116
    bool m_isEnableReordering;
    bool firstPacket ;


    std::map <uint64_t,NrPdcpSapUser::ReceivePdcpSduParameters> NewPdcpBuffer;
    std::map<uint64_t,uint16_t> HFN;
    std::map<uint64_t,uint32_t> SN;
    uint32_t RCVD_SN;
    uint16_t RCVD_HFN;
    uint64_t RCVD_COUNT;
    uint64_t RX_NEXT;
    uint64_t RX_RECORD;
    uint64_t RX_DELIV;
    uint64_t LAST_SUBMITED;
    bool outOfDelivery = false;

    // for processing packet duplication
  //  std::map <uint16_t, uint16_t> checkPacketDuplication; //sjkang
    std::vector<uint32_t> CheckSN;

};


} // namespace ns3

#endif // MC_UE_PDCP_H
