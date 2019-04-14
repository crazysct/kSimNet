/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 * Modification: Dizhi Zhou <dizhi.zhou@gmail.com>    // modify codes related to downlink scheduler
 */

#ifndef FDTBFQ_FF_MAC_SCHEDULER_H
#define FDTBFQ_FF_MAC_SCHEDULER_H

#include <ns3/nr-common.h>
#include <ns3/nr-ff-mac-csched-sap.h>
#include <ns3/nr-ff-mac-sched-sap.h>
#include <ns3/nr-ff-mac-scheduler.h>
#include <vector>
#include <map>
#include <ns3/nstime.h>
#include <ns3/nr-amc.h>
#include <ns3/nr-ffr-sap.h>

// value for SINR outside the range defined by FF-API, used to indicate that there
// is no CQI for this element
#define NO_SINR -5000


#define HARQ_PROC_NUM 8
#define HARQ_DL_TIMEOUT 11

namespace ns3 {


typedef std::vector < uint8_t > DlHarqProcessesStatus_t;
typedef std::vector < uint8_t > DlHarqProcessesTimer_t;
typedef std::vector < NrDlDciListElement_s > DlHarqProcessesDciBuffer_t;
typedef std::vector < std::vector <struct NrRlcPduListElement_s> > RlcPduList_t; // vector of the LCs and layers per UE
typedef std::vector < RlcPduList_t > DlHarqRlcPduListBuffer_t; // vector of the 8 HARQ processes per UE

typedef std::vector < NrUlDciListElement_s > UlHarqProcessesDciBuffer_t;
typedef std::vector < uint8_t > UlHarqProcessesStatus_t;


/**
 *  Flow information
 */
struct fdtbfqsFlowPerf_t
{
  Time flowStart;
  uint64_t packetArrivalRate;     /// packet arrival rate( byte/s)
  uint64_t tokenGenerationRate;   /// token generation rate ( byte/s )
  uint32_t tokenPoolSize;         /// current size of token pool (byte)
  uint32_t maxTokenPoolSize;      /// maximum size of token pool (byte)
  int counter;                    /// the number of token borrow or given to token bank
  uint32_t burstCredit;           /// the maximum number of tokens connection i can borrow from the bank each time
  int debtLimit;                  /// counter threshold that the flow cannot further borrow tokens from bank
  uint32_t creditableThreshold;   /// the flow cannot borrow token from bank until the number of token it has deposited to bank reaches this threshold
};

/**
 * \ingroup ff-api
 * \brief Implements the SCHED SAP and CSCHED SAP for a Frequency Domain Token Bank Fair Queue  scheduler
 *
 * This class implements the interface defined by the NrFfMacScheduler abstract class
 */

class NrFdTbfqFfMacScheduler : public NrFfMacScheduler
{
public:
  /**
   * \brief Constructor
   *
   * Creates the MAC Scheduler interface implementation
   */
  NrFdTbfqFfMacScheduler ();

  /**
   * Destructor
   */
  virtual ~NrFdTbfqFfMacScheduler ();

  // inherited from Object
  virtual void DoDispose (void);
  static TypeId GetTypeId (void);

  // inherited from NrFfMacScheduler
  virtual void SetNrFfMacCschedSapUser (NrFfMacCschedSapUser* s);
  virtual void SetNrFfMacSchedSapUser (NrFfMacSchedSapUser* s);
  virtual NrFfMacCschedSapProvider* GetNrFfMacCschedSapProvider ();
  virtual NrFfMacSchedSapProvider* GetNrFfMacSchedSapProvider ();

  // FFR SAPs
  virtual void SetNrFfrSapProvider (NrFfrSapProvider* s);
  virtual NrFfrSapUser* GetNrFfrSapUser ();

  friend class FdTbfqSchedulerMemberCschedSapProvider;
  friend class FdTbfqSchedulerMemberSchedSapProvider;

  void TransmissionModeConfigurationUpdate (uint16_t rnti, uint8_t txMode);

private:
  //
  // Implementation of the CSCHED API primitives
  // (See 4.1 for description of the primitives)
  //

  void DoCschedCellConfigReq (const struct NrFfMacCschedSapProvider::CschedCellConfigReqParameters& params);

  void DoCschedUeConfigReq (const struct NrFfMacCschedSapProvider::CschedUeConfigReqParameters& params);

  void DoCschedLcConfigReq (const struct NrFfMacCschedSapProvider::CschedLcConfigReqParameters& params);

  void DoCschedLcReleaseReq (const struct NrFfMacCschedSapProvider::CschedLcReleaseReqParameters& params);

  void DoCschedUeReleaseReq (const struct NrFfMacCschedSapProvider::CschedUeReleaseReqParameters& params);

  //
  // Implementation of the SCHED API primitives
  // (See 4.2 for description of the primitives)
  //

  void DoSchedDlRlcBufferReq (const struct NrFfMacSchedSapProvider::SchedDlRlcBufferReqParameters& params);

  void DoSchedDlPagingBufferReq (const struct NrFfMacSchedSapProvider::SchedDlPagingBufferReqParameters& params);

  void DoSchedDlMacBufferReq (const struct NrFfMacSchedSapProvider::SchedDlMacBufferReqParameters& params);

  void DoSchedDlTriggerReq (const struct NrFfMacSchedSapProvider::SchedDlTriggerReqParameters& params);

  void DoSchedDlRachInfoReq (const struct NrFfMacSchedSapProvider::SchedDlRachInfoReqParameters& params);

  void DoSchedDlCqiInfoReq (const struct NrFfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params);

  void DoSchedUlTriggerReq (const struct NrFfMacSchedSapProvider::SchedUlTriggerReqParameters& params);

  void DoSchedUlNoiseInterferenceReq (const struct NrFfMacSchedSapProvider::SchedUlNoiseInterferenceReqParameters& params);

  void DoSchedUlSrInfoReq (const struct NrFfMacSchedSapProvider::SchedUlSrInfoReqParameters& params);

  void DoSchedUlMacCtrlInfoReq (const struct NrFfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params);

  void DoSchedUlCqiInfoReq (const struct NrFfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params);


  int GetRbgSize (int dlbandwidth);

  int LcActivePerFlow (uint16_t rnti);

  double EstimateUlSinr (uint16_t rnti, uint16_t rb);

  void RefreshDlCqiMaps (void);
  void RefreshUlCqiMaps (void);

  void UpdateDlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size);
  void UpdateUlRlcBufferInfo (uint16_t rnti, uint16_t size);

  /**
  * \brief Update and return a new process Id for the RNTI specified
  *
  * \param rnti the RNTI of the UE to be updated
  * \return the process id  value
  */
  uint8_t UpdateHarqProcessId (uint16_t rnti);

  /**
  * \brief Return the availability of free process for the RNTI specified
  *
  * \param rnti the RNTI of the UE to be updated
  * \return the process id  value
  */
  uint8_t HarqProcessAvailability (uint16_t rnti);

  /**
  * \brief Refresh HARQ processes according to the timers
  *
  */
  void RefreshHarqProcesses ();

  Ptr<NrAmc> m_amc;

  /*
   * Vectors of UE's LC info
  */
  std::map <NrFlowId_t, NrFfMacSchedSapProvider::SchedDlRlcBufferReqParameters> m_rlcBufferReq;


  /*
  * Map of UE statistics (per RNTI basis) in downlink
  */
  std::map <uint16_t, fdtbfqsFlowPerf_t> m_flowStatsDl;

  /*
  * Map of UE statistics (per RNTI basis)
  */
  std::map <uint16_t, fdtbfqsFlowPerf_t> m_flowStatsUl;


  /*
  * Map of UE's DL CQI P01 received
  */
  std::map <uint16_t,uint8_t> m_p10CqiRxed;
  /*
  * Map of UE's timers on DL CQI P01 received
  */
  std::map <uint16_t,uint32_t> m_p10CqiTimers;

  /*
  * Map of UE's DL CQI A30 received
  */
  std::map <uint16_t,NrSbMeasResult_s> m_a30CqiRxed;
  /*
  * Map of UE's timers on DL CQI A30 received
  */
  std::map <uint16_t,uint32_t> m_a30CqiTimers;

  /*
  * Map of previous allocated UE per RBG
  * (used to retrieve info from UL-CQI)
  */
  std::map <uint16_t, std::vector <uint16_t> > m_allocationMaps;

  /*
  * Map of UEs' UL-CQI per RBG
  */
  std::map <uint16_t, std::vector <double> > m_ueCqi;
  /*
  * Map of UEs' timers on UL-CQI per RBG
  */
  std::map <uint16_t, uint32_t> m_ueCqiTimers;

  /*
  * Map of UE's buffer status reports received
  */
  std::map <uint16_t,uint32_t> m_ceBsrRxed;

  // MAC SAPs
  NrFfMacCschedSapUser* m_cschedSapUser;
  NrFfMacSchedSapUser* m_schedSapUser;
  NrFfMacCschedSapProvider* m_cschedSapProvider;
  NrFfMacSchedSapProvider* m_schedSapProvider;

  // FFR SAPs
  NrFfrSapUser* m_ffrSapUser;
  NrFfrSapProvider* m_ffrSapProvider;

  // Internal parameters
  NrFfMacCschedSapProvider::CschedCellConfigReqParameters m_cschedCellConfig;


  uint16_t m_nextRntiUl; // RNTI of the next user to be served next scheduling in UL

  uint32_t m_cqiTimersThreshold; // # of TTIs for which a CQI canbe considered valid

  std::map <uint16_t,uint8_t> m_uesTxMode; // txMode of the UEs

  uint64_t bankSize;  // the number of bytes in token bank

  int m_debtLimit;  // flow debt limit (byte)

  uint32_t m_creditLimit;  // flow credit limit (byte)

  uint32_t m_tokenPoolSize; // maximum size of token pool (byte)

  uint32_t m_creditableThreshold;  // threshold of flow credit

  // HARQ attributes
  /**
  * m_harqOn when false inhibit te HARQ mechanisms (by default active)
  */
  bool m_harqOn;
  std::map <uint16_t, uint8_t> m_dlHarqCurrentProcessId;
  //HARQ status
  // 0: process Id available
  // x>0: process Id equal to `x` trasmission count
  std::map <uint16_t, DlHarqProcessesStatus_t> m_dlHarqProcessesStatus;
  std::map <uint16_t, DlHarqProcessesTimer_t> m_dlHarqProcessesTimer;
  std::map <uint16_t, DlHarqProcessesDciBuffer_t> m_dlHarqProcessesDciBuffer;
  std::map <uint16_t, DlHarqRlcPduListBuffer_t> m_dlHarqProcessesRlcPduListBuffer;
  std::vector <NrDlInfoListElement_s> m_dlInfoListBuffered; // HARQ retx buffered

  std::map <uint16_t, uint8_t> m_ulHarqCurrentProcessId;
  //HARQ status
  // 0: process Id available
  // x>0: process Id equal to `x` trasmission count
  std::map <uint16_t, UlHarqProcessesStatus_t> m_ulHarqProcessesStatus;
  std::map <uint16_t, UlHarqProcessesDciBuffer_t> m_ulHarqProcessesDciBuffer;


  // RACH attributes
  std::vector <struct NrRachListElement_s> m_rachList;
  std::vector <uint16_t> m_rachAllocationMap;
  uint8_t m_ulGrantMcs; // MCS for UL grant (default 0)
};

} // namespace ns3

#endif /* FDTBFQ_FF_MAC_SCHEDULER_H */
