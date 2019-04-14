/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *         Budiarto Herman <budiarto.herman@magister.fi>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#include "nr-ue-rrc.h"

#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>
#include <ns3/simulator.h>

#include <ns3/nr-rlc.h>
#include <ns3/nr-rlc-tm.h>
#include <ns3/nr-rlc-um.h>
#include <ns3/nr-rlc-am.h>
#include <ns3/nr-pdcp.h>
#include <ns3/nr-mc-ue-pdcp.h>
#include <ns3/nr-radio-bearer-info.h>
#include <ns3/nr-ue-mac.h>
#include <ns3/nr-rlc-am-header.h>

#include <ns3/nr-rlc-sdu-status-tag.h>
#include <ns3/nr-pdcp-header.h>
#include <ns3/nr-rlc-um-lowlat.h>
#include "ns3/nr-pdcp-tag.h"
#include <ns3/nr-rlc-sap.h>


#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrUeRrc");

/////////////////////////////
// CMAC SAP forwarder
/////////////////////////////

class UeMemberNrUeCmacSapUser : public NrUeCmacSapUser
{
public:
  UeMemberNrUeCmacSapUser (NrUeRrc* rrc);

  virtual void SetTemporaryCellRnti (uint16_t rnti);
  virtual void NotifyRandomAccessSuccessful ();
  virtual void SendServiceRequest ();
  virtual void NotifyRandomAccessFailed ();

private:
  NrUeRrc* m_rrc;
};

UeMemberNrUeCmacSapUser::UeMemberNrUeCmacSapUser (NrUeRrc* rrc)
  : m_rrc (rrc)
{
}

void
UeMemberNrUeCmacSapUser::SetTemporaryCellRnti (uint16_t rnti)

{
   NS_LOG_FUNCTION(this);
	m_rrc->DoSetTemporaryCellRnti (rnti);
}


void
UeMemberNrUeCmacSapUser::NotifyRandomAccessSuccessful ()
{
  NS_LOG_FUNCTION(this);
	m_rrc->DoNotifyRandomAccessSuccessful ();
}


void
UeMemberNrUeCmacSapUser::SendServiceRequest ()
{
  NS_LOG_FUNCTION(this);
	m_rrc->DoSendServiceRequest ();
}


void
UeMemberNrUeCmacSapUser::NotifyRandomAccessFailed ()
{
  m_rrc->DoNotifyRandomAccessFailed ();
}






/// Map each of UE RRC states to its string representation.
static const std::string g_ueRrcStateName[NrUeRrc::NUM_STATES] =
{
  "IDLE_START",
  "IDLE_CELL_SEARCH",
  "IDLE_WAIT_MIB_SIB1",
  "IDLE_WAIT_MIB",
  "IDLE_WAIT_SIB1",
  "IDLE_CAMPED_NORMALLY",
  "IDLE_WAIT_SIB2",
  "IDLE_RANDOM_ACCESS",
  "IDLE_CONNECTING",
  "CONNECTED_NORMALLY",
  "CONNECTED_HANDOVER",
  "CONNECTED_PHY_PROBLEM",
  "CONNECTED_REESTABLISHING"
};

/**
 * \param s The UE RRC state.
 * \return The string representation of the given state.
 */
static const std::string & ToString (NrUeRrc::State s)
{
  return g_ueRrcStateName[s];
}


/////////////////////////////
// ue RRC methods
/////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NrUeRrc);


NrUeRrc::NrUeRrc ()
  : m_cphySapProvider (0),
    m_cmacSapProvider (0),
    m_rrcSapUser (0),
    m_macSapProvider (0),
    m_asSapUser (0),
    m_state (IDLE_START),
    m_imsi (0),
    m_rnti (0),
    m_cellId (0),
    m_useRlcSm (true),
    m_connectionPending (false),
    m_hasReceivedMib (false),
    m_hasReceivedSib1 (false),
    m_hasReceivedSib2 (false),
    m_csgWhiteList (0),
    m_ncRaStarted (true)
{
  NS_LOG_FUNCTION (this);
  m_cphySapUser = new MemberNrUeCphySapUser<NrUeRrc> (this);
  m_cmacSapUser = new UeMemberNrUeCmacSapUser (this);
  m_rrcSapProvider = new MemberNrUeRrcSapProvider<NrUeRrc> (this);
  m_drbPdcpSapUser = new NrPdcpSpecificNrPdcpSapUser<NrUeRrc> (this);
  m_asSapProvider = new MemberNrAsSapProvider<NrUeRrc> (this);
  m_isSecondaryRRC=false;
  m_isThirdRrc=false;
  //m_onNr = true; // MC devices first connect to a NR macro eNB
}


NrUeRrc::~NrUeRrc ()
{
  NS_LOG_FUNCTION (this);
}

void
NrUeRrc::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_cphySapUser;
  delete m_cmacSapUser;
  delete m_rrcSapProvider;
  delete m_drbPdcpSapUser;
  delete m_asSapProvider;
  std::cout<< "UE will destroy drb in NR-UE-RRC" <<std::endl;

  m_drbMap.clear ();
  m_rlcMap.clear ();
}

TypeId
NrUeRrc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrUeRrc")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddConstructor<NrUeRrc> ()
    .AddAttribute ("DataRadioBearerMap", "List of UE RadioBearerInfo for Data Radio Bearers by LCID.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&NrUeRrc::m_drbMap),
                   MakeObjectMapChecker<NrDataRadioBearerInfo> ())
    .AddAttribute ("DataRadioRlcMap", "List of UE RLC for Secondary Connection by LCID.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&NrUeRrc::m_rlcMap),
                   MakeObjectMapChecker<RlcBearerInfo> ())
    .AddAttribute ("Srb0", "SignalingRadioBearerInfo for SRB0",
                   PointerValue (),
                   MakePointerAccessor (&NrUeRrc::m_srb0),
                   MakePointerChecker<NrSignalingRadioBearerInfo> ())
    .AddAttribute ("Srb1", "SignalingRadioBearerInfo for SRB1",
                   PointerValue (),
                   MakePointerAccessor (&NrUeRrc::m_srb1),
                   MakePointerChecker<NrSignalingRadioBearerInfo> ())
    .AddAttribute ("CellId",
                   "Serving cell identifier",
                   UintegerValue (0), // unused, read-only attribute
                   MakeUintegerAccessor (&NrUeRrc::GetCellId),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("C-RNTI",
                   "Cell Radio Network Temporary Identifier",
                   UintegerValue (0), // unused, read-only attribute
                   MakeUintegerAccessor (&NrUeRrc::GetRnti),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("T300",
                   "Timer for the RRC Connection Establishment procedure "
                   "(i.e., the procedure is deemed as failed if it takes longer than this)",
                   TimeValue (MilliSeconds (100)),//100
                   MakeTimeAccessor (&NrUeRrc::m_t300),
                   MakeTimeChecker ())
    .AddTraceSource ("MibReceived",
                     "trace fired upon reception of Master Information Block",
                     MakeTraceSourceAccessor (&NrUeRrc::m_mibReceivedTrace),
                     "ns3::NrUeRrc::MibSibHandoverTracedCallback")
    .AddTraceSource ("Sib1Received",
                     "trace fired upon reception of System Information Block Type 1",
                     MakeTraceSourceAccessor (&NrUeRrc::m_sib1ReceivedTrace),
                     "ns3::NrUeRrc::MibSibHandoverTracedCallback")
    .AddTraceSource ("Sib2Received",
                     "trace fired upon reception of System Information Block Type 2",
                     MakeTraceSourceAccessor (&NrUeRrc::m_sib2ReceivedTrace),
                     "ns3::NrUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("StateTransition",
                     "trace fired upon every UE RRC state transition",
                     MakeTraceSourceAccessor (&NrUeRrc::m_stateTransitionTrace),
                     "ns3::NrUeRrc::StateTracedCallback")
    .AddTraceSource ("InitialCellSelectionEndOk",
                     "trace fired upon successful initial cell selection procedure",
                     MakeTraceSourceAccessor (&NrUeRrc::m_initialCellSelectionEndOkTrace),
                     "ns3::NrUeRrc::CellSelectionTracedCallback")
    .AddTraceSource ("InitialCellSelectionEndError",
                     "trace fired upon failed initial cell selection procedure",
                     MakeTraceSourceAccessor (&NrUeRrc::m_initialCellSelectionEndErrorTrace),
                     "ns3::NrUeRrc::CellSelectionTracedCallback")
    .AddTraceSource ("RandomAccessSuccessful",
                     "trace fired upon successful completion of the random access procedure",
                     MakeTraceSourceAccessor (&NrUeRrc::m_randomAccessSuccessfulTrace),
                     "ns3::NrUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("RandomAccessError",
                     "trace fired upon failure of the random access procedure",
                     MakeTraceSourceAccessor (&NrUeRrc::m_randomAccessErrorTrace),
                     "ns3::NrUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("ConnectionEstablished",
                     "trace fired upon successful RRC connection establishment",
                     MakeTraceSourceAccessor (&NrUeRrc::m_connectionEstablishedTrace),
                     "ns3::NrUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("ConnectionTimeout",
                     "trace fired upon timeout RRC connection establishment because of T300",
                     MakeTraceSourceAccessor (&NrUeRrc::m_connectionTimeoutTrace),
                     "ns3::NrUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("ConnectionReconfiguration",
                     "trace fired upon RRC connection reconfiguration",
                     MakeTraceSourceAccessor (&NrUeRrc::m_connectionReconfigurationTrace),
                     "ns3::NrUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("HandoverStart",
                     "trace fired upon start of a handover procedure",
                     MakeTraceSourceAccessor (&NrUeRrc::m_handoverStartTrace),
                     "ns3::NrUeRrc::MibSibHandoverTracedCallback")
    .AddTraceSource ("HandoverEndOk",
                     "trace fired upon successful termination of a handover procedure",
                     MakeTraceSourceAccessor (&NrUeRrc::m_handoverEndOkTrace),
                     "ns3::NrUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("HandoverEndError",
                     "trace fired upon failure of a handover procedure",
                     MakeTraceSourceAccessor (&NrUeRrc::m_handoverEndErrorTrace),
                     "ns3::NrUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("SwitchToNr",
                     "trace fired upon switching to NR RAT",
                     MakeTraceSourceAccessor (&NrUeRrc::m_switchToNrTrace),
                     "ns3::NrUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("SwitchToMmWave",
                     "trace fired upon switching to MmWave RAT",
                     MakeTraceSourceAccessor (&NrUeRrc::m_switchToMmWaveTrace),
                     "ns3::NrUeRrc::ImsiCidRntiTracedCallback")
    .AddAttribute ("SecondaryRRC",
                     "True if this is the RRC in charge of the secondary cell (MmWaveCell) for a MC device",
                     BooleanValue (false),
                     MakeBooleanAccessor (&NrUeRrc::m_isSecondaryRRC),
                     MakeBooleanChecker ())
	.AddAttribute ("ThirdRRC",
					        "True if this is the RRC in charge of the secondary cell (MmWaveCell) for a MC device",
					                BooleanValue (false),
					                      MakeBooleanAccessor (&NrUeRrc::m_isThirdRrc),
					                      MakeBooleanChecker ())
    .AddAttribute ("InterRatHoCapable",
                     "True if this RRC supports hard handover between NR and MmWave",
                     BooleanValue (false),
                     MakeBooleanAccessor (&NrUeRrc::m_interRatHoCapable),
                     MakeBooleanChecker ())
  ;
  return tid;
}


void
NrUeRrc::SetNrUeCphySapProvider (NrUeCphySapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_cphySapProvider = s;
  m_nrCphySapProvider = s;
}

void
NrUeRrc::SetMmWaveUeCphySapProvider (NrUeCphySapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_mmWaveCphySapProvider = s;
}
void
NrUeRrc::SetMmWaveUeCphySapProvider_2 (NrUeCphySapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_mmWaveCphySapProvider_2 = s;
}
NrUeCphySapUser*
NrUeRrc::GetNrUeCphySapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_cphySapUser;
}

void
NrUeRrc::SetNrUeCmacSapProvider (NrUeCmacSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_cmacSapProvider = s;
  m_nrCmacSapProvider = s;
}



void
NrUeRrc::SetMmWaveUeCmacSapProvider (NrUeCmacSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_mmWaveCmacSapProvider = s;
}
void
NrUeRrc::SetMmWaveUeCmacSapProvider_2 (NrUeCmacSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_mmWaveCmacSapProvider_2 = s;
}
NrUeCmacSapUser*
NrUeRrc::GetNrUeCmacSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_cmacSapUser;
}

void
NrUeRrc::SetNrUeRrcSapUser (NrUeRrcSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_rrcSapUser = s;
}

NrUeRrcSapProvider*
NrUeRrc::GetNrUeRrcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_rrcSapProvider;
}

void
NrUeRrc::SetNrMacSapProvider (NrMacSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_macSapProvider = s;
  m_nrMacSapProvider = s;
}
void
NrUeRrc::SetMmWaveMacSapProvider_2 (NrMacSapProvider * s) //sjkang
{
  NS_LOG_FUNCTION (this << s);
  //m_macSapProvider_2 = s;
  //m_nrMacSapProvider_2 = s;
  m_mmWaveMacSapProvider_2=s;
}
void
NrUeRrc::SetMmWaveMacSapProvider (NrMacSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_mmWaveMacSapProvider = s;
}

void
NrUeRrc::SetAsSapUser (NrAsSapUser* s)
{
  m_asSapUser = s;
}

NrAsSapProvider* 
NrUeRrc::GetAsSapProvider ()
{
  return m_asSapProvider;
}

void 
NrUeRrc::SetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  m_imsi = imsi;
}

uint64_t
NrUeRrc::GetImsi (void) const
{
  return m_imsi;
}

uint16_t
NrUeRrc::GetRnti () const
{
  NS_LOG_FUNCTION (this);
  return m_rnti;
}

uint16_t
NrUeRrc::GetCellId () const
{
  NS_LOG_FUNCTION (this);
  return m_cellId;
}

void 
NrUeRrc::AddMmWaveCellId(uint16_t cellId)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG(m_interRatHoCapable, "Trying to setup unnecessary information on a non interRatHoCapable device");
  if(m_isMmWaveCellMap.find(cellId) == m_isMmWaveCellMap.end())
  {
    m_isMmWaveCellMap.insert(std::pair<uint16_t, bool> (cellId, true));
  }
  else // modify
  {
    m_isMmWaveCellMap.find(cellId)->second = true;
  }
}

void 
NrUeRrc::AddNrCellId(uint16_t cellId)
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT_MSG(m_interRatHoCapable, "Trying to setup unnecessary information on a non interRatHoCapable device");
  if(m_isMmWaveCellMap.find(cellId) == m_isMmWaveCellMap.end())
  {
    m_isMmWaveCellMap.insert(std::pair<uint16_t, bool> (cellId, false));
  }
  else // modify
  {
    m_isMmWaveCellMap.find(cellId)->second = false;
  }
}

bool
NrUeRrc::SwitchLowerLayerProviders (uint16_t cellId)
{
	 NS_LOG_INFO (this <<"SwitchLowerLayer"<< cellId <<m_isMmWaveCellMap.find(cellId)->second);
	if(m_isMmWaveCellMap.find(cellId) != m_isMmWaveCellMap.end()) ///if mmWave cell is true
  {
    if(m_isMmWaveCellMap.find(cellId)->second)
    {
      NS_LOG_INFO("Switch SAP to MmWave");
      NS_LOG_LOGIC("Before switch " << m_cphySapProvider << m_cmacSapProvider << m_macSapProvider);
      std::cout<<"NrUeRrc : Switch Lower Layer Provider"<<this<<" "<< sw<< std::endl;
      if(sw==0){ //sjkang
    	  m_cphySapProvider = m_mmWaveCphySapProvider_2;
    	    m_cmacSapProvider = m_mmWaveCmacSapProvider_2;
    	     m_macSapProvider = m_mmWaveMacSapProvider_2;
    	     sw=1;

      }else{
      m_cphySapProvider = m_mmWaveCphySapProvider;
      m_cmacSapProvider = m_mmWaveCmacSapProvider;
      m_macSapProvider = m_mmWaveMacSapProvider;
      }
      m_hasReceivedSib2 = false;
      NS_LOG_LOGIC("After switch " << m_cphySapProvider << m_cmacSapProvider << m_macSapProvider);
      return true;
    }
    else //if Nr cell is true
    {
      NS_LOG_INFO("Switch SAP to NR");
      NS_LOG_FUNCTION("Before switch " << m_cphySapProvider << m_cmacSapProvider << m_macSapProvider);
      m_cphySapProvider = m_nrCphySapProvider;
      m_cmacSapProvider = m_nrCmacSapProvider;
      m_macSapProvider = m_nrMacSapProvider;

      m_hasReceivedSib2 = false;

      NS_LOG_UNCOND("After switch " << m_cphySapProvider << m_cmacSapProvider << m_macSapProvider);
      return true;
    }
  }
  else
  {
    if(m_interRatHoCapable)
    {
      NS_FATAL_ERROR("Unkown cell, set it up in the helper!");
    }
    else
    {
      // do nothing, always use the ones set at the beginning
      return false;
    }
  }
}

uint8_t 
NrUeRrc::GetUlBandwidth () const
{
  NS_LOG_FUNCTION (this);
  return m_ulBandwidth;
}

uint8_t 
NrUeRrc::GetDlBandwidth () const
{
  NS_LOG_FUNCTION (this);
  return m_dlBandwidth;
}

uint16_t
NrUeRrc::GetDlEarfcn () const
{
  return m_dlEarfcn;
}

uint16_t 
NrUeRrc::GetUlEarfcn () const
{
  NS_LOG_FUNCTION (this);
  return m_ulEarfcn;
}

NrUeRrc::State
NrUeRrc::GetState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_state;
}

void
NrUeRrc::SetUseRlcSm (bool val) 
{
  NS_LOG_FUNCTION (this);
  m_useRlcSm = val;
}

void
NrUeRrc::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  // setup the UE side of SRB0
  uint8_t lcid = 0;

  Ptr<NrRlc> rlc = CreateObject<NrRlcTm> ()->GetObject<NrRlc> ();
  rlc->SetNrMacSapProvider (m_macSapProvider);
  rlc->SetRnti (m_rnti);
  rlc->SetLcId (lcid);

  m_srb0 = CreateObject<NrSignalingRadioBearerInfo> ();
  m_srb0->m_rlc = rlc;
  m_srb0->m_srbIdentity = 0;
  NrUeRrcSapUser::SetupParameters ueParams;
  ueParams.srb0SapProvider = m_srb0->m_rlc->GetNrRlcSapProvider ();
  ueParams.srb1SapProvider = 0;
  m_rrcSapUser->Setup (ueParams);

  // CCCH (LCID 0) is pre-configured, here is the hardcoded configuration:
  NrUeCmacSapProvider::LogicalChannelConfig lcConfig;
  lcConfig.priority = 0; // highest priority
  lcConfig.prioritizedBitRateKbps = 65535; // maximum
  lcConfig.bucketSizeDurationMs = 65535; // maximum
  lcConfig.logicalChannelGroup = 0; // all SRBs mapped to LCG 0

  
  if(m_interRatHoCapable)
  {
    m_mmWaveCmacSapProvider->AddLc (lcid, lcConfig, rlc->GetNrMacSapUser ());
    m_nrCmacSapProvider->AddLc (lcid, lcConfig, rlc->GetNrMacSapUser ());
  }
  else
  {
    m_cmacSapProvider->AddLc (lcid, lcConfig, rlc->GetNrMacSapUser ());
  }

}


void
NrUeRrc::DoSendData (Ptr<Packet> packet, uint8_t bid)
{
  NS_LOG_FUNCTION (this << packet);

  uint8_t drbid = Bid2Drbid (bid);
  NS_LOG_INFO("send data in NrUeRrc to Pdcp Layer");

  if (drbid != 0)
    {
  std::map<uint8_t, Ptr<NrDataRadioBearerInfo> >::iterator it =   m_drbMap.find (drbid);
  NS_ASSERT_MSG (it != m_drbMap.end (), "could not find bearer with drbid == " << drbid);

  NrPdcpSapProvider::TransmitPdcpSduParameters params;
  params.pdcpSdu = packet;
  params.rnti = m_rnti;
  params.lcid = it->second->m_logicalChannelIdentity;

  NS_LOG_LOGIC (this << " RNTI=" << m_rnti << " sending packet " << packet
                     << " on DRBID " << (uint32_t) drbid
                     << " (LCID " << (uint32_t) params.lcid << ")"
                     << " (" << packet->GetSize () << " bytes)");
  it->second->m_pdcp->GetNrPdcpSapProvider ()->TransmitPdcpSdu (params);
    }
}


void
NrUeRrc::DoDisconnect ()
{
  NS_LOG_FUNCTION (this);

  switch (m_state)
    {
    case IDLE_START:
    case IDLE_CELL_SEARCH:
    case IDLE_WAIT_MIB_SIB1:
    case IDLE_WAIT_MIB:
    case IDLE_WAIT_SIB1:
    case IDLE_CAMPED_NORMALLY:
      NS_LOG_INFO ("already disconnected");
      break;

    case IDLE_WAIT_SIB2:
    case IDLE_CONNECTING:
      NS_FATAL_ERROR ("cannot abort connection setup procedure");
      break;

    case CONNECTED_NORMALLY:
    case CONNECTED_HANDOVER:
    case CONNECTED_PHY_PROBLEM:
    case CONNECTED_REESTABLISHING:
      LeaveConnectedMode ();
      break;

    default: // i.e. IDLE_RANDOM_ACCESS
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void
NrUeRrc::DoNotifySecondaryCellConnected(uint16_t mmWaveRnti, uint16_t mmWaveCellId)
{
	NS_LOG_FUNCTION (this);
	//std::cout << "sjkang07010 -------------------> Nofify  Secondary cell connected " << std::endl;
	m_mmWaveCellId = mmWaveCellId;
  m_mmWaveRnti = mmWaveRnti;
  NS_LOG_FUNCTION(this);
  m_rrcSapUser->SendNotifySecondaryCellConnected(mmWaveRnti, mmWaveCellId); ///will go to mmWave-nr-rrc-protocol-real
 }
void
NrUeRrc::DoNotifySecondaryCellConnected(uint16_t mmWaveRnti_73G,uint16_t mmWaveRnti_28G, uint16_t mmWaveCellId_73G, uint16_t mmWaveCellId_28G)
{
	NS_LOG_FUNCTION (this);
	std::cout << "sjkang07010 -------------------> Nofify  Secondary cell connected " <<mmWaveRnti_28G<< mmWaveRnti_73G<< std::endl;
	m_mmWaveCellId_28G = mmWaveCellId_28G;
	m_mmWaveCellId_73G = mmWaveCellId_73G;
	m_mmWaveCellId = m_mmWaveCellId_28G; // the last connected cellID
  m_mmWaveRnti_73 = mmWaveRnti_73G;
  m_mmWaveRnti_28 = mmWaveRnti_28G;
  NS_LOG_FUNCTION(this);
  m_rrcSapUser->SendNotifySecondaryCellConnected(mmWaveRnti_28G, mmWaveCellId_28G); ///will go to mmWave-nr-rrc-protocol-real
 }
void
NrUeRrc::DoReceivePdcpSdu (NrPdcpSapUser::ReceivePdcpSduParameters params)
{
  NS_LOG_FUNCTION (this);
   m_asSapUser->RecvData (params.pdcpSdu);
}


void
NrUeRrc::DoSetTemporaryCellRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  m_rnti = rnti;
  m_srb0->m_rlc->SetRnti (m_rnti);
  m_cphySapProvider->SetRnti (m_rnti);
}

void
NrUeRrc::DoNotifyRadioLinkFailure (double lastSinrValue)
{
  NS_LOG_DEBUG("Rlf");
}

/*
void
NrUeRrc::DoNotifyRandomAccessSuccessful ()
{
  NS_LOG_FUNCTION (this << m_imsi << ToString (m_state));
  m_randomAccessSuccessfulTrace (m_imsi, m_cellId, m_rnti);

  switch (m_state)
    {
    case IDLE_RANDOM_ACCESS:
      {
        // we just received a RAR with a T-C-RNTI and an UL grant
        // send RRC connection request as message 3 of the random access procedure 
        SwitchToState (IDLE_CONNECTING);
        NrRrcSap::RrcConnectionRequest msg;
        msg.ueIdentity = m_imsi;
        msg.isMc = m_isSecondaryRRC ; //sjkang // 28G connection
        msg.isMc_2=m_isThirdRrc;// 73G connection
        m_rrcSapUser->SendRrcConnectionRequest (msg);
        m_connectionTimeout = Simulator::Schedule (m_t300,
                                                   &NrUeRrc::ConnectionTimeout,
                                                   this);
      }
      break;

    case CONNECTED_HANDOVER:
      {

    	  NrRrcSap::RrcConnectionReconfigurationCompleted msg;
        msg.rrcTransactionIdentifier = m_lastRrcTransactionIdentifier;
        m_rrcSapUser->SendRrcConnectionReconfigurationCompleted (msg);

        // The following is not needed
        //if(m_isSecondaryRRC)
        //{
        //  m_asSapUser->NotifyHandoverSuccessful (m_rnti, m_cellId); // this triggers MC reconfiguration         
        //}

        // 3GPP TS 36.331 section 5.5.6.1 Measurements related actions upon handover
        std::map<uint8_t, NrRrcSap::MeasIdToAddMod>::iterator measIdIt;
        for (measIdIt = m_varMeasConfig.measIdList.begin ();
             measIdIt != m_varMeasConfig.measIdList.end ();
             ++measIdIt)
          {
            VarMeasReportListClear (measIdIt->second.measId);
          }

        SwitchToState (CONNECTED_NORMALLY);
        m_handoverEndOkTrace (m_imsi, m_cellId, m_rnti);
        if(m_isSecondaryRRC) // an handover for secondary cells has happened. 
        // this trace is used to keep a consistent trace of the cell to which the UE is connected
        {
          NS_LOG_UNCOND("DoNotifyRandomAccessSuccessful at time " << Simulator::Now().GetSeconds());
          m_switchToMmWaveTrace(m_imsi, m_cellId, m_rnti);
        }
      }
      break;

    default:
      NS_FATAL_ERROR ("unexpected event in state " << ToString (m_state));
      break; 
    }
}
*/

void

NrUeRrc::DoNotifyRandomAccessSuccessful ()

{

  NS_LOG_FUNCTION (this << m_imsi << ToString (m_state));

  m_randomAccessSuccessfulTrace (m_imsi, m_cellId, m_rnti);



  switch (m_state)

    {

    case IDLE_RANDOM_ACCESS:

      {

        // we just received a RAR with a T-C-RNTI and an UL grant

        // send RRC connection request as message 3 of the random access procedure 

        SwitchToState (IDLE_CONNECTING);

        NrRrcSap::RrcConnectionRequest msg;

        msg.ueIdentity = m_imsi;

        msg.isMc = m_isSecondaryRRC ; //sjkang // 28G connection

        msg.isMc_2=m_isThirdRrc;// 73G connection

        

        // hmlee

        /*

        msg.registrationType = registrationType;

        msg.GUTI = guti;

        */

        m_rrcSapUser->SendRrcConnectionRequest (msg); 

        m_connectionTimeout = Simulator::Schedule (m_t300,

                                                   &NrUeRrc::ConnectionTimeout,

                                                   this);

      }

      break;



    case CONNECTED_HANDOVER:

      {



    	  NrRrcSap::RrcConnectionReconfigurationCompleted msg;

        msg.rrcTransactionIdentifier = m_lastRrcTransactionIdentifier;

        m_rrcSapUser->SendRrcConnectionReconfigurationCompleted (msg);



        // The following is not needed

        //if(m_isSecondaryRRC)

        //{

        //  m_asSapUser->NotifyHandoverSuccessful (m_rnti, m_cellId); // this triggers MC reconfiguration         

        //}



        // 3GPP TS 36.331 section 5.5.6.1 Measurements related actions upon handover

        std::map<uint8_t, NrRrcSap::MeasIdToAddMod>::iterator measIdIt;

        for (measIdIt = m_varMeasConfig.measIdList.begin ();

             measIdIt != m_varMeasConfig.measIdList.end ();

             ++measIdIt)

          {

            VarMeasReportListClear (measIdIt->second.measId);

          }



        SwitchToState (CONNECTED_NORMALLY);

        m_handoverEndOkTrace (m_imsi, m_cellId, m_rnti);

        if(m_isSecondaryRRC) // an handover for secondary cells has happened. 

        // this trace is used to keep a consistent trace of the cell to which the UE is connected

        {

          NS_LOG_UNCOND("DoNotifyRandomAccessSuccessful at time " << Simulator::Now().GetSeconds());

          m_switchToMmWaveTrace(m_imsi, m_cellId, m_rnti);

        }

      }

      break;



    default:

      NS_FATAL_ERROR ("unexpected event in state " << ToString (m_state));

      break; 

    }

}

void
NrUeRrc::DoSendServiceRequest (){

  //m_randomAccessSuccessfulTrace (m_imsi, m_cellId, m_rnti);

  switch (m_state)
    {
    case IDLE_RANDOM_ACCESS:
      {
        // we just received a RAR with a T-C-RNTI and an UL grant
        // send RRC connection request as message 3 of the random access procedure 
        SwitchToState (IDLE_CONNECTING);
        NrRrcSap::RrcConnectionRequest msg;
        msg.ueIdentity = m_imsi;
        msg.isMc = m_isSecondaryRRC ; //sjkang // 28G connection
        msg.isMc_2=m_isThirdRrc;// 73G connection
        m_rrcSapUser->SendRrcConnectionRequest (msg);
        m_connectionTimeout = Simulator::Schedule (m_t300,
                                                   &NrUeRrc::ConnectionTimeout,
                                                   this);
      }
      break;

    default:
      NS_FATAL_ERROR ("unexpected event in state " << ToString (m_state));
      break; 
    }
}

void
NrUeRrc::DoNotifyRandomAccessFailed ()
{
  NS_LOG_FUNCTION (this << m_imsi << ToString (m_state));
  m_randomAccessErrorTrace (m_imsi, m_cellId, m_rnti);

  switch (m_state)
    {
    case IDLE_RANDOM_ACCESS:
      {
        SwitchToState (IDLE_CAMPED_NORMALLY);
        m_asSapUser->NotifyConnectionFailed ();
      }
      break;

    case CONNECTED_HANDOVER:
      {
        m_handoverEndErrorTrace (m_imsi, m_cellId, m_rnti);
        /**
         * \todo After a handover failure because of a random access failure,
         *       send an RRC Connection Re-establishment and switch to
         *       CONNECTED_REESTABLISHING state.
         */
      }
      break;

    default:
      NS_FATAL_ERROR ("unexpected event in state " << ToString (m_state));
      break;
    }
}


void
NrUeRrc::DoSetCsgWhiteList (uint32_t csgId)
{
  NS_LOG_FUNCTION (this << m_imsi << csgId);
  m_csgWhiteList = csgId;
}

void 
NrUeRrc::DoStartCellSelection (uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << m_imsi << dlEarfcn);
  NS_ASSERT_MSG (m_state == IDLE_START,
                 "cannot start cell selection from state " << ToString (m_state));
  m_dlEarfcn = dlEarfcn;
  m_cphySapProvider->StartCellSearch (dlEarfcn);
  SwitchToState (IDLE_CELL_SEARCH);
}

void 
NrUeRrc::DoForceCampedOnEnb (uint16_t cellId, uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << m_imsi << cellId << dlEarfcn);

  switch (m_state)
    {
    case IDLE_START:
      m_cellId = cellId;
      SwitchLowerLayerProviders(m_cellId); // for InterRat HO      
      m_dlEarfcn = dlEarfcn;
      m_cphySapProvider->SynchronizeWithEnb (m_cellId, m_dlEarfcn);
      SwitchToState (IDLE_WAIT_MIB);
      break;

    case IDLE_CELL_SEARCH:
    case IDLE_WAIT_MIB_SIB1:
    case IDLE_WAIT_SIB1:
      NS_FATAL_ERROR ("cannot abort cell selection " << ToString (m_state));
      break;

    case IDLE_WAIT_MIB:
      NS_LOG_INFO ("already forced to camp to cell " << m_cellId);
      break;

    case IDLE_CAMPED_NORMALLY:
    case IDLE_WAIT_SIB2:
    case IDLE_RANDOM_ACCESS:
    case IDLE_CONNECTING:
      NS_LOG_INFO ("already camped to cell " << m_cellId);
      break;

    case CONNECTED_NORMALLY:
    case CONNECTED_HANDOVER:
    case CONNECTED_PHY_PROBLEM:
    case CONNECTED_REESTABLISHING:
      NS_LOG_INFO ("already connected to cell " << m_cellId);
      break;

    default:
      NS_FATAL_ERROR ("unexpected event in state " << ToString (m_state));
      break;
    }

}

void
NrUeRrc::DoConnect ()
{
  NS_LOG_FUNCTION (this << m_imsi);

  switch (m_state)
    {
    case IDLE_START:
    case IDLE_CELL_SEARCH:
    case IDLE_WAIT_MIB_SIB1:
    case IDLE_WAIT_SIB1:
    case IDLE_WAIT_MIB:
      m_connectionPending = true;
      break;

    case IDLE_CAMPED_NORMALLY:
      m_connectionPending = true;
      SwitchToState (IDLE_WAIT_SIB2);
      break;

    case IDLE_WAIT_SIB2:
    case IDLE_RANDOM_ACCESS:
    case IDLE_CONNECTING:
      NS_LOG_INFO ("already connecting");
      break;

    case CONNECTED_NORMALLY:
    case CONNECTED_REESTABLISHING:
    case CONNECTED_HANDOVER:
      NS_LOG_INFO ("already connected");
      break;

    default:
      NS_FATAL_ERROR ("unexpected event in state " << ToString (m_state));
      break;
    }
}



// CPHY SAP methods

void
NrUeRrc::DoRecvMasterInformationBlock (uint16_t cellId,
                                        NrRrcSap::MasterInformationBlock msg)
{ 
  m_dlBandwidth = msg.dlBandwidth;
  m_cphySapProvider->SetDlBandwidth (msg.dlBandwidth);
  m_hasReceivedMib = true;
  m_mibReceivedTrace (m_imsi, m_cellId, m_rnti, cellId);

  switch (m_state)
    {
    case IDLE_WAIT_MIB:
      // manual attachment
      SwitchToState (IDLE_CAMPED_NORMALLY);
      break;

    case IDLE_WAIT_MIB_SIB1:
      // automatic attachment from Idle mode cell selection
      SwitchToState (IDLE_WAIT_SIB1);
      break;

    default:
      // do nothing extra
      break;
    }
}

void
NrUeRrc::DoRecvSystemInformationBlockType1 (uint16_t cellId,
                                             NrRrcSap::SystemInformationBlockType1 msg)
{
  switch (m_state)
    {
    case IDLE_WAIT_SIB1:
      NS_ASSERT_MSG (cellId == msg.cellAccessRelatedInfo.cellIdentity,
                     "Cell identity in SIB1 does not match with the originating cell");
      m_hasReceivedSib1 = true;
      m_lastSib1 = msg;
      m_sib1ReceivedTrace (m_imsi, m_cellId, m_rnti, cellId);
      EvaluateCellForSelection ();
      break;

    case IDLE_CAMPED_NORMALLY:
    case IDLE_RANDOM_ACCESS:
    case IDLE_CONNECTING:
    case CONNECTED_NORMALLY:
    case CONNECTED_HANDOVER:
    case CONNECTED_PHY_PROBLEM:
    case CONNECTED_REESTABLISHING:
      NS_ASSERT_MSG (cellId == msg.cellAccessRelatedInfo.cellIdentity,
                     "Cell identity in SIB1 does not match with the originating cell");
      m_hasReceivedSib1 = true;
      m_lastSib1 = msg;
      m_sib1ReceivedTrace (m_imsi, m_cellId, m_rnti, cellId);
      break;

    case IDLE_WAIT_MIB_SIB1:
      // MIB has not been received, so ignore this SIB1
      break;

    default: // e.g. IDLE_START, IDLE_CELL_SEARCH, IDLE_WAIT_MIB, IDLE_WAIT_SIB2
      // do nothing
      break;
    }
}

void
NrUeRrc::DoReportUeMeasurements (NrUeCphySapUser::UeMeasurementsParameters params)
{
  NS_LOG_FUNCTION (this);

  // layer 3 filtering does not apply in IDLE mode
  bool useLayer3Filtering = (m_state == CONNECTED_NORMALLY);

  std::vector <NrUeCphySapUser::UeMeasurementsElement>::iterator newMeasIt;
  for (newMeasIt = params.m_ueMeasurementsList.begin ();
       newMeasIt != params.m_ueMeasurementsList.end (); ++newMeasIt)
    {
      SaveUeMeasurements (newMeasIt->m_cellId, newMeasIt->m_rsrp,
                          newMeasIt->m_rsrq, useLayer3Filtering);
    }

  if (m_state == IDLE_CELL_SEARCH)
    {
      // start decoding BCH
      SynchronizeToStrongestCell ();
    }
  else
    {
      std::map<uint8_t, NrRrcSap::MeasIdToAddMod>::iterator measIdIt;
      for (measIdIt = m_varMeasConfig.measIdList.begin ();
           measIdIt != m_varMeasConfig.measIdList.end (); ++measIdIt)
        {
          MeasurementReportTriggering (measIdIt->first);
        }
    }

} // end of NrUeRrc::DoReportUeMeasurements



// RRC SAP methods

void 
NrUeRrc::DoCompleteSetup (NrUeRrcSapProvider::CompleteSetupParameters params)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);
  m_srb0->m_rlc->SetNrRlcSapUser (params.srb0SapUser);
  if (m_srb1)
    {
      m_srb1->m_pdcp->SetNrPdcpSapUser (params.srb1SapUser);
    }
}


void 
NrUeRrc::DoRecvSystemInformation (NrRrcSap::SystemInformation msg) //sjkang
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti<<ToString(m_state));

  if (msg.haveSib2)
    {
      switch (m_state)
        {
        case IDLE_CAMPED_NORMALLY:
        case IDLE_WAIT_SIB2:
        case IDLE_RANDOM_ACCESS:
        case IDLE_CONNECTING:
        case CONNECTED_NORMALLY:
        case CONNECTED_HANDOVER:
        case CONNECTED_PHY_PROBLEM:
        case CONNECTED_REESTABLISHING:
          m_hasReceivedSib2 = true;
          m_ulBandwidth = msg.sib2.freqInfo.ulBandwidth;
          m_ulEarfcn = msg.sib2.freqInfo.ulCarrierFreq;
          m_sib2ReceivedTrace (m_imsi, m_cellId, m_rnti);
          NrUeCmacSapProvider::RachConfig rc;
          rc.numberOfRaPreambles = msg.sib2.radioResourceConfigCommon.rachConfigCommon.preambleInfo.numberOfRaPreambles;
          rc.preambleTransMax = msg.sib2.radioResourceConfigCommon.rachConfigCommon.raSupervisionInfo.preambleTransMax;
          rc.raResponseWindowSize = msg.sib2.radioResourceConfigCommon.rachConfigCommon.raSupervisionInfo.raResponseWindowSize;
          m_cmacSapProvider->ConfigureRach (rc);
          m_cphySapProvider->ConfigureUplink (m_ulEarfcn, m_ulBandwidth);
          m_cphySapProvider->ConfigureReferenceSignalPower(msg.sib2.radioResourceConfigCommon.pdschConfigCommon.referenceSignalPower);
          if (m_state == IDLE_WAIT_SIB2)
            {
              NS_ASSERT (m_connectionPending);
              StartConnection ();
            }
          if (m_state == CONNECTED_HANDOVER && m_interRatHoCapable && !m_ncRaStarted)
            {
              m_ncRaStarted = true;
              NS_LOG_INFO("StartNonContentionBasedRandomAccessProcedure after SIB2 received at time " << Simulator::Now());
              m_cmacSapProvider->StartNonContentionBasedRandomAccessProcedure (m_rnti, m_rachConfigDedicated.raPreambleIndex, m_rachConfigDedicated.raPrachMaskIndex);  
            }
          break;

        default: // IDLE_START, IDLE_CELL_SEARCH, IDLE_WAIT_MIB, IDLE_WAIT_MIB_SIB1, IDLE_WAIT_SIB1
          // do nothing
          break;
        }
    }

}


void 
NrUeRrc::DoRecvRrcConnectionSetup (NrRrcSap::RrcConnectionSetup msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti <<ToString(m_state));
  std::cout << "UE " << m_imsi << " receives RRC connection setup from eNB "<<m_cellId<< std::endl;
  switch (m_state)
    {
    case IDLE_CONNECTING:
      {
        ApplyRadioResourceConfigDedicated (msg.radioResourceConfigDedicated);
        m_connectionTimeout.Cancel ();
        SwitchToState (CONNECTED_NORMALLY);
        NrRrcSap::RrcConnectionSetupCompleted msg2;
        msg2.rrcTransactionIdentifier = msg.rrcTransactionIdentifier;  ///check RRC index
        m_rrcSapUser->SendRrcConnectionSetupCompleted (msg2);
        m_asSapUser->NotifyConnectionSuccessful (m_rnti);
        m_connectionEstablishedTrace (m_imsi, m_cellId, m_rnti);
      }
      break;

    default:
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void 
NrUeRrc::DoRecvRrcConnectToMmWave (uint16_t mmWaveCellId, uint16_t mmWaveCellId_2) //sjkang
{
	 NS_LOG_FUNCTION (this << " mmWaveCellId " <<mmWaveCellId);
		m_asSapUser->NotifyConnectToMmWave(mmWaveCellId, mmWaveCellId_2);//sjkang
}

void
NrUeRrc::DoRecvRrcConnectionReconfiguration (NrRrcSap::RrcConnectionReconfiguration msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti << ToString(m_state));
  std::cout << "UE receives RRC connection reconfiguration message from source eNB at time  "<<Simulator::Now().GetSeconds()<< std::endl;
if ((unsigned)(msg.HandoverCase)==0)//sjkang  // mmWave Stack of UE
{
  switch (m_state)
    {
    case CONNECTED_NORMALLY:
      if (msg.haveMobilityControlInfo)
        {
          NS_LOG_UNCOND ("UE :: " << m_rnti << " on cellId " << m_cellId << " haveMobilityControlInfo == true");
          SwitchToState (CONNECTED_HANDOVER);
          const NrRrcSap::MobilityControlInfo& mci = msg.mobilityControlInfo;
          m_handoverStartTrace (m_imsi, m_cellId, m_rnti, mci.targetPhysCellId);
          m_cmacSapProvider->Reset ();
          m_cphySapProvider->Reset ();
          m_cellId = mci.targetPhysCellId;
             SwitchLowerLayerProviders(m_cellId); // for InterRat HO
           NS_ASSERT (mci.haveCarrierFreq);
          NS_ASSERT (mci.haveCarrierBandwidth);
          m_cphySapProvider->SynchronizeWithEnb (m_cellId, mci.carrierFreq.dlCarrierFreq);
          m_cphySapProvider->SetDlBandwidth ( mci.carrierBandwidth.dlBandwidth);
          m_cphySapProvider->ConfigureUplink (mci.carrierFreq.ulCarrierFreq, mci.carrierBandwidth.ulBandwidth);

          uint16_t oldRnti = m_rnti;
          m_rnti = msg.mobilityControlInfo.newUeIdentity;

          m_srb0->m_rlc->SetRnti (m_rnti);
          NS_ASSERT_MSG (mci.haveRachConfigDedicated, "handover is only supported with non-contention-based random access procedure");
          if(m_interRatHoCapable)
          {
            m_rachConfigDedicated = mci.rachConfigDedicated;
            m_ncRaStarted = false;
          }
          else
          {
           std::cout <<"UE will start NonContentionBased Random Access Procedure. " <<"UE's rnti is "<<m_rnti <<std::endl;
        	  m_cmacSapProvider->StartNonContentionBasedRandomAccessProcedure (m_rnti, mci.rachConfigDedicated.raPreambleIndex, mci.rachConfigDedicated.raPrachMaskIndex);
          }
          m_cphySapProvider->SetRnti (m_rnti);
          m_lastRrcTransactionIdentifier = msg.rrcTransactionIdentifier;
          NS_ASSERT (msg.haveRadioResourceConfigDedicated);

          // we re-establish SRB1 by creating a new entity
          // note that we can't dispose the old entity now, because
          // it's in the current stack, so we would corrupt the stack
          // if we did so. Hence we schedule it for later disposal
          m_srb1Old = m_srb1;
          Simulator::ScheduleNow (&NrUeRrc::DisposeOldSrb1, this);
          m_srb1 = 0; // new instance will be be created within ApplyRadioResourceConfigDedicated
           //TODO find how to clear these maps and forward buffer contents!
          // clear the RLC maps after having forwarded buffer content 
        //  std::map <uint8_t, Ptr<NrDataRadioBearerInfo> > m_drbMap;
          ///std::map <uint8_t, Ptr<RlcBearerInfo> > m_rlcMap;
          m_drbMap.clear (); // dispose all DRBs
          //m_rlcMap.clear (); // dispose all MmWave RLCs
          std::map <uint8_t, Ptr<RlcBearerInfo> > ::iterator iter;

          for (iter = m_rlcMap.begin(); iter!=m_rlcMap.end(); iter++){ ///sjkang0709
        	  if (msg.HandoverCase == 2) {
        	 Ptr<NrRlc > rlc = iter->second->m_rlc_2->GetObject<NrRlc>();
        	 iter->second->Cleanup();
        	  iter->second->m_rlc_2 = rlc;
        	  }else if (msg.HandoverCase == 1){
        		  Ptr<NrRlc > rlc = iter->second->m_rlc->GetObject<NrRlc>();
        		  iter->second->Cleanup();
        		   iter->second->m_rlc = rlc;

        	  }

          }
       //  m_rlcMap.clear ();
          if(!m_isSecondaryRRC & !m_isThirdRrc)
          {
            ApplyRadioResourceConfigDedicated (msg.radioResourceConfigDedicated);
          }
          else
          {
            // this is the secondary mmWave RRC. When a secondary HO happens,
            // the primary NR RRC must be notified in order to update the RLC instances
            // Forward this message to NgcUeNas and then to NrUeRrc for NR RRC
        	  std::cout << "Notify Secondary cell Handover started " <<std::endl;
            m_asSapUser->NotifySecondaryCellHandoverStarted(oldRnti, m_rnti, m_cellId, msg.radioResourceConfigDedicated); // to EPS UE NAS
            msg.radioResourceConfigDedicated.drbToAddModList.clear(); // remove the drb list, since no Data RLC must be setup in this RRC
            // setup SRB1
            ApplyRadioResourceConfigDedicated (msg.radioResourceConfigDedicated);
          }



          if (msg.haveMeasConfig)
            {
              ApplyMeasConfig (msg.measConfig);
            }
          // RRC connection reconfiguration completed will be sent
          // after handover is complete
        }
      else
        {
          NS_LOG_INFO ("UE " << m_rnti << " on cellId " << m_cellId << " haveMobilityControlInfo == false");
          if (msg.haveRadioResourceConfigDedicated) /// haveRadioResourceconfigDedicated=true
            {
              ApplyRadioResourceConfigDedicated (msg.radioResourceConfigDedicated);
            } 
          if (msg.haveMeasConfig)
            {
              ApplyMeasConfig (msg.measConfig);
            }
          NrRrcSap::RrcConnectionReconfigurationCompleted msg2;
          msg2.rrcTransactionIdentifier = msg.rrcTransactionIdentifier;
          m_rrcSapUser->SendRrcConnectionReconfigurationCompleted (msg2); //from mmWave UE stack to mmwave eNB
          m_connectionReconfigurationTrace (m_imsi, m_cellId, m_rnti);
        }
      break;

    default:
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}
else{  //this is the case of handover
	m_asSapUser->SendRrcReconfigurationMessageforHandover(msg);//sjkang
}
}

// jhlim
void
NrUeRrc::DoRecvRrcIdentityRequest (NrRrcSap::RrcIdentityRequest msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti << ToString(m_state));
  std::cout << "UE receives RRC identity request message from source eNB at time  "<<Simulator::Now().GetSeconds()<< std::endl;
  
  NrRrcSap::RrcIdentityResponse msg2;
  m_rrcSapUser->SendRrcIdentityResponse (msg2);
}
void
NrUeRrc::DoRecvRrcRegistrationAccept (NrRrcSap::RrcRegistrationAccept msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti << ToString(m_state));
  std::cout << "UE receives RRC registration accept message from source eNB at time  "<<Simulator::Now().GetSeconds()<< std::endl;
  
  NrRrcSap::RrcRegistrationComplete msg2;
  m_rrcSapUser->SendRrcRegistrationComplete (msg2);
}

void 
NrUeRrc::DoRecvRrcConnectionReestablishment (NrRrcSap::RrcConnectionReestablishment msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);
  switch (m_state)
    {
    case CONNECTED_REESTABLISHING:
      {
        /**
         * \todo After receiving RRC Connection Re-establishment, stop timer
         *       T301, fire a new trace source, reply with RRC Connection
         *       Re-establishment Complete, and finally switch to
         *       CONNECTED_NORMALLY state. See Section 5.3.7.5 of 3GPP TS
         *       36.331.
         */
      }
      break;

    default:
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void 
NrUeRrc::DoRecvRrcConnectionReestablishmentReject (NrRrcSap::RrcConnectionReestablishmentReject msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);
  switch (m_state)
    {
    case CONNECTED_REESTABLISHING:
      {
        /**
         * \todo After receiving RRC Connection Re-establishment Reject, stop
         *       timer T301. See Section 5.3.7.8 of 3GPP TS 36.331.
         */
        LeaveConnectedMode ();
      }
      break;

    default:
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void 
NrUeRrc::DoRecvRrcConnectionRelease (NrRrcSap::RrcConnectionRelease msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);
  /// \todo Currently not implemented, see Section 5.3.8 of 3GPP TS 36.331.
}

void 
NrUeRrc::DoRecvRrcConnectionReject (NrRrcSap::RrcConnectionReject msg)
{
  NS_LOG_FUNCTION (this);
  m_connectionTimeout.Cancel ();

  m_cmacSapProvider->Reset ();       // reset the MAC
  m_hasReceivedSib2 = false;         // invalidate the previously received SIB2
  SwitchToState (IDLE_CAMPED_NORMALLY);
  m_asSapUser->NotifyConnectionFailed ();  // inform upper layer
}

void
NrUeRrc::DoRecvRrcConnectionSwitch (NrRrcSap::RrcConnectionSwitch msg)

{
	std::cout << "UE Recv Rrc Connection switch " <<std::endl;
 NS_LOG_FUNCTION(this);
	NS_LOG_INFO("Recv RRC Connection Switch on rnti " << m_rnti << " of cell " << m_cellId << " m_mmWaveCellId " << m_mmWaveCellId << " in state " << ToString(m_state));
  std::vector<uint8_t> drbidList = msg.drbidList;
  for(std::vector<uint8_t>::iterator iter = drbidList.begin(); iter != drbidList.end(); ++iter)
  {
    if(m_drbMap.find(*iter) != m_drbMap.end())
    {
      Ptr<NrMcUePdcp> pdcp = DynamicCast<NrMcUePdcp>(m_drbMap.find(*iter)->second->m_pdcp);
      if(pdcp != 0)
      {
        pdcp->SwitchConnection(msg.useMmWaveConnection);

        if(msg.useMmWaveConnection) // I was on NR, now I switch to mmWave
        {
          uint8_t lcid = m_drbMap.find(*iter)->second->m_logicalChannelIdentity;
          m_cmacSapProvider->RemoveLc (lcid);

          // before resetting the RLC, forward the content of the NR RLC to the mmWave RLC
          // check if this rlc is already in the map
          if(m_drbMap.find(*iter) != m_drbMap.end())
          {
            // get the RLC buffer content and forward it to the m_rlcMap RLC!
            CopyRlcBuffers(m_drbMap.find(*iter)->second->m_rlc, pdcp, lcid); //sjkang1113
           // CopyRlcBuffers(m_drbMap.find(*iter)->second->m_rlc)
          } 

          // reset the NR RLC
          TypeId rlcTypeId;
          if (m_useRlcSm)
            {
              rlcTypeId = NrRlcSm::GetTypeId ();
            }
          else
            {
              switch (m_drbMap.find(*iter)->second->m_rlcConfig.choice)
                {
                case NrRrcSap::RlcConfig::AM: 
                  rlcTypeId = NrRlcAm::GetTypeId ();
                  break;
          
                case NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL: 
                  rlcTypeId = NrRlcUm::GetTypeId ();
                  break;
          
                default:
                  NS_FATAL_ERROR ("unsupported RLC configuration");
                  break;                
                }
            }
  
          ObjectFactory rlcObjectFactory;
          rlcObjectFactory.SetTypeId (rlcTypeId);
          Ptr<NrRlc> rlc = rlcObjectFactory.Create ()->GetObject<NrRlc> ();
          rlc->SetNrMacSapProvider (m_macSapProvider);
          rlc->SetRnti (m_rnti);
          rlc->SetLcId (lcid);

          
          m_drbMap.find(*iter)->second->m_rlc = rlc;
 
          // we need PDCP only for real RLC, i.e., RLC/UM or RLC/AM
          // if we are using RLC/SM we don't care of anything above RLC
          if (rlcTypeId != NrRlcSm::GetTypeId ())
            {
              pdcp->SetNrRlcSapProvider (rlc->GetNrRlcSapProvider ()); 
              rlc->SetNrRlcSapUser (pdcp->GetNrRlcSapUser ());
            }

          struct NrUeCmacSapProvider::LogicalChannelConfig lcConfig;
          lcConfig.priority = m_drbMap.find(*iter)->second->m_logicalChannelConfig.priority;
          lcConfig.prioritizedBitRateKbps = m_drbMap.find(*iter)->second->m_logicalChannelConfig.prioritizedBitRateKbps;
          lcConfig.bucketSizeDurationMs =   m_drbMap.find(*iter)->second->m_logicalChannelConfig.bucketSizeDurationMs;
          lcConfig.logicalChannelGroup =    m_drbMap.find(*iter)->second->m_logicalChannelConfig.logicalChannelGroup;

          m_cmacSapProvider->AddLc (lcid,
                                    lcConfig,
                                    rlc->GetNrMacSapUser ());
          rlc->Initialize ();

        }
        else // I was on mmWave, and I switch to NR
        {
          m_mmWaveCmacSapProvider->RemoveLc(m_rlcMap.find(*iter)->second->logicalChannelIdentity);
          m_mmWaveCmacSapProvider_2->RemoveLc(m_rlcMap.find(*iter)->second->logicalChannelIdentity); //sjkang1110

          // before resetting, forward the content of the RLC in m_rlcMap to the RLC in m_drbMap
          if(m_rlcMap.find(*iter) != m_rlcMap.end())
          {
            // get the RLC buffer content and store it in this RLC
            CopyRlcBuffers(m_rlcMap.find(*iter)->second->m_rlc, pdcp, m_rlcMap.find(*iter)->second->logicalChannelIdentity);
            CopyRlcBuffers(m_rlcMap.find(*iter)->second->m_rlc_2, pdcp, m_rlcMap.find(*iter)->second->logicalChannelIdentity); //sjkang1113

          } 

          // create Rlc
          TypeId rlcTypeId;
          if (m_useRlcSm)
            {
              rlcTypeId = NrRlcSm::GetTypeId ();
              NS_LOG_INFO("SM");
            }
          else
            {
              switch (m_rlcMap.find(*iter)->second->rlcConfig.choice)
                {
                case NrRrcSap::RlcConfig::AM: 
                  rlcTypeId = NrRlcAm::GetTypeId ();
                  NS_LOG_INFO("AM");
                  break;
          
                case NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL: 
                  rlcTypeId = NrRlcUm::GetTypeId ();
                  NS_LOG_INFO("UM");
                  break;
          
                default:
                  NS_FATAL_ERROR ("unsupported RLC configuration");
                  break;                
                }
            }
  
          ObjectFactory rlcObjectFactory;
          rlcObjectFactory.SetTypeId (rlcTypeId);
          Ptr<NrRlc> rlc = rlcObjectFactory.Create ()->GetObject<NrRlc> ();
          rlc->SetNrMacSapProvider (m_mmWaveMacSapProvider); 
          rlc->SetRnti (m_mmWaveRnti_28);
          rlc->SetLcId (m_rlcMap.find(*iter)->second->logicalChannelIdentity);

          Ptr<NrRlc> rlc_2 = rlcObjectFactory.Create ()->GetObject<NrRlc> ();
           rlc_2->SetNrMacSapProvider (m_mmWaveMacSapProvider_2);
                    rlc_2->SetRnti (m_mmWaveRnti_73); //need to be modified later sjkang1113
                    rlc_2->SetLcId (m_rlcMap.find(*iter)->second->logicalChannelIdentity);


          struct NrUeCmacSapProvider::LogicalChannelConfig lcConfig;
          lcConfig.priority =               m_rlcMap.find(*iter)->second->logicalChannelConfig.priority;
          lcConfig.prioritizedBitRateKbps = m_rlcMap.find(*iter)->second->logicalChannelConfig.prioritizedBitRateKbps;
          lcConfig.bucketSizeDurationMs =   m_rlcMap.find(*iter)->second->logicalChannelConfig.bucketSizeDurationMs;
          lcConfig.logicalChannelGroup =    m_rlcMap.find(*iter)->second->logicalChannelConfig.logicalChannelGroup;      

          m_mmWaveCmacSapProvider->AddLc (m_rlcMap.find(*iter)->second->logicalChannelIdentity, 
                                  lcConfig,
                                  rlc->GetNrMacSapUser ());
          m_mmWaveCmacSapProvider->AddLc (m_rlcMap.find(*iter)->second->logicalChannelIdentity,
                                            lcConfig,
                                            rlc_2->GetNrMacSapUser ());

          if (rlcTypeId != NrRlcSm::GetTypeId ())
          {
            pdcp->SetMmWaveRnti (m_mmWaveRnti_73, m_mmWaveCellId_28G);
            pdcp->SetMmWaveRlcSapProvider (rlc->GetNrRlcSapProvider ());
            pdcp ->SetMmWaveRlcSapProvider_2(rlc_2->GetNrRlcSapProvider());
            rlc->SetNrRlcSapUser (pdcp->GetNrRlcSapUser ());
            rlc_2->SetNrRlcSapUser(pdcp->GetNrRlcSapUser ());

          } 
          rlc->Initialize();
          rlc_2 ->Initialize();
          m_rlcMap.find(*iter)->second->m_rlc = rlc;
          m_rlcMap.find(*iter)->second->m_rlc_2 = rlc_2;

        }

        if(msg.useMmWaveConnection)
        {
          NS_LOG_INFO("NrUeRrc SwitchToMmWave " << m_imsi << m_cellId << m_rnti << " at time " << Simulator::Now().GetSeconds());
          m_switchToMmWaveTrace(m_imsi, m_mmWaveCellId, m_mmWaveRnti);
        }
        else if(!msg.useMmWaveConnection)
        {
          NS_LOG_INFO("NrUeRrc SwitchToNr " << m_imsi << m_cellId << m_rnti << " at time " << Simulator::Now().GetSeconds());
          m_switchToNrTrace(m_imsi, m_cellId, m_rnti);          
        }
      }
      else
      {
        NS_FATAL_ERROR("Trying to switch a non MC bearer");
      }
    }
  }
}


/*
 * Merge 2 buffers of RlcAmPdus into 1 vector with increment order of Pdus
 */
std::vector < NrRlcAm::RetxPdu >
NrUeRrc::MergeBuffers(std::vector < NrRlcAm::RetxPdu > first, std::vector < NrRlcAm::RetxPdu > second)
{
  NrRlcAmHeader rlcamHeader_1, rlcamHeader_2;
  std::vector < NrRlcAm::RetxPdu> result;
  std::vector < NrRlcAm::RetxPdu>::iterator it_1 = first.begin();
  std::vector < NrRlcAm::RetxPdu>::iterator it_2 = second.begin();
  bool end_1_reached = false;
  bool end_2_reached = false;
  while (it_1 != first.end() && it_2 != second.end()){
    while ((*it_1).m_pdu == 0){
      ++it_1;
      if(it_1 == first.end())
      {
        end_1_reached = true;
        break;
      }
    }
    while ((*it_2).m_pdu == 0){
      ++it_2;
      if(it_2 == second.end())
      {
        end_2_reached = true;
        break;
      }
    }
    if(!end_1_reached && !end_2_reached)
    {
      (*it_1).m_pdu->PeekHeader(rlcamHeader_1);
      (*it_2).m_pdu->PeekHeader(rlcamHeader_2);
      if (rlcamHeader_1.GetSequenceNumber() > rlcamHeader_2.GetSequenceNumber()){
        result.push_back((*it_2));  
        ++it_2;       
      }
      else if (rlcamHeader_2.GetSequenceNumber() > rlcamHeader_1.GetSequenceNumber()){
        result.push_back((*it_1));
        ++it_1;         
      }
      else {
        result.push_back((*it_1));
        ++it_1;
        ++it_2;
      }
      NS_LOG_DEBUG ("first,second = " << rlcamHeader_1.GetSequenceNumber() << "," << rlcamHeader_2.GetSequenceNumber());
    }
    else
    {
      break;
    }
  }
  while (it_1 != first.end()){
    result.push_back((*it_1));
    it_1++;
  }
  while (it_2 != second.end()){
    result.push_back((*it_2));
    it_2++;
  }
  return result;
}

// This code from the LL HO implementation is refactored in a function
// in order to be used also when switching from NR to MmWave and back
void
NrUeRrc::CopyRlcBuffers(Ptr<NrRlc> rlc, Ptr<NrPdcp> pdcp, uint16_t lcid)
{
  // RlcBuffers forwarding only for RlcAm bearers.
  if (0 != rlc->GetObject<NrRlcAm> ())
  {
    //Copy nr-rlc-am.m_txOnBuffer to X2 forwarding buffer.
    Ptr<NrRlcAm> rlcAm = rlc->GetObject<NrRlcAm>();
    uint32_t txonBufferSize = rlcAm->GetTxBufferSize();
    std::vector < Ptr<Packet> > txonBuffer = rlcAm->GetTxBuffer();
    //m_rlcBufferToBeForwardedSize =  drbIt->second->m_rlc->GetObject<NrRlcAm>()->GetTxBufferSize();
    //m_rlcBufferToBeForwarded = drbIt->second->m_rlc->GetObject<NrRlcAm>()->GetTxBuffer();
    uint32_t txedBufferSize = rlcAm->GetTxedBufferSize();
    std::vector < NrRlcAm::RetxPdu > txedBuffer = rlcAm->GetTxedBuffer();
    uint32_t retxBufferSize = rlcAm->GetRetxBufferSize();
    std::vector < NrRlcAm::RetxPdu > retxBuffer = rlcAm->GetRetxBuffer();
    
    //Translate Pdus in Rlc txed/retx buffer into RLC Sdus
    //and put these Sdus into rlcAm->m_transmittingRlcSdus.
    NS_LOG_INFO("UE RRC: retxBuffer size = " << retxBufferSize);
    NS_LOG_INFO("UE RRC: txedBuffer size = " << txedBufferSize);
    //Merge txed and retx buffers into a single buffer before doing RlcPdusToRlc.
    if ( retxBufferSize + txedBufferSize > 0 ){
      std::vector< NrRlcAm::RetxPdu > sortedTxedRetxBuffer;
      if (retxBufferSize == 0){
        sortedTxedRetxBuffer = txedBuffer;
      }
      else if (txedBufferSize == 0){
        sortedTxedRetxBuffer = retxBuffer;
      }
      else {
        sortedTxedRetxBuffer = MergeBuffers(txedBuffer, retxBuffer);
      }
      rlcAm->RlcPdusToRlcSdus(sortedTxedRetxBuffer);  
    }

    //Construct the forwarding buffer
    //Forwarding buffer = retxBuffer + txedBuffer + txonBuffer.
    //if ( txonBufferSize > 0 )
    //{
      NrPdcpHeader pdcpHeader;
      uint32_t pos = 0;
      for (std::vector< Ptr<Packet> >::iterator it = txonBuffer.begin(); it != txonBuffer.end(); ++it)
      {
        pos++;
        if((*it)->GetSize() > 3) 
        {
          (*it)->PeekHeader(pdcpHeader);
          NS_LOG_DEBUG("UE RRC: txonBuffer SEQ = " << pdcpHeader.GetSequenceNumber() << " Size = " << (*it)->GetSize());
        }
        else
        {
          NS_LOG_UNCOND("UE RRC: Fragment too small in txonBuffer, pos " << pos);
        }
          
      }

      // this cycle adds the SDUs given by the merge of txed and retxed buffers
      if ( rlcAm->GetTransmittingRlcSduBufferSize() > 0 )
      { //something inside the RLC AM's transmitting buffer 
        NS_LOG_DEBUG ("UE RRC: ADDING TRANSMITTING SDUS OF RLC AM TO X2FORWARDINGBUFFER... Size = " << rlcAm->GetTransmittingRlcSduBufferSize() );
        //copy the RlcSdu buffer (map) to forwardingBuffer.
        std::map < uint32_t, Ptr<Packet> > rlcAmTransmittingBuffer = rlcAm->GetTransmittingRlcSduBuffer();
        NS_LOG_DEBUG ("UE RRC:  *** SIZE = " << rlcAmTransmittingBuffer.size());
        for (std::map< uint32_t, Ptr<Packet> >::iterator it = rlcAmTransmittingBuffer.begin(); it != rlcAmTransmittingBuffer.end(); ++it)
        {
          if (it->second != 0)
          {
            NS_LOG_DEBUG ( this << " add to forwarding buffer SEQ = " << it->first << " Ptr<Packet> = " << it->second );
            m_rlcBufferToBeForwarded.push_back(it->second);
          }
        } 
        NS_LOG_DEBUG(this << "UE RRC:  ADDING TXONBUFFER OF RLC AM " << m_rnti << " Size = " << txonBufferSize) ;

      
        Ptr<Packet> segmentedRlcsdu = rlcAm->GetSegmentedRlcsdu();
        if (segmentedRlcsdu != NULL){
          segmentedRlcsdu->PeekHeader(pdcpHeader);
          NS_LOG_DEBUG(this << "UE RRC: SegmentedRlcSdu = " << segmentedRlcsdu->GetSize() << " SEQ = " << pdcpHeader.GetSequenceNumber());
          //insert the complete version of the fragmented SDU to the front of txonBuffer.
          txonBuffer.insert(txonBuffer.begin(),segmentedRlcsdu);
        }
        m_rlcBufferToBeForwarded.insert(m_rlcBufferToBeForwarded.end(), txonBuffer.begin(), txonBuffer.end());
        m_rlcBufferToBeForwardedSize += rlcAm->GetTransmittingRlcSduBufferSize() + txonBufferSize;

        //Get the rlcAm
        std::vector < Ptr <Packet> > rlcAmTxedSduBuffer = rlcAm->GetTxedRlcSduBuffer();
        NrPdcpHeader pdcpHeader_1;
        m_rlcBufferToBeForwarded.at(0)->PeekHeader(pdcpHeader_1);
        uint16_t i = 0;
        for (std::vector< Ptr<Packet> >::iterator it = rlcAmTxedSduBuffer.begin(); it != rlcAmTxedSduBuffer.end(); ++it)
        {
          if ((*it) != NULL)
          {
            (*it)->PeekHeader(pdcpHeader);
            NS_LOG_DEBUG("UE RRC: rlcAmTxedSduBuffer SEQ = " << pdcpHeader.GetSequenceNumber() << " Size = " << (*it)->GetSize());
          
            //add the previous SDU of the forwarding buffer to the forwarding buffer.
            if (pdcpHeader.GetSequenceNumber() >= (pdcpHeader_1.GetSequenceNumber() - 2) && pdcpHeader.GetSequenceNumber() <= (pdcpHeader_1.GetSequenceNumber()) )
            {
              NS_LOG_DEBUG("UE RRC: Added previous SDU to forwarding buffer SEQ = " << pdcpHeader.GetSequenceNumber() << " Size = " << (*it)->GetSize());
              m_rlcBufferToBeForwarded.insert(m_rlcBufferToBeForwarded.begin()+i, (*it)->Copy());
              ++i;
            }
          }
        }
        
      }
      else 
      { //TransmittingBuffer is empty. Only copy TxonBuffer.
        NS_LOG_DEBUG(this << " UE RRC: ADDING TXONBUFFER OF RLC AM " << m_rnti << " Size = " << txonBufferSize) ;
        m_rlcBufferToBeForwarded = txonBuffer;
        m_rlcBufferToBeForwardedSize += txonBufferSize;
      }
    //}
  }
  //For RlcUM, no forwarding available as the simulator itself (seamless HO).
  //However, as the NR-UMTS book, PDCP txbuffer should be forwarded for seamless 
  //HO. Enable this code for txbuffer forwarding in seamless HO (which is believe to 
  //be correct).
  else if (0 != rlc->GetObject<NrRlcUm> ())
  {
    //Copy nr-rlc-um.m_txOnBuffer to X2 forwarding buffer.
    NS_LOG_DEBUG(this << " UE RRC: Copying txonBuffer from RLC UM " << m_rnti);
    m_rlcBufferToBeForwarded = rlc->GetObject<NrRlcUm>()->GetTxBuffer();
    m_rlcBufferToBeForwardedSize =  rlc->GetObject<NrRlcUm>()->GetTxBufferSize();
  }
  else if (0 != rlc->GetObject<NrRlcUmLowLat> ())
  {
    //Copy nr-rlc-um-low-lat.m_txOnBuffer to X2 forwarding buffer.
    NS_LOG_DEBUG(this << " UE RRC: Copying txonBuffer from RLC UM " << m_rnti);
    m_rlcBufferToBeForwarded = rlc->GetObject<NrRlcUmLowLat>()->GetTxBuffer();
    m_rlcBufferToBeForwardedSize =  rlc->GetObject<NrRlcUmLowLat>()->GetTxBufferSize();
  }
  //NrRlcAm m_txBuffer stores PDCP "PDU".
  NS_LOG_DEBUG(this << " UE RRC: m_x2forw buffer size = " << m_rlcBufferToBeForwardedSize);
    //Forwarding the packet inside m_rlcBufferToBeForwarded to target eNB. 

  while (!m_rlcBufferToBeForwarded.empty())
  {
    NS_LOG_DEBUG(this << " UE RRC: Forwarding m_rlcBufferToBeForwarded to target eNB, lcid = " << lcid );
    //Remove tags to get PDCP SDU from PDCP PDU.
    //Ptr<Packet> rlcSdu =  (*(m_rlcBufferToBeForwarded.begin()))->Copy();
    Ptr<Packet> rlcSdu =  m_rlcBufferToBeForwarded.at(0);
    //Tags to be removed from rlcSdu (from outer to inner)
    //NrRlcSduStatusTag rlcSduStatusTag;
    //RlcTag  rlcTag; //rlc layer timestamp
    //PdcpTag pdcpTag;  //pdcp layer timestamp
    NrPdcpHeader pdcpHeader;
    
    
    NS_LOG_DEBUG ("UE RRC: RlcSdu size = " << rlcSdu->GetSize() );
    //rlcSdu->RemoveHeader(pdcpHeader); //remove pdcp header
    
    //only forward data PDCP PDUs (1-DATA_PDU,0-CTR_PDU)
    if(rlcSdu->GetSize() >= 3)
    {
      rlcSdu->RemoveHeader(pdcpHeader);
      if (pdcpHeader.GetDcBit() == 1 )
      { //ignore control SDU.
        NS_LOG_LOGIC ("UE RRC: SEQ = " << pdcpHeader.GetSequenceNumber());
        NS_LOG_LOGIC ("UE RRC: removed pdcp header, size = " << rlcSdu->GetSize());

        rlcSdu->RemoveAllPacketTags(); // this does not remove byte tags
        NS_LOG_LOGIC ("UE RRC: removed tags, size = " << rlcSdu->GetSize() );

        /*
        rlcSdu->RemovePacketTag(rlcSduStatusTag); //remove Rlc status tag.
        NS_LOG_DEBUG ("removed rlc status tag, size = " << rlcSdu->GetSize() );
        rlcSdu->RemovePacketTag(rlcTag);  //remove Rlc timestamp
        NS_LOG_DEBUG ("removed rlc timestamp, size = " << rlcSdu->GetSize() );
        //rlcSdu->RemoveByteTag(pdcpTag); //remove pdcp timestamp
        //NS_LOG_DEBUG ("removed pdcp timestamp, size = " << rlcSdu->GetSize());
        */

        // re-inject into PDCP
        NrPdcpSapProvider::TransmitPdcpSduParameters pdcpParams;
        pdcpParams.pdcpSdu = rlcSdu;
        pdcpParams.rnti = m_rnti;
        pdcpParams.lcid = lcid;
        pdcp->GetNrPdcpSapProvider()->TransmitPdcpSdu(pdcpParams);

      }
    }
    else
    {
      NS_LOG_UNCOND("UE RRC: Too small, not forwarded");
    }
    m_rlcBufferToBeForwardedSize -= (*(m_rlcBufferToBeForwarded.begin()))->GetSize();
    m_rlcBufferToBeForwarded.erase (m_rlcBufferToBeForwarded.begin());
    NS_LOG_LOGIC(this << " UE RRC: After forwarding: buffer size = " << m_rlcBufferToBeForwardedSize );
  }
}



void
NrUeRrc::SynchronizeToStrongestCell ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_state == IDLE_CELL_SEARCH);

  uint16_t maxRsrpCellId = 0;
  double maxRsrp = -std::numeric_limits<double>::infinity ();

  std::map<uint16_t, MeasValues>::iterator it;
  for (it = m_storedMeasValues.begin (); it != m_storedMeasValues.end (); it++)
    {
      /*
       * This block attempts to find a cell with strongest RSRP and has not
       * yet been identified as "acceptable cell".
       */
      if (maxRsrp < it->second.rsrp)
        {
          std::set<uint16_t>::const_iterator itCell;
          itCell = m_acceptableCell.find (it->first);
          if (itCell == m_acceptableCell.end ())
            {
              maxRsrpCellId = it->first;
              maxRsrp = it->second.rsrp;
            }
        }
    }

  if (maxRsrpCellId == 0)
    {
      NS_LOG_WARN (this << " Cell search is unable to detect surrounding cell to attach to");
    }
  else
    {
      NS_LOG_LOGIC (this << " cell " << maxRsrpCellId
                         << " is the strongest untried surrounding cell");
      m_cphySapProvider->SynchronizeWithEnb (maxRsrpCellId, m_dlEarfcn);
      SwitchToState (IDLE_WAIT_MIB_SIB1);
    }

} // end of void NrUeRrc::SynchronizeToStrongestCell ()


void
NrUeRrc::EvaluateCellForSelection ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_state == IDLE_WAIT_SIB1);
  NS_ASSERT (m_hasReceivedMib);
  NS_ASSERT (m_hasReceivedSib1);
  uint16_t cellId = m_lastSib1.cellAccessRelatedInfo.cellIdentity;

  // Cell selection criteria evaluation

  bool isSuitableCell = false;
  bool isAcceptableCell = false;
  std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.find (cellId);
  double qRxLevMeas = storedMeasIt->second.rsrp;
  double qRxLevMin = NrEutranMeasurementMapping::IeValue2ActualQRxLevMin (m_lastSib1.cellSelectionInfo.qRxLevMin);
  NS_LOG_LOGIC (this << " cell selection to cellId=" << cellId
                     << " qrxlevmeas=" << qRxLevMeas << " dBm"
                     << " qrxlevmin=" << qRxLevMin << " dBm");

  if (qRxLevMeas - qRxLevMin > 0)
    {
      isAcceptableCell = true;

      uint32_t cellCsgId = m_lastSib1.cellAccessRelatedInfo.csgIdentity;
      bool cellCsgIndication = m_lastSib1.cellAccessRelatedInfo.csgIndication;

      isSuitableCell = (cellCsgIndication == false) || (cellCsgId == m_csgWhiteList);

      NS_LOG_LOGIC (this << " csg(ue/cell/indication)=" << m_csgWhiteList << "/"
                         << cellCsgId << "/" << cellCsgIndication);
    }

  // Cell selection decision

  if (isSuitableCell)
    {
      m_cellId = cellId;
      SwitchLowerLayerProviders(m_cellId); // for InterRat HO
      m_cphySapProvider->SynchronizeWithEnb (cellId, m_dlEarfcn);
      m_cphySapProvider->SetDlBandwidth (m_dlBandwidth);
      m_initialCellSelectionEndOkTrace (m_imsi, cellId);
      SwitchToState (IDLE_CAMPED_NORMALLY);
    }
  else
    {
      // ignore the MIB and SIB1 received from this cell
      m_hasReceivedMib = false;
      m_hasReceivedSib1 = false;

      m_initialCellSelectionEndErrorTrace (m_imsi, cellId);

      if (isAcceptableCell)
        {
          /*
           * The cells inserted into this list will not be considered for
           * subsequent cell search attempt.
           */
          m_acceptableCell.insert (cellId);
        }

      SwitchToState (IDLE_CELL_SEARCH);
      SynchronizeToStrongestCell (); // retry to a different cell
    }

} // end of void NrUeRrc::EvaluateCellForSelection ()

void //NR eNB RRC will run this function
NrUeRrc::DoNotifySecondaryCellHandover (uint16_t oldRnti, uint16_t newRnti, uint16_t mmWaveCellId, NrRrcSap::RadioResourceConfigDedicated rrcd)
{
  NS_ASSERT_MSG(!m_isSecondaryRRC or !m_isThirdRrc, "Trying to modify RLCs in the mmWave RRC (they are instances in the NR RRC)");
  m_mmWaveRnti = newRnti;

  //std::cout << "rnti and cellId " << newRnti << "\t " << mmWaveCellId << std::endl;
  std::list<NrRrcSap::DrbToAddMod>::const_iterator dtamIt; // iterate over the 
  for (dtamIt = rrcd.drbToAddModList.begin ();
       dtamIt != rrcd.drbToAddModList.end ();
       ++dtamIt)
    {
      NS_LOG_INFO (this << " IMSI " << m_imsi << " modifying DRBID " << (uint32_t) dtamIt->drbIdentity << " LC " << (uint32_t) dtamIt->logicalChannelIdentity);
      NS_ASSERT_MSG (dtamIt->logicalChannelIdentity > 2, "LCID value " << dtamIt->logicalChannelIdentity << " is reserved for SRBs");

      std::map<uint8_t, Ptr<NrDataRadioBearerInfo> >::iterator drbMapIt = m_drbMap.find (dtamIt->drbIdentity);
      NS_ASSERT_MSG (drbMapIt != m_drbMap.end(), "Trying to modify an unknown bearer");
        
      Ptr<NrDataRadioBearerInfo> drbInfo = drbMapIt->second;
      NS_LOG_INFO ("Is MC " << dtamIt->is_mc);
      if(dtamIt->is_mc == true || dtamIt->is_mc_2) // we need to modify the RLC for MmWave communications
      {
        Ptr<NrMcUePdcp> pdcp = DynamicCast<NrMcUePdcp> (drbInfo->m_pdcp);
        if(pdcp !=0)
        {
          // create Rlc
          TypeId rlcTypeId;
          if (m_useRlcSm)
            {
              rlcTypeId = NrRlcSm::GetTypeId ();
              NS_LOG_INFO("SM");
            }
          else
            {
              switch (dtamIt->rlcConfig.choice)
                {
                case NrRrcSap::RlcConfig::AM: 
                  rlcTypeId = NrRlcAm::GetTypeId ();
                  NS_LOG_INFO("AM");
                  break;
          
                case NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL: 
                  rlcTypeId = NrRlcUm::GetTypeId ();
                  NS_LOG_INFO("UM");
                  break;
          
                default:
                  NS_FATAL_ERROR ("unsupported RLC configuration");
                  break;                
                }
            }
  std::cout << "sjkang0709 DoNotifySecondaryCellHandover ----> " <<dtamIt->is_mc << "\t "<<dtamIt->is_mc_2 << std::endl;
          ObjectFactory rlcObjectFactory;
          rlcObjectFactory.SetTypeId (rlcTypeId);
          Ptr<NrRlc> rlc = rlcObjectFactory.Create ()->GetObject<NrRlc> ();
          std::ostringstream fileName;
          Ptr<NrRlc> rlc_p;

          if (dtamIt->is_mc){
        	  rlc->SetNrMacSapProvider (m_mmWaveMacSapProvider);
        	  fileName << "rlc_Tput_senb2_ue"<<m_imsi<<"_bearer_"<< (uint16_t)(dtamIt->drbIdentity) << ".txt";
        	  rlc_p =  m_rlcMap[dtamIt->drbIdentity]->m_rlc_2; // will store the rlc layers and lower layers of 73G UE Stack
        	  NS_ASSERT_MSG(oldRnti == m_mmWaveRnti_28, "Wrong RNTI! - unknown device");
        	  m_mmWaveCellId_28G = mmWaveCellId;
        	  m_mmWaveRnti_28 = m_mmWaveRnti;
        	  m_mmWaveCellId = m_mmWaveCellId_28G;
        		   rlc->SetRnti (m_mmWaveRnti_28);

          }
          else if (dtamIt->is_mc_2){
        	  rlc->SetNrMacSapProvider (m_mmWaveMacSapProvider_2);

        	  fileName << "rlc_Tput_senb1_ue"<<m_imsi<<"_bearer_"<< (uint16_t)(dtamIt->drbIdentity) << ".txt";
        	  rlc_p =  m_rlcMap[dtamIt->drbIdentity]->m_rlc; // will store the rlc layers and lower layers of 28G UE Stack
        	//  std::cout << oldRnti << "\t" << m_mmWaveRnti_73 << std::endl;
        	  NS_ASSERT_MSG(oldRnti == m_mmWaveRnti_73, "Wrong RNTI! - unknown device");
        	  m_mmWaveCellId_73G = mmWaveCellId;
        	  m_mmWaveRnti_73 = m_mmWaveRnti;
        	  m_mmWaveCellId = m_mmWaveCellId_73G;
         	   rlc->SetRnti (m_mmWaveRnti_73);
          }

          rlc->SetLcId (dtamIt->logicalChannelIdentity);


          	  //fileName<<"UE-"<<m_imsi<<"-Path-0_"<<"Bearer-"<< (uint16_t)(dtamIt->drbIdentity) << "-RLC-Throughput.txt";
         // if (dtamIt ->is_mc)
          //   fileName << "rlc_Tput_senb2_ue"<<m_imsi<<"_bearer_"<< (uint16_t)(dtamIt->drbIdentity) << ".txt";
         // else
        	//  fileName << "rlc_Tput_senb1_ue"<<m_imsi<<"_bearer_"<< (uint16_t)(dtamIt->drbIdentity) << ".txt";

          	  std::ofstream* streamPathThroughput = new std::ofstream(fileName.str().c_str(), std::ofstream::app);
              rlc->CalculatePathThroughput(streamPathThroughput);

          //  Ptr<NrRlc> rlc_2 = rlcObjectFactory.Create ()->GetObject<NrRlc> ();


           // rlc_2->SetNrMacSapProvider (m_mmWaveMacSapProvider_2);
            //rlc_2->SetRnti (m_mmWaveRnti);
            //rlc_2->SetLcId (dtamIt->logicalChannelIdentity);

             //std::ostringstream fileName_2;
             //fileName_2<<"UE-"<<m_imsi<<"-Path-1_"<<"Bearer-"<< (uint16_t)(dtamIt->drbIdentity) << "-RLC-Throughput.txt";
             //fileName_2 <<"rlc_Tput_senb1_ue"<<m_imsi<<"_bearer_"<< (uint16_t)(dtamIt->drbIdentity) << ".txt";
           //  std::ofstream* streamPathThroughput_2 = new std::ofstream(fileName_2.str().c_str(),std::ofstream::app);
            // rlc_2->CalculatePathThroughput(streamPathThroughput_2);

          struct NrUeCmacSapProvider::LogicalChannelConfig lcConfig;
          lcConfig.priority = dtamIt->logicalChannelConfig.priority;
          lcConfig.prioritizedBitRateKbps = dtamIt->logicalChannelConfig.prioritizedBitRateKbps;
          lcConfig.bucketSizeDurationMs = dtamIt->logicalChannelConfig.bucketSizeDurationMs;
          lcConfig.logicalChannelGroup = dtamIt->logicalChannelConfig.logicalChannelGroup;


           //m_mmWaveCmacSapProvider_2->AddLc(dtamIt->logicalChannelIdentity,
        	//	   lcConfig, rlc_2->GetNrMacSapUser());
          if (dtamIt->is_mc)
           m_mmWaveCmacSapProvider->AddLc (dtamIt->logicalChannelIdentity,
        	                                  lcConfig,
        	                                 rlc->GetNrMacSapUser ());
          else if(dtamIt->is_mc_2)
        	  m_mmWaveCmacSapProvider_2->AddLc (dtamIt->logicalChannelIdentity,
        	          	                                  lcConfig,
        	          	                                 rlc->GetNrMacSapUser ());


          if (rlcTypeId != NrRlcSm::GetTypeId ())
          {
            pdcp->SetMmWaveRnti (m_mmWaveRnti_73, m_mmWaveCellId_28G);
            //	rlc_2->SetNrRlcSapUser(pdcp->GetNrRlcSapUser());
            //	pdcp->SetMmWaveRlcSapProvider(rlc_2->GetNrRlcSapProvider());
            	   if (dtamIt->is_mc)
            		pdcp->SetMmWaveRlcSapProvider (rlc->GetNrRlcSapProvider ());
            	   else if (dtamIt->is_mc_2)
            		  pdcp->SetMmWaveRlcSapProvider_2(rlc->GetNrRlcSapProvider());

            	    rlc->SetNrRlcSapUser (pdcp->GetNrRlcSapUser ());
            	 //   rlc_p->SetNrRlcSapUser(pdcp->GetNrRlcSapUser());


          } 
          rlc->Initialize();
          	 //rlc_2 ->Initialize();

          Ptr<RlcBearerInfo> rlcInfo = CreateObject<RlcBearerInfo>();
          if (dtamIt->is_mc){
        	  rlcInfo->m_rlc_2 = rlc_p;
        	  rlcInfo->m_rlc = rlc;
        //	  pdcp->SetMmWaveRlcSapProvider_2(rlc_p->GetNrRlcSapProvider());
          }
          else if (dtamIt->is_mc_2){
           rlcInfo->m_rlc = rlc_p;
           rlcInfo->m_rlc_2 = rlc;
       //    pdcp->SetMmWaveRlcSapProvider(rlc_p->GetNrRlcSapProvider());
          }
          rlcInfo->rlcConfig.choice = dtamIt->rlcConfig.choice;
          rlcInfo->logicalChannelIdentity = dtamIt->logicalChannelIdentity;
          rlcInfo->mmWaveRnti = m_mmWaveRnti_28;
          	rlcInfo->mmWaveRnti_73 = m_mmWaveRnti_73;

          NrRrcSap::LogicalChannelConfig logicalChannelConfig;
          logicalChannelConfig.priority = dtamIt->logicalChannelConfig.priority;
          logicalChannelConfig.prioritizedBitRateKbps = dtamIt->logicalChannelConfig.prioritizedBitRateKbps;
          logicalChannelConfig.bucketSizeDurationMs = dtamIt->logicalChannelConfig.bucketSizeDurationMs;
          logicalChannelConfig.logicalChannelGroup = dtamIt->logicalChannelConfig.logicalChannelGroup;  

          rlcInfo->logicalChannelConfig = logicalChannelConfig;

          // check if this rlc is already in the map
          if(m_rlcMap.find(dtamIt->drbIdentity) != m_rlcMap.end())
          {
            // get the RLC buffer content and store it in this RLC
        	  if (dtamIt->is_mc)
            CopyRlcBuffers(m_rlcMap.find(dtamIt->drbIdentity)->second->m_rlc, drbInfo->m_pdcp, drbInfo->m_logicalChannelIdentity);
        	  else if (dtamIt->is_mc_2)
            CopyRlcBuffers(m_rlcMap.find(dtamIt->drbIdentity)->second->m_rlc_2, drbInfo->m_pdcp, drbInfo->m_logicalChannelIdentity);



          } 
          m_rlcMap[dtamIt->drbIdentity] = rlcInfo;
          
          NS_LOG_INFO(this << "  NrUeRrc Secondary Cell Handover, created new rlc " << m_imsi <<m_mmWaveCellId_28G<<"\t"
        		  << m_mmWaveCellId_73G<<"\t" << m_mmWaveRnti << " at time " << Simulator::Now().GetSeconds());
        }
        else
        {
          NS_FATAL_ERROR("MC setup on a non MC-capable bearer");
        }
      }
    }
}

void 
NrUeRrc::ApplyRadioResourceConfigDedicated (NrRrcSap::RadioResourceConfigDedicated rrcd)
{
  NS_LOG_FUNCTION (this);

  const struct NrRrcSap::PhysicalConfigDedicated& pcd = rrcd.physicalConfigDedicated;
 if (pcd.haveAntennaInfoDedicated)
    {
      m_cphySapProvider->SetTransmissionMode (pcd.antennaInfo.transmissionMode);
    }
  if (pcd.haveSoundingRsUlConfigDedicated)
    {
      m_cphySapProvider->SetSrsConfigurationIndex (pcd.soundingRsUlConfigDedicated.srsConfigIndex);
    }

  if (pcd.havePdschConfigDedicated)
    {
      // update PdschConfigDedicated (i.e. P_A value)
      m_pdschConfigDedicated = pcd.pdschConfigDedicated;
      double paDouble = NrRrcSap::ConvertPdschConfigDedicated2Double (m_pdschConfigDedicated);
      m_cphySapProvider->SetPa (paDouble);
    }

  std::list<NrRrcSap::SrbToAddMod>::const_iterator stamIt = rrcd.srbToAddModList.begin ();
  if (stamIt != rrcd.srbToAddModList.end ())
    {
      if (m_srb1 == 0)
        {
          NS_LOG_INFO("Setup SBR1 for rnti " << m_rnti << " on cell " << m_cellId);
          // SRB1 not setup yet        

          NS_ASSERT_MSG ((m_state == IDLE_CONNECTING) || (m_state == CONNECTED_HANDOVER), 
                         "unexpected state " << ToString (m_state));
          NS_ASSERT_MSG (stamIt->srbIdentity == 1, "only SRB1 supported");

          const uint8_t lcid = 1; // fixed LCID for SRB1

          Ptr<NrRlc> rlc = CreateObject<NrRlcAm> ();
          rlc->SetNrMacSapProvider (m_macSapProvider);

          rlc->SetRnti (m_rnti);
          rlc->SetLcId (lcid);      

          Ptr<NrPdcp> pdcp = CreateObject<NrPdcp> ();
          pdcp->SetRnti (m_rnti);
          pdcp->SetLcId (lcid);
          pdcp->SetNrPdcpSapUser (m_drbPdcpSapUser);
          pdcp->SetNrRlcSapProvider (rlc->GetNrRlcSapProvider ());
          rlc->SetNrRlcSapUser (pdcp->GetNrRlcSapUser ());

          m_srb1 = CreateObject<NrSignalingRadioBearerInfo> ();
          m_srb1->m_rlc = rlc;
          m_srb1->m_pdcp = pdcp;
          m_srb1->m_srbIdentity = 1;
          
          m_srb1->m_logicalChannelConfig.priority = stamIt->logicalChannelConfig.priority;
          m_srb1->m_logicalChannelConfig.prioritizedBitRateKbps = stamIt->logicalChannelConfig.prioritizedBitRateKbps;
          m_srb1->m_logicalChannelConfig.bucketSizeDurationMs = stamIt->logicalChannelConfig.bucketSizeDurationMs;
          m_srb1->m_logicalChannelConfig.logicalChannelGroup = stamIt->logicalChannelConfig.logicalChannelGroup;

          struct NrUeCmacSapProvider::LogicalChannelConfig lcConfig;
          lcConfig.priority = stamIt->logicalChannelConfig.priority;
          lcConfig.prioritizedBitRateKbps = stamIt->logicalChannelConfig.prioritizedBitRateKbps;
          lcConfig.bucketSizeDurationMs = stamIt->logicalChannelConfig.bucketSizeDurationMs;
          lcConfig.logicalChannelGroup = stamIt->logicalChannelConfig.logicalChannelGroup;
      
          m_cmacSapProvider->AddLc (lcid, lcConfig, rlc->GetNrMacSapUser ());
      
          ++stamIt;
          NS_ASSERT_MSG (stamIt == rrcd.srbToAddModList.end (), "at most one SrbToAdd supported");     
          
          NrUeRrcSapUser::SetupParameters ueParams;
          ueParams.srb0SapProvider = m_srb0->m_rlc->GetNrRlcSapProvider ();
          ueParams.srb1SapProvider = m_srb1->m_pdcp->GetNrPdcpSapProvider ();
          m_rrcSapUser->Setup (ueParams);
        }
      else
        {
          NS_LOG_INFO ("request to modify SRB1 (skipping as currently not implemented)");
          // would need to modify m_srb1, and then propagate changes to the MAC
        }
    }

  std::list<NrRrcSap::DrbToAddMod>::const_iterator dtamIt;
  for (dtamIt = rrcd.drbToAddModList.begin ();
       dtamIt != rrcd.drbToAddModList.end ();
       ++dtamIt)
    {
      NS_LOG_UNCOND (this << " IMSI " << m_imsi << " adding/modifying DRBID " << (uint32_t) dtamIt->drbIdentity << " LC " << (uint32_t) dtamIt->logicalChannelIdentity);
      NS_ASSERT_MSG (dtamIt->logicalChannelIdentity > 2, "LCID value " << dtamIt->logicalChannelIdentity << " is reserved for SRBs");

      std::map<uint8_t, Ptr<NrDataRadioBearerInfo> >::iterator drbMapIt = m_drbMap.begin();
      for(; drbMapIt != m_drbMap.end(); ++drbMapIt)
      {
        NS_LOG_DEBUG("In m_drbMap " << (uint16_t)drbMapIt->first);
      }


      //std::map<uint8_t, Ptr<NrDataRadioBearerInfo> >::iterator 
      drbMapIt = m_drbMap.find (dtamIt->drbIdentity);

      if (drbMapIt == m_drbMap.end ())
        {
          NS_LOG_UNCOND ("New Data Radio Bearer");

          TypeId rlcTypeId;
          if (m_useRlcSm)
            {
              rlcTypeId = NrRlcSm::GetTypeId ();
            }
          else
            {
              switch (dtamIt->rlcConfig.choice)
                {
                case NrRrcSap::RlcConfig::AM: 
                  rlcTypeId = NrRlcAm::GetTypeId ();
                  break;
          
                case NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL: 
                  rlcTypeId = NrRlcUm::GetTypeId ();
                  break;
          
                default:
                  NS_FATAL_ERROR ("unsupported RLC configuration");
                  break;                
                }
            }

          ObjectFactory rlcObjectFactory;
          rlcObjectFactory.SetTypeId (rlcTypeId);
          Ptr<NrRlc> rlc = rlcObjectFactory.Create ()->GetObject<NrRlc> ();
          rlc->SetNrMacSapProvider (m_macSapProvider); // TODO interRatHoCapable 
          rlc->SetRnti (m_rnti);
          rlc->SetLcId (dtamIt->logicalChannelIdentity);


          Ptr<NrDataRadioBearerInfo> drbInfo = CreateObject<NrDataRadioBearerInfo> ();
          drbInfo->m_rlc = rlc;
          drbInfo->m_epsBearerIdentity = dtamIt->epsBearerIdentity;
          drbInfo->m_logicalChannelIdentity = dtamIt->logicalChannelIdentity;
          drbInfo->m_drbIdentity = dtamIt->drbIdentity;
          drbInfo->m_rlcConfig.choice = dtamIt->rlcConfig.choice;
 
          // we need PDCP only for real RLC, i.e., RLC/UM or RLC/AM
          // if we are using RLC/SM we don't care of anything above RLC
          if (rlcTypeId != NrRlcSm::GetTypeId ())
            {
              Ptr<NrMcUePdcp> pdcp = CreateObject<NrMcUePdcp> (); 
              pdcp->SetRnti (m_rnti);
              pdcp->SetLcId (dtamIt->logicalChannelIdentity);
              pdcp->SetNrPdcpSapUser (m_drbPdcpSapUser);
              pdcp->SetNrRlcSapProvider (rlc->GetNrRlcSapProvider ());
              rlc->SetNrRlcSapUser (pdcp->GetNrRlcSapUser ());
              drbInfo->m_pdcp = pdcp;
            }

          m_bid2DrbidMap[dtamIt->epsBearerIdentity] = dtamIt->drbIdentity;

          struct NrRrcSap::LogicalChannelConfig lcConfigToStore;
          lcConfigToStore.priority = dtamIt->logicalChannelConfig.priority;
          lcConfigToStore.prioritizedBitRateKbps = dtamIt->logicalChannelConfig.prioritizedBitRateKbps;
          lcConfigToStore.bucketSizeDurationMs = dtamIt->logicalChannelConfig.bucketSizeDurationMs;
          lcConfigToStore.logicalChannelGroup = dtamIt->logicalChannelConfig.logicalChannelGroup;
          drbInfo->m_logicalChannelConfig = lcConfigToStore;
  
          m_drbMap.insert (std::pair<uint8_t, Ptr<NrDataRadioBearerInfo> > (dtamIt->drbIdentity, drbInfo));
  

          struct NrUeCmacSapProvider::LogicalChannelConfig lcConfig;
          lcConfig.priority = dtamIt->logicalChannelConfig.priority;
          lcConfig.prioritizedBitRateKbps = dtamIt->logicalChannelConfig.prioritizedBitRateKbps;
          lcConfig.bucketSizeDurationMs = dtamIt->logicalChannelConfig.bucketSizeDurationMs;
          lcConfig.logicalChannelGroup = dtamIt->logicalChannelConfig.logicalChannelGroup;

          // TODO check which for interRatHoCapable
          m_cmacSapProvider->AddLc (dtamIt->logicalChannelIdentity,
                                    lcConfig,
                                    rlc->GetNrMacSapUser ());
          std::ostringstream fileName_0;
                    //  fileName_0<<"UE-"<<m_imsi<<"-NR-" << "Bearer-"<< (uint16_t)(dtamIt->drbIdentity) << "-RLC-Throughput.txt";
          fileName_0<<"rlc_Tput_menb_ue"<<m_imsi<<"_bearer_"<< (uint16_t)(dtamIt->drbIdentity) << ".txt";
                      std::ofstream* streamPathThroughput_0 = new std::ofstream(fileName_0.str().c_str());
                     // *streamPathThroughput_0<<"time " <<"\t " <<"Instant Tput " << "\t " <<"Avg Tput "<< std::endl;
                      rlc->CalculatePathThroughput(streamPathThroughput_0);

          rlc->Initialize ();
        }
      else
        {
          NS_LOG_UNCOND("request to modify existing DRBID");
          Ptr<NrDataRadioBearerInfo> drbInfo = drbMapIt->second;
          NS_LOG_INFO ("Is MC " << dtamIt->is_mc);
          if(dtamIt->is_mc == true or dtamIt->is_mc_2 ==true) // we need to setup the RLC for MmWave communications  //sjkang1109 i gues nr drb already setup
          {
            Ptr<NrMcUePdcp> pdcp = DynamicCast<NrMcUePdcp> (drbInfo->m_pdcp);
            if(pdcp !=0)
            {
              // create Rlc
              TypeId rlcTypeId;
              if (m_useRlcSm)
                {
                  rlcTypeId = NrRlcSm::GetTypeId ();
                  NS_LOG_INFO("SM");
                }
              else
                {
                  switch (dtamIt->rlcConfig.choice)
                    {
                    case NrRrcSap::RlcConfig::AM: 
                      rlcTypeId = NrRlcAm::GetTypeId ();
                      NS_LOG_INFO("AM");
                      break;
              
                    case NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL: 
                      rlcTypeId = NrRlcUm::GetTypeId ();
                      NS_LOG_INFO("UM");
                      break;
              
                    default:
                      NS_FATAL_ERROR ("unsupported RLC configuration");
                      break;                
                    }
                }
      
              ObjectFactory rlcObjectFactory;
              rlcObjectFactory.SetTypeId (rlcTypeId);
           //   rlcObjectFactory_2.SetTypeId (rlcTypeId);
               std::ostringstream fileName;
            //  fileName<<"UE-"<<m_imsi<<"-Path-0_" << "Bearer-"<< (uint16_t)(dtamIt->drbIdentity) << "-RLC-Throughput.txt";
            fileName << "rlc_Tput_senb2_ue"<<m_imsi<<"_bearer_"<< (uint16_t)(dtamIt->drbIdentity) << ".txt";

              std::ofstream* streamPathThroughput_1 = new std::ofstream(fileName.str().c_str());
            //  *streamPathThroughput_1<<"time " <<"\t " <<"Instant Tput " << "\t " <<"Avg Tput "<< std::endl;

              Ptr<NrRlc> rlc = rlcObjectFactory.Create ()->GetObject<NrRlc> ();
              rlc->SetNrMacSapProvider (m_mmWaveMacSapProvider); 
              rlc->SetRnti (m_mmWaveRnti_28);
              rlc->SetLcId (dtamIt->logicalChannelIdentity);
              //if(dtamIt->drbIdentity >=2)
              rlc->CalculatePathThroughput(streamPathThroughput_1); //sjkang1113

              //  Simulator::Schedule(MilliSeconds(100),&NrRlcUm::)
              std::ostringstream fileName_2;
             // fileName_2<<"UE-"<<m_imsi<<"-Path-1_"<<"Bearer-"<< (uint16_t)(dtamIt->drbIdentity) << "-RLC-Throughput.txt";
            fileName_2 <<"rlc_Tput_senb1_ue"<<m_imsi<<"_bearer_"<< (uint16_t)(dtamIt->drbIdentity) << ".txt";

              std::ofstream* streamPathThroughput_2 = new std::ofstream(fileName_2.str().c_str());
             // *streamPathThroughput_2<<"time " <<"\t " <<"Instant Tput " << "\t " <<"Avg Tput"<<std::endl;

              ////sjkang1110
              Ptr<NrRlc> rlc_2 = rlcObjectFactory.Create ()->GetObject<NrRlc> ();
                rlc_2->SetNrMacSapProvider (m_mmWaveMacSapProvider_2); //sjkang1110
                rlc_2->SetRnti (m_mmWaveRnti_73);//sjkang1110
                rlc_2->SetLcId (dtamIt->logicalChannelIdentity);//sjkang1110
                //if(dtamIt->drbIdentity >=2)
                rlc_2->CalculatePathThroughput(streamPathThroughput_2); //sjkang1113

                std::ostringstream fileName_3;
                fileName_3<<"UE-"<<m_imsi<<"-Bearer-"<< (uint16_t)(dtamIt->drbIdentity) << "-PDCP-Throughput.txt";
                std::ofstream* streamPathThroughput_3 = new std::ofstream(fileName_3.str().c_str());
                 pdcp->CalculatePdcpThroughput(streamPathThroughput_3); //sjkang1113

                std::ostringstream SN_diff_fileName; //sjkang1116
                SN_diff_fileName << "UE-"<<m_imsi << "-Bearer-"<< (uint16_t)(dtamIt)->drbIdentity << "-SN_Difference.txt"; //sjkang1116
                std::ofstream* SN_differenceStream = new std::ofstream(SN_diff_fileName.str().c_str()); //sjkang1116
                pdcp->SetStreams(SN_differenceStream);//sjkang1116

              struct NrUeCmacSapProvider::LogicalChannelConfig lcConfig;
              lcConfig.priority = dtamIt->logicalChannelConfig.priority;
              lcConfig.prioritizedBitRateKbps = dtamIt->logicalChannelConfig.prioritizedBitRateKbps;
              lcConfig.bucketSizeDurationMs = dtamIt->logicalChannelConfig.bucketSizeDurationMs;
              lcConfig.logicalChannelGroup = dtamIt->logicalChannelConfig.logicalChannelGroup;      


              m_mmWaveCmacSapProvider->AddLc (dtamIt->logicalChannelIdentity, 
                                      lcConfig,
                                      rlc->GetNrMacSapUser ());

              m_mmWaveCmacSapProvider_2->AddLc (dtamIt->logicalChannelIdentity,
                                      lcConfig,
                                      rlc_2->GetNrMacSapUser ()); //sjkang1109

              if (rlcTypeId != NrRlcSm::GetTypeId ())
              {
                pdcp->SetMmWaveRnti (m_mmWaveRnti_73, m_mmWaveCellId_28G);
                pdcp->SetMmWaveRlcSapProvider (rlc->GetNrRlcSapProvider ());
                pdcp->SetMmWaveRlcSapProvider_2 (rlc_2->GetNrRlcSapProvider ());//sjkang1110

                rlc->SetNrRlcSapUser (pdcp->GetNrRlcSapUser ());
                rlc_2->SetNrRlcSapUser (pdcp->GetNrRlcSapUser ()); //sjkang1110
              } 
              rlc->Initialize();
              rlc_2->Initialize();//sjkang1110
              Ptr<RlcBearerInfo> rlcInfo = CreateObject<RlcBearerInfo>();
              rlcInfo->m_rlc = rlc;
              rlcInfo->m_rlc_2 = rlc_2;//sjkang1110
              rlcInfo->rlcConfig.choice = dtamIt->rlcConfig.choice;
              rlcInfo->logicalChannelIdentity = dtamIt->logicalChannelIdentity;
              rlcInfo->mmWaveRnti = m_mmWaveRnti_28;
              rlcInfo->mmWaveRnti_73 = m_mmWaveCellId_73G;

              NrRrcSap::LogicalChannelConfig logicalChannelConfig;
              logicalChannelConfig.priority = dtamIt->logicalChannelConfig.priority;
              logicalChannelConfig.prioritizedBitRateKbps = dtamIt->logicalChannelConfig.prioritizedBitRateKbps;
              logicalChannelConfig.bucketSizeDurationMs = dtamIt->logicalChannelConfig.bucketSizeDurationMs;
              logicalChannelConfig.logicalChannelGroup = dtamIt->logicalChannelConfig.logicalChannelGroup;  

              rlcInfo->logicalChannelConfig = logicalChannelConfig;

              // switch to the mmWave RLC
              bool useMmWaveConnection = true;
              pdcp->SwitchConnection(useMmWaveConnection);
              //std::cout << this << " Ue rrc will switch connection to Mmwave-----------> " << m_imsi << "\t" << useMmWaveConnection<<std::endl;
              NS_LOG_INFO("NrUeRrc SwitchToMmWave and create new rlc " << m_imsi << m_mmWaveCellId << m_mmWaveRnti << " at time " << Simulator::Now().GetSeconds());
              m_switchToMmWaveTrace(m_imsi, m_mmWaveCellId, m_mmWaveRnti);

              // check if this rlc is already in the map
              if(m_rlcMap.find(dtamIt->drbIdentity) != m_rlcMap.end())
              {
                // get the RLC buffer content and store it in this RLC
                CopyRlcBuffers(m_rlcMap.find(dtamIt->drbIdentity)->second->m_rlc, drbInfo->m_pdcp, drbInfo->m_logicalChannelIdentity);
                CopyRlcBuffers(m_rlcMap.find(dtamIt->drbIdentity)->second->m_rlc_2, drbInfo->m_pdcp, drbInfo->m_logicalChannelIdentity); //sjkang1110

              } 
              m_rlcMap[dtamIt->drbIdentity] = rlcInfo;  /// this is finall setting sjkang1109
            }
            else
            {
              NS_FATAL_ERROR("MC setup on a non MC-capable bearer");
            }

          }
          else
          {
            NS_LOG_INFO ("Modify Data Radio Bearer : not implemented");
            // TypeId rlcTypeId;
            // if (m_useRlcSm)
            //   {
            //     rlcTypeId = NrRlcSm::GetTypeId ();
            //   }
            // else
            //   {
            //     switch (dtamIt->rlcConfig.choice)
            //       {
            //       case NrRrcSap::RlcConfig::AM: 
            //         rlcTypeId = NrRlcAm::GetTypeId ();
            //         break;
            
            //       case NrRrcSap::RlcConfig::UM_BI_DIRECTIONAL: 
            //         rlcTypeId = NrRlcUm::GetTypeId ();
            //         break;
            
            //       default:
            //         NS_FATAL_ERROR ("unsupported RLC configuration");
            //         break;                
            //       }
            //   }

            // ObjectFactory rlcObjectFactory;
            // rlcObjectFactory.SetTypeId (rlcTypeId);
            // Ptr<NrRlc> rlc = rlcObjectFactory.Create ()->GetObject<NrRlc> ();
            // rlc->SetNrMacSapProvider (m_macSapProvider);
            // rlc->SetRnti (m_rnti);
            // rlc->SetLcId (dtamIt->logicalChannelIdentity);

            // Ptr<NrDataRadioBearerInfo> drbInfo = CreateObject<NrDataRadioBearerInfo> ();
            // drbInfo->m_rlc = rlc;
            // drbInfo->m_epsBearerIdentity = dtamIt->epsBearerIdentity;
            // drbInfo->m_logicalChannelIdentity = dtamIt->logicalChannelIdentity;
            // drbInfo->m_drbIdentity = dtamIt->drbIdentity;
            // drbInfo->m_rlcConfig.choice = dtamIt->rlcConfig.choice;
   
            // // we need PDCP only for real RLC, i.e., RLC/UM or RLC/AM
            // // if we are using RLC/SM we don't care of anything above RLC
            // if (rlcTypeId != NrRlcSm::GetTypeId ())
            //   {
            //     Ptr<NrPdcp> pdcp = CreateObject<NrPdcp> (); 
            //     pdcp->SetRnti (m_rnti);
            //     pdcp->SetLcId (dtamIt->logicalChannelIdentity);
            //     pdcp->SetNrPdcpSapUser (m_drbPdcpSapUser);
            //     pdcp->SetNrRlcSapProvider (rlc->GetNrRlcSapProvider ());
            //     rlc->SetNrRlcSapUser (pdcp->GetNrRlcSapUser ());
            //     drbInfo->m_pdcp = pdcp;
            //   }

            // m_bid2DrbidMap[dtamIt->epsBearerIdentity] = dtamIt->drbIdentity;

            // struct NrRrcSap::LogicalChannelConfig lcConfigToStore;
            // lcConfigToStore.priority = dtamIt->logicalChannelConfig.priority;
            // lcConfigToStore.prioritizedBitRateKbps = dtamIt->logicalChannelConfig.prioritizedBitRateKbps;
            // lcConfigToStore.bucketSizeDurationMs = dtamIt->logicalChannelConfig.bucketSizeDurationMs;
            // lcConfigToStore.logicalChannelGroup = dtamIt->logicalChannelConfig.logicalChannelGroup;
            // drbInfo->m_logicalChannelConfig = lcConfigToStore;    

            // struct NrUeCmacSapProvider::LogicalChannelConfig lcConfig;
            // lcConfig.priority = dtamIt->logicalChannelConfig.priority;
            // lcConfig.prioritizedBitRateKbps = dtamIt->logicalChannelConfig.prioritizedBitRateKbps;
            // lcConfig.bucketSizeDurationMs = dtamIt->logicalChannelConfig.bucketSizeDurationMs;
            // lcConfig.logicalChannelGroup = dtamIt->logicalChannelConfig.logicalChannelGroup;

            // rlc->Initialize ();    

            // // check if an RLC for this drbid is already in the map
            // if(m_drbMap.find(dtamIt->drbIdentity) != m_drbMap.end())
            // {
            //   // get the RLC buffer content and store it in this RLC
            //   CopyRlcBuffers(m_drbMap.find(dtamIt->drbIdentity)->second->m_rlc, drbInfo->m_pdcp, drbInfo->m_logicalChannelIdentity);
            // } 
            // m_drbMap[dtamIt->drbIdentity] = drbInfo;

            // m_cmacSapProvider->AddLc (dtamIt->logicalChannelIdentity,
            //                           lcConfig,
            //                           rlc->GetNrMacSapUser ());

          }
        }
    }
  
  std::list<uint8_t>::iterator dtdmIt;
  for (dtdmIt = rrcd.drbToReleaseList.begin ();
       dtdmIt != rrcd.drbToReleaseList.end ();
       ++dtdmIt)
    {
      uint8_t drbid = *dtdmIt;
      NS_LOG_INFO (this << " IMSI " << m_imsi << " releasing DRB " << (uint32_t) drbid << drbid);
      std::map<uint8_t, Ptr<NrDataRadioBearerInfo> >::iterator it =   m_drbMap.find (drbid);
      NS_ASSERT_MSG (it != m_drbMap.end (), "could not find bearer with given lcid");
      m_drbMap.erase (it);      
      m_bid2DrbidMap.erase (drbid);
      //Remove LCID
      m_cmacSapProvider->RemoveLc (drbid + 2);

    }
}


void 
NrUeRrc::ApplyMeasConfig (NrRrcSap::MeasConfig mc)
{
  NS_LOG_FUNCTION (this);

  // perform the actions specified in 3GPP TS 36.331 section 5.5.2.1 

  // 3GPP TS 36.331 section 5.5.2.4 Measurement object removal
  for (std::list<uint8_t>::iterator it = mc.measObjectToRemoveList.begin ();
       it !=  mc.measObjectToRemoveList.end ();
       ++it)
    {
      uint8_t measObjectId = *it;
      NS_LOG_LOGIC (this << " deleting measObjectId " << (uint32_t)  measObjectId);
      m_varMeasConfig.measObjectList.erase (measObjectId);
      std::map<uint8_t, NrRrcSap::MeasIdToAddMod>::iterator measIdIt = m_varMeasConfig.measIdList.begin ();
      while (measIdIt != m_varMeasConfig.measIdList.end ())
        {
          if (measIdIt->second.measObjectId == measObjectId)
            {
              uint8_t measId = measIdIt->second.measId;
              NS_ASSERT (measId == measIdIt->first);
              NS_LOG_LOGIC (this << " deleting measId " << (uint32_t) measId << " because referring to measObjectId " << (uint32_t)  measObjectId);
              // note: postfix operator preserves iterator validity
              m_varMeasConfig.measIdList.erase (measIdIt++);
              VarMeasReportListClear (measId);
            }
          else
            {
              ++measIdIt;
            }
        }

    }

  // 3GPP TS 36.331 section 5.5.2.5  Measurement object addition/ modification
  for (std::list<NrRrcSap::MeasObjectToAddMod>::iterator it = mc.measObjectToAddModList.begin ();
       it !=  mc.measObjectToAddModList.end ();
       ++it)
    {
      // simplifying assumptions
      NS_ASSERT_MSG (it->measObjectEutra.cellsToRemoveList.empty (), "cellsToRemoveList not supported");
      NS_ASSERT_MSG (it->measObjectEutra.cellsToAddModList.empty (), "cellsToAddModList not supported");
      NS_ASSERT_MSG (it->measObjectEutra.cellsToRemoveList.empty (), "blackCellsToRemoveList not supported");
      NS_ASSERT_MSG (it->measObjectEutra.blackCellsToAddModList.empty (), "blackCellsToAddModList not supported");
      NS_ASSERT_MSG (it->measObjectEutra.haveCellForWhichToReportCGI == false, "cellForWhichToReportCGI is not supported");

      uint8_t measObjectId = it->measObjectId;
      std::map<uint8_t, NrRrcSap::MeasObjectToAddMod>::iterator measObjectIt = m_varMeasConfig.measObjectList.find (measObjectId);
      if (measObjectIt != m_varMeasConfig.measObjectList.end ())
        {
          NS_LOG_LOGIC ("measObjectId " << (uint32_t) measObjectId << " exists, updating entry");
          measObjectIt->second = *it;
          for (std::map<uint8_t, NrRrcSap::MeasIdToAddMod>::iterator measIdIt 
                 = m_varMeasConfig.measIdList.begin ();
               measIdIt != m_varMeasConfig.measIdList.end ();
               ++measIdIt)
            {
              if (measIdIt->second.measObjectId == measObjectId)
                {
                  uint8_t measId = measIdIt->second.measId;
                  NS_LOG_LOGIC (this << " found measId " << (uint32_t) measId << " referring to measObjectId " << (uint32_t)  measObjectId);
                  VarMeasReportListClear (measId);
                }
            }
        }
      else
        {
          NS_LOG_LOGIC ("measObjectId " << (uint32_t) measObjectId << " is new, adding entry");
          m_varMeasConfig.measObjectList[measObjectId] = *it;
        }

    }

  // 3GPP TS 36.331 section 5.5.2.6 Reporting configuration removal
  for (std::list<uint8_t>::iterator it = mc.reportConfigToRemoveList.begin ();
       it !=  mc.reportConfigToRemoveList.end ();
       ++it)
    {
      uint8_t reportConfigId = *it;
      NS_LOG_LOGIC (this << " deleting reportConfigId " << (uint32_t)  reportConfigId);
      m_varMeasConfig.reportConfigList.erase (reportConfigId);
      std::map<uint8_t, NrRrcSap::MeasIdToAddMod>::iterator measIdIt = m_varMeasConfig.measIdList.begin ();
      while (measIdIt != m_varMeasConfig.measIdList.end ())
        {
          if (measIdIt->second.reportConfigId == reportConfigId)
            {
              uint8_t measId = measIdIt->second.measId;
              NS_ASSERT (measId == measIdIt->first);
              NS_LOG_LOGIC (this << " deleting measId " << (uint32_t) measId << " because referring to reportConfigId " << (uint32_t)  reportConfigId);
              // note: postfix operator preserves iterator validity
              m_varMeasConfig.measIdList.erase (measIdIt++);
              VarMeasReportListClear (measId);
            }
          else
            {
              ++measIdIt;
            }
        }

    }

  // 3GPP TS 36.331 section 5.5.2.7 Reporting configuration addition/ modification
  for (std::list<NrRrcSap::ReportConfigToAddMod>::iterator it = mc.reportConfigToAddModList.begin ();
       it !=  mc.reportConfigToAddModList.end ();
       ++it)
    {
      // simplifying assumptions
      NS_ASSERT_MSG (it->reportConfigEutra.triggerType == NrRrcSap::ReportConfigEutra::EVENT,
                     "only trigger type EVENT is supported");

      uint8_t reportConfigId = it->reportConfigId;
      std::map<uint8_t, NrRrcSap::ReportConfigToAddMod>::iterator reportConfigIt = m_varMeasConfig.reportConfigList.find (reportConfigId);
      if (reportConfigIt != m_varMeasConfig.reportConfigList.end ())
        {
          NS_LOG_LOGIC ("reportConfigId " << (uint32_t) reportConfigId << " exists, updating entry");
          m_varMeasConfig.reportConfigList[reportConfigId] = *it;
          for (std::map<uint8_t, NrRrcSap::MeasIdToAddMod>::iterator measIdIt 
                 = m_varMeasConfig.measIdList.begin ();
               measIdIt != m_varMeasConfig.measIdList.end ();
               ++measIdIt)
            {
              if (measIdIt->second.reportConfigId == reportConfigId)
                {
                  uint8_t measId = measIdIt->second.measId;
                  NS_LOG_LOGIC (this << " found measId " << (uint32_t) measId << " referring to reportConfigId " << (uint32_t)  reportConfigId);
                  VarMeasReportListClear (measId);
                }
            }
        }
      else
        {
          NS_LOG_LOGIC ("reportConfigId " << (uint32_t) reportConfigId << " is new, adding entry");
          m_varMeasConfig.reportConfigList[reportConfigId] = *it;
        }

    }

  // 3GPP TS 36.331 section 5.5.2.8 Quantity configuration
  if (mc.haveQuantityConfig)
    {
      NS_LOG_LOGIC (this << " setting quantityConfig");
      m_varMeasConfig.quantityConfig = mc.quantityConfig;
      // we calculate here the coefficient a used for Layer 3 filtering, see 3GPP TS 36.331 section 5.5.3.2
      m_varMeasConfig.aRsrp = std::pow (0.5, mc.quantityConfig.filterCoefficientRSRP / 4.0);
      m_varMeasConfig.aRsrq = std::pow (0.5, mc.quantityConfig.filterCoefficientRSRQ / 4.0);
      NS_LOG_LOGIC (this << " new filter coefficients: aRsrp=" << m_varMeasConfig.aRsrp << ", aRsrq=" << m_varMeasConfig.aRsrq);

      for (std::map<uint8_t, NrRrcSap::MeasIdToAddMod>::iterator measIdIt
             = m_varMeasConfig.measIdList.begin ();
           measIdIt != m_varMeasConfig.measIdList.end ();
           ++measIdIt)
        {
          VarMeasReportListClear (measIdIt->second.measId);
        }
    }

  // 3GPP TS 36.331 section 5.5.2.2 Measurement identity removal
  for (std::list<uint8_t>::iterator it = mc.measIdToRemoveList.begin ();
       it !=  mc.measIdToRemoveList.end ();
       ++it)
    {
      uint8_t measId = *it;
      NS_LOG_LOGIC (this << " deleting measId " << (uint32_t) measId);
      m_varMeasConfig.measIdList.erase (measId);
      VarMeasReportListClear (measId);

      // removing time-to-trigger queues
      m_enteringTriggerQueue.erase (measId);
      m_leavingTriggerQueue.erase (measId);
    }

  // 3GPP TS 36.331 section 5.5.2.3 Measurement identity addition/ modification
  for (std::list<NrRrcSap::MeasIdToAddMod>::iterator it = mc.measIdToAddModList.begin ();
       it !=  mc.measIdToAddModList.end ();
       ++it)
    {
      NS_LOG_LOGIC (this << " measId " << (uint32_t) it->measId
                         << " (measObjectId=" << (uint32_t) it->measObjectId
                         << ", reportConfigId=" << (uint32_t) it->reportConfigId
                         << ")");
      NS_ASSERT (m_varMeasConfig.measObjectList.find (it->measObjectId)
                 != m_varMeasConfig.measObjectList.end ());
      NS_ASSERT (m_varMeasConfig.reportConfigList.find (it->reportConfigId)
                 != m_varMeasConfig.reportConfigList.end ());
      m_varMeasConfig.measIdList[it->measId] = *it; // side effect: create new entry if not exists
      std::map<uint8_t, VarMeasReport>::iterator measReportIt = m_varMeasReportList.find (it->measId);
      if (measReportIt != m_varMeasReportList.end ())
        {
          measReportIt->second.periodicReportTimer.Cancel ();
          m_varMeasReportList.erase (measReportIt);
        }
      NS_ASSERT (m_varMeasConfig.reportConfigList.find (it->reportConfigId)
                 ->second.reportConfigEutra.triggerType != NrRrcSap::ReportConfigEutra::PERIODICAL);

      // new empty queues for time-to-trigger
      std::list<PendingTrigger_t> s;
      m_enteringTriggerQueue[it->measId] = s;
      m_leavingTriggerQueue[it->measId] = s;
    }

  if (mc.haveMeasGapConfig)
    {
      NS_FATAL_ERROR ("measurement gaps are currently not supported");
    }

  if (mc.haveSmeasure)
    {
      NS_FATAL_ERROR ("s-measure is currently not supported");
    }

  if (mc.haveSpeedStatePars)
    {
      NS_FATAL_ERROR ("SpeedStatePars are currently not supported");
    }
}

void
NrUeRrc::SaveUeMeasurements (uint16_t cellId, double rsrp, double rsrq,
                              bool useLayer3Filtering)
{
  NS_LOG_FUNCTION (this << cellId << rsrp << rsrq << useLayer3Filtering);

  std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.find (cellId);

  if (storedMeasIt != m_storedMeasValues.end ())
    {
      if (useLayer3Filtering)
        {
          // F_n = (1-a) F_{n-1} + a M_n
          storedMeasIt->second.rsrp = (1 - m_varMeasConfig.aRsrp) * storedMeasIt->second.rsrp
            + m_varMeasConfig.aRsrp * rsrp;

          if (std::isnan (storedMeasIt->second.rsrq))
            {
              // the previous RSRQ measurements provided UE PHY are invalid
              storedMeasIt->second.rsrq = rsrq; // replace it with unfiltered value
            }
          else
            {
              storedMeasIt->second.rsrq = (1 - m_varMeasConfig.aRsrq) * storedMeasIt->second.rsrq
                + m_varMeasConfig.aRsrq * rsrq;
            }
        }
      else
        {
          storedMeasIt->second.rsrp = rsrp;
          storedMeasIt->second.rsrq = rsrq;
        }
    }
  else
    {
      // first value is always unfiltered
      MeasValues v;
      v.rsrp = rsrp;
      v.rsrq = rsrq;
      std::pair<uint16_t, MeasValues> val (cellId, v);
      std::pair<std::map<uint16_t, MeasValues>::iterator, bool>
        ret = m_storedMeasValues.insert (val);
      NS_ASSERT_MSG (ret.second == true, "element already existed");
      storedMeasIt = ret.first;
    }

  NS_LOG_DEBUG (this << " IMSI " << m_imsi << " state " << ToString (m_state)
                     << ", measured cell " << m_cellId
                     << ", new RSRP " << rsrp << " stored " << storedMeasIt->second.rsrp
                     << ", new RSRQ " << rsrq << " stored " << storedMeasIt->second.rsrq);
  storedMeasIt->second.timestamp = Simulator::Now ();

} // end of void SaveUeMeasurements

void
NrUeRrc::MeasurementReportTriggering (uint8_t measId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);

  std::map<uint8_t, NrRrcSap::MeasIdToAddMod>::iterator measIdIt =
    m_varMeasConfig.measIdList.find (measId);
  NS_ASSERT (measIdIt != m_varMeasConfig.measIdList.end ());
  NS_ASSERT (measIdIt->first == measIdIt->second.measId);

  std::map<uint8_t, NrRrcSap::ReportConfigToAddMod>::iterator
    reportConfigIt = m_varMeasConfig.reportConfigList.find (measIdIt->second.reportConfigId);
  NS_ASSERT (reportConfigIt != m_varMeasConfig.reportConfigList.end ());
  NrRrcSap::ReportConfigEutra& reportConfigEutra = reportConfigIt->second.reportConfigEutra;

  std::map<uint8_t, NrRrcSap::MeasObjectToAddMod>::iterator
    measObjectIt = m_varMeasConfig.measObjectList.find (measIdIt->second.measObjectId);
  NS_ASSERT (measObjectIt != m_varMeasConfig.measObjectList.end ());
  NrRrcSap::MeasObjectEutra& measObjectEutra = measObjectIt->second.measObjectEutra;

  std::map<uint8_t, VarMeasReport>::iterator
    measReportIt = m_varMeasReportList.find (measId);
  bool isMeasIdInReportList = (measReportIt != m_varMeasReportList.end ());

  // we don't check the purpose field, as it is only included for
  // triggerType == periodical, which is not supported
  NS_ASSERT_MSG (reportConfigEutra.triggerType
                 == NrRrcSap::ReportConfigEutra::EVENT,
                 "only triggerType == event is supported");
  // only EUTRA is supported, no need to check for it

  NS_LOG_LOGIC (this << " considering measId " << (uint32_t) measId);
  bool eventEntryCondApplicable = false;
  bool eventLeavingCondApplicable = false;
  ConcernedCells_t concernedCellsEntry;
  ConcernedCells_t concernedCellsLeaving;

  switch (reportConfigEutra.eventId)
    {
    case NrRrcSap::ReportConfigEutra::EVENT_A1:
      {
        /*
         * Event A1 (Serving becomes better than threshold)
         * Please refer to 3GPP TS 36.331 Section 5.5.4.2
         */

        double ms; // Ms, the measurement result of the serving cell
        double thresh; // Thresh, the threshold parameter for this event
        // Hys, the hysteresis parameter for this event.
        double hys = NrEutranMeasurementMapping::IeValue2ActualHysteresis (reportConfigEutra.hysteresis);

        switch (reportConfigEutra.triggerQuantity)
          {
          case NrRrcSap::ReportConfigEutra::RSRP:
            ms = m_storedMeasValues[m_cellId].rsrp;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            thresh = NrEutranMeasurementMapping::RsrpRange2Dbm (reportConfigEutra.threshold1.range);
            break;
          case NrRrcSap::ReportConfigEutra::RSRQ:
            ms = m_storedMeasValues[m_cellId].rsrq;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            thresh = NrEutranMeasurementMapping::RsrqRange2Db (reportConfigEutra.threshold1.range);
            break;
          default:
            NS_FATAL_ERROR ("unsupported triggerQuantity");
            break;
          }

        // Inequality A1-1 (Entering condition): Ms - Hys > Thresh
        bool entryCond = ms - hys > thresh;

        if (entryCond)
          {
            if (!isMeasIdInReportList)
              {
                concernedCellsEntry.push_back (m_cellId);
                eventEntryCondApplicable = true;
              }
            else
              {
                /*
                 * This is to check that the triggered cell recorded in the
                 * VarMeasReportList is the serving cell.
                 */
                NS_ASSERT (measReportIt->second.cellsTriggeredList.find (m_cellId)
                           != measReportIt->second.cellsTriggeredList.end ());
              }
          }
        else if (reportConfigEutra.timeToTrigger > 0)
          {
            CancelEnteringTrigger (measId);
          }

        // Inequality A1-2 (Leaving condition): Ms + Hys < Thresh
        bool leavingCond = ms + hys < thresh;

        if (leavingCond)
          {
            if (isMeasIdInReportList)
              {
                /*
                 * This is to check that the triggered cell recorded in the
                 * VarMeasReportList is the serving cell.
                 */
                NS_ASSERT (measReportIt->second.cellsTriggeredList.find (m_cellId)
                           != measReportIt->second.cellsTriggeredList.end ());
                concernedCellsLeaving.push_back (m_cellId);
                eventLeavingCondApplicable = true;
              }
          }
        else if (reportConfigEutra.timeToTrigger > 0)
          {
            CancelLeavingTrigger (measId);
          }

        NS_LOG_LOGIC (this << " event A1: serving cell " << m_cellId
                           << " ms=" << ms << " thresh=" << thresh
                           << " entryCond=" << entryCond
                           << " leavingCond=" << leavingCond);

      } // end of case NrRrcSap::ReportConfigEutra::EVENT_A1

      break;

    case NrRrcSap::ReportConfigEutra::EVENT_A2:
      {
        /*
         * Event A2 (Serving becomes worse than threshold)
         * Please refer to 3GPP TS 36.331 Section 5.5.4.3
         */

        double ms; // Ms, the measurement result of the serving cell
        double thresh; // Thresh, the threshold parameter for this event
        // Hys, the hysteresis parameter for this event.
        double hys = NrEutranMeasurementMapping::IeValue2ActualHysteresis (reportConfigEutra.hysteresis);

        switch (reportConfigEutra.triggerQuantity)
          {
          case NrRrcSap::ReportConfigEutra::RSRP:
            ms = m_storedMeasValues[m_cellId].rsrp;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            thresh = NrEutranMeasurementMapping::RsrpRange2Dbm (reportConfigEutra.threshold1.range);
            break;
          case NrRrcSap::ReportConfigEutra::RSRQ:
            ms = m_storedMeasValues[m_cellId].rsrq;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            thresh = NrEutranMeasurementMapping::RsrqRange2Db (reportConfigEutra.threshold1.range);
            break;
          default:
            NS_FATAL_ERROR ("unsupported triggerQuantity");
            break;
          }

        // Inequality A2-1 (Entering condition): Ms + Hys < Thresh
        bool entryCond = ms + hys < thresh;

        if (entryCond)
          {
            if (!isMeasIdInReportList)
              {
                concernedCellsEntry.push_back (m_cellId);
                eventEntryCondApplicable = true;
              }
            else
              {
                /*
                 * This is to check that the triggered cell recorded in the
                 * VarMeasReportList is the serving cell.
                 */
                NS_ASSERT (measReportIt->second.cellsTriggeredList.find (m_cellId)
                           != measReportIt->second.cellsTriggeredList.end ());
              }
          }
        else if (reportConfigEutra.timeToTrigger > 0)
          {
            CancelEnteringTrigger (measId);
          }

        // Inequality A2-2 (Leaving condition): Ms - Hys > Thresh
        bool leavingCond = ms - hys > thresh;

        if (leavingCond)
          {
            if (isMeasIdInReportList)
              {
                /*
                 * This is to check that the triggered cell recorded in the
                 * VarMeasReportList is the serving cell.
                 */
                NS_ASSERT (measReportIt->second.cellsTriggeredList.find (m_cellId)
                           != measReportIt->second.cellsTriggeredList.end ());
                concernedCellsLeaving.push_back (m_cellId);
                eventLeavingCondApplicable = true;
              }
          }
        else if (reportConfigEutra.timeToTrigger > 0)
          {
            CancelLeavingTrigger (measId);
          }

        NS_LOG_LOGIC (this << " event A2: serving cell " << m_cellId
                           << " ms=" << ms << " thresh=" << thresh
                           << " entryCond=" << entryCond
                           << " leavingCond=" << leavingCond);

      } // end of case NrRrcSap::ReportConfigEutra::EVENT_A2

      break;

    case NrRrcSap::ReportConfigEutra::EVENT_A3:
      {
        /*
         * Event A3 (Neighbour becomes offset better than PCell)
         * Please refer to 3GPP TS 36.331 Section 5.5.4.4
         */

        double mn; // Mn, the measurement result of the neighbouring cell
        double ofn = measObjectEutra.offsetFreq; // Ofn, the frequency specific offset of the frequency of the
        double ocn = 0.0; // Ocn, the cell specific offset of the neighbour cell
        double mp; // Mp, the measurement result of the PCell
        double ofp = measObjectEutra.offsetFreq; // Ofp, the frequency specific offset of the primary frequency
        double ocp = 0.0; // Ocp, the cell specific offset of the PCell
        // Off, the offset parameter for this event.
        double off = NrEutranMeasurementMapping::IeValue2ActualA3Offset (reportConfigEutra.a3Offset);
        // Hys, the hysteresis parameter for this event.
        double hys = NrEutranMeasurementMapping::IeValue2ActualHysteresis (reportConfigEutra.hysteresis);

        switch (reportConfigEutra.triggerQuantity)
          {
          case NrRrcSap::ReportConfigEutra::RSRP:
            mp = m_storedMeasValues[m_cellId].rsrp;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            break;
          case NrRrcSap::ReportConfigEutra::RSRQ:
            mp = m_storedMeasValues[m_cellId].rsrq;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            break;
          default:
            NS_FATAL_ERROR ("unsupported triggerQuantity");
            break;
          }

        for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
             storedMeasIt != m_storedMeasValues.end ();
             ++storedMeasIt)
          {
            uint16_t cellId = storedMeasIt->first;
            if (cellId == m_cellId)
              {
                continue;
              }

            switch (reportConfigEutra.triggerQuantity)
              {
              case NrRrcSap::ReportConfigEutra::RSRP:
                mn = storedMeasIt->second.rsrp;
                break;
              case NrRrcSap::ReportConfigEutra::RSRQ:
                mn = storedMeasIt->second.rsrq;
                break;
              default:
                NS_FATAL_ERROR ("unsupported triggerQuantity");
                break;
              }

            bool hasTriggered = isMeasIdInReportList
              && (measReportIt->second.cellsTriggeredList.find (cellId)
                  != measReportIt->second.cellsTriggeredList.end ());

            // Inequality A3-1 (Entering condition): Mn + Ofn + Ocn - Hys > Mp + Ofp + Ocp + Off
            bool entryCond = mn + ofn + ocn - hys > mp + ofp + ocp + off;

            if (entryCond)
              {
                if (!hasTriggered)
                  {
                    concernedCellsEntry.push_back (cellId);
                    eventEntryCondApplicable = true;
                  }
              }
            else if (reportConfigEutra.timeToTrigger > 0)
              {
                CancelEnteringTrigger (measId, cellId);
              }

            // Inequality A3-2 (Leaving condition): Mn + Ofn + Ocn + Hys < Mp + Ofp + Ocp + Off
            bool leavingCond = mn + ofn + ocn + hys < mp + ofp + ocp + off;

            if (leavingCond)
              {
                if (hasTriggered)
                  {
                    concernedCellsLeaving.push_back (cellId);
                    eventLeavingCondApplicable = true;
                  }
              }
            else if (reportConfigEutra.timeToTrigger > 0)
              {
                CancelLeavingTrigger (measId, cellId);
              }

            NS_LOG_LOGIC (this << " event A3: neighbor cell " << cellId
                               << " mn=" << mn << " mp=" << mp << " offset=" << off
                               << " entryCond=" << entryCond
                               << " leavingCond=" << leavingCond);

          } // end of for (storedMeasIt)

      } // end of case NrRrcSap::ReportConfigEutra::EVENT_A3

      break;

    case NrRrcSap::ReportConfigEutra::EVENT_A4:
      {
        /*
         * Event A4 (Neighbour becomes better than threshold)
         * Please refer to 3GPP TS 36.331 Section 5.5.4.5
         */

        double mn; // Mn, the measurement result of the neighbouring cell
        double ofn = measObjectEutra.offsetFreq; // Ofn, the frequency specific offset of the frequency of the
        double ocn = 0.0; // Ocn, the cell specific offset of the neighbour cell
        double thresh; // Thresh, the threshold parameter for this event
        // Hys, the hysteresis parameter for this event.
        double hys = NrEutranMeasurementMapping::IeValue2ActualHysteresis (reportConfigEutra.hysteresis);

        switch (reportConfigEutra.triggerQuantity)
          {
          case NrRrcSap::ReportConfigEutra::RSRP:
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            thresh = NrEutranMeasurementMapping::RsrpRange2Dbm (reportConfigEutra.threshold1.range);
            break;
          case NrRrcSap::ReportConfigEutra::RSRQ:
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            thresh = NrEutranMeasurementMapping::RsrqRange2Db (reportConfigEutra.threshold1.range);
            break;
          default:
            NS_FATAL_ERROR ("unsupported triggerQuantity");
            break;
          }

        for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
             storedMeasIt != m_storedMeasValues.end ();
             ++storedMeasIt)
          {
            uint16_t cellId = storedMeasIt->first;
            if (cellId == m_cellId)
              {
                continue;
              }

            switch (reportConfigEutra.triggerQuantity)
              {
              case NrRrcSap::ReportConfigEutra::RSRP:
                mn = storedMeasIt->second.rsrp;
                break;
              case NrRrcSap::ReportConfigEutra::RSRQ:
                mn = storedMeasIt->second.rsrq;
                break;
              default:
                NS_FATAL_ERROR ("unsupported triggerQuantity");
                break;
              }

            bool hasTriggered = isMeasIdInReportList
              && (measReportIt->second.cellsTriggeredList.find (cellId)
                  != measReportIt->second.cellsTriggeredList.end ());

            // Inequality A4-1 (Entering condition): Mn + Ofn + Ocn - Hys > Thresh
            bool entryCond = mn + ofn + ocn - hys > thresh;

            if (entryCond)
              {
                if (!hasTriggered)
                  {
                    concernedCellsEntry.push_back (cellId);
                    eventEntryCondApplicable = true;
                  }
              }
            else if (reportConfigEutra.timeToTrigger > 0)
              {
                CancelEnteringTrigger (measId, cellId);
              }

            // Inequality A4-2 (Leaving condition): Mn + Ofn + Ocn + Hys < Thresh
            bool leavingCond = mn + ofn + ocn + hys < thresh;

            if (leavingCond)
              {
                if (hasTriggered)
                  {
                    concernedCellsLeaving.push_back (cellId);
                    eventLeavingCondApplicable = true;
                  }
              }
            else if (reportConfigEutra.timeToTrigger > 0)
              {
                CancelLeavingTrigger (measId, cellId);
              }

            NS_LOG_LOGIC (this << " event A4: neighbor cell " << cellId
                               << " mn=" << mn << " thresh=" << thresh
                               << " entryCond=" << entryCond
                               << " leavingCond=" << leavingCond);

          } // end of for (storedMeasIt)

      } // end of case NrRrcSap::ReportConfigEutra::EVENT_A4

      break;

    case NrRrcSap::ReportConfigEutra::EVENT_A5:
      {
        /*
         * Event A5 (PCell becomes worse than threshold1 and neighbour
         * becomes better than threshold2)
         * Please refer to 3GPP TS 36.331 Section 5.5.4.6
         */

        double mp; // Mp, the measurement result of the PCell
        double mn; // Mn, the measurement result of the neighbouring cell
        double ofn = measObjectEutra.offsetFreq; // Ofn, the frequency specific offset of the frequency of the
        double ocn = 0.0; // Ocn, the cell specific offset of the neighbour cell
        double thresh1; // Thresh1, the threshold parameter for this event
        double thresh2; // Thresh2, the threshold parameter for this event
        // Hys, the hysteresis parameter for this event.
        double hys = NrEutranMeasurementMapping::IeValue2ActualHysteresis (reportConfigEutra.hysteresis);

        switch (reportConfigEutra.triggerQuantity)
          {
          case NrRrcSap::ReportConfigEutra::RSRP:
            mp = m_storedMeasValues[m_cellId].rsrp;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            NS_ASSERT (reportConfigEutra.threshold2.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            thresh1 = NrEutranMeasurementMapping::RsrpRange2Dbm (reportConfigEutra.threshold1.range);
            thresh2 = NrEutranMeasurementMapping::RsrpRange2Dbm (reportConfigEutra.threshold2.range);
            break;
          case NrRrcSap::ReportConfigEutra::RSRQ:
            mp = m_storedMeasValues[m_cellId].rsrq;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            NS_ASSERT (reportConfigEutra.threshold2.choice
                       == NrRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            thresh1 = NrEutranMeasurementMapping::RsrqRange2Db (reportConfigEutra.threshold1.range);
            thresh2 = NrEutranMeasurementMapping::RsrqRange2Db (reportConfigEutra.threshold2.range);
            break;
          default:
            NS_FATAL_ERROR ("unsupported triggerQuantity");
            break;
          }

        // Inequality A5-1 (Entering condition 1): Mp + Hys < Thresh1
        bool entryCond = mp + hys < thresh1;

        if (entryCond)
          {
            for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
                 storedMeasIt != m_storedMeasValues.end ();
                 ++storedMeasIt)
              {
                uint16_t cellId = storedMeasIt->first;
                if (cellId == m_cellId)
                  {
                    continue;
                  }

                switch (reportConfigEutra.triggerQuantity)
                  {
                  case NrRrcSap::ReportConfigEutra::RSRP:
                    mn = storedMeasIt->second.rsrp;
                    break;
                  case NrRrcSap::ReportConfigEutra::RSRQ:
                    mn = storedMeasIt->second.rsrq;
                    break;
                  default:
                    NS_FATAL_ERROR ("unsupported triggerQuantity");
                    break;
                  }

                bool hasTriggered = isMeasIdInReportList
                  && (measReportIt->second.cellsTriggeredList.find (cellId)
                      != measReportIt->second.cellsTriggeredList.end ());

                // Inequality A5-2 (Entering condition 2): Mn + Ofn + Ocn - Hys > Thresh2

                entryCond = mn + ofn + ocn - hys > thresh2;

                if (entryCond)
                  {
                    if (!hasTriggered)
                      {
                        concernedCellsEntry.push_back (cellId);
                        eventEntryCondApplicable = true;
                      }
                  }
                else if (reportConfigEutra.timeToTrigger > 0)
                  {
                    CancelEnteringTrigger (measId, cellId);
                  }

                NS_LOG_LOGIC (this << " event A5: neighbor cell " << cellId
                                   << " mn=" << mn << " mp=" << mp
                                   << " thresh2=" << thresh2
                                   << " thresh1=" << thresh1
                                   << " entryCond=" << entryCond);

              } // end of for (storedMeasIt)

          } // end of if (entryCond)
        else
          {
            NS_LOG_LOGIC (this << " event A5: serving cell " << m_cellId
                               << " mp=" << mp << " thresh1=" << thresh1
                               << " entryCond=" << entryCond);

            if (reportConfigEutra.timeToTrigger > 0)
              {
                CancelEnteringTrigger (measId);
              }
          }

        if (isMeasIdInReportList)
          {
            // Inequality A5-3 (Leaving condition 1): Mp - Hys > Thresh1
            bool leavingCond = mp - hys > thresh1;

            if (leavingCond)
              {
                if (reportConfigEutra.timeToTrigger == 0)
                  {
                    // leaving condition #2 does not have to be checked

                    for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
                         storedMeasIt != m_storedMeasValues.end ();
                         ++storedMeasIt)
                      {
                        uint16_t cellId = storedMeasIt->first;
                        if (cellId == m_cellId)
                          {
                            continue;
                          }

                        if (measReportIt->second.cellsTriggeredList.find (cellId)
                            != measReportIt->second.cellsTriggeredList.end ())
                          {
                            concernedCellsLeaving.push_back (cellId);
                            eventLeavingCondApplicable = true;
                          }
                      }
                  } // end of if (reportConfigEutra.timeToTrigger == 0)
                else
                  {
                    // leaving condition #2 has to be checked to cancel time-to-trigger

                    for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
                         storedMeasIt != m_storedMeasValues.end ();
                         ++storedMeasIt)
                      {
                        uint16_t cellId = storedMeasIt->first;
                        if (cellId == m_cellId)
                          {
                            continue;
                          }

                        if (measReportIt->second.cellsTriggeredList.find (cellId)
                            != measReportIt->second.cellsTriggeredList.end ())
                          {
                            switch (reportConfigEutra.triggerQuantity)
                              {
                              case NrRrcSap::ReportConfigEutra::RSRP:
                                mn = storedMeasIt->second.rsrp;
                                break;
                              case NrRrcSap::ReportConfigEutra::RSRQ:
                                mn = storedMeasIt->second.rsrq;
                                break;
                              default:
                                NS_FATAL_ERROR ("unsupported triggerQuantity");
                                break;
                              }

                            // Inequality A5-4 (Leaving condition 2): Mn + Ofn + Ocn + Hys < Thresh2

                            leavingCond = mn + ofn + ocn + hys < thresh2;

                            if (!leavingCond)
                              {
                                CancelLeavingTrigger (measId, cellId);
                              }

                            /*
                             * Whatever the result of leaving condition #2, this
                             * cell is still "in", because leaving condition #1
                             * is already true.
                             */
                            concernedCellsLeaving.push_back (cellId);
                            eventLeavingCondApplicable = true;

                            NS_LOG_LOGIC (this << " event A5: neighbor cell " << cellId
                                               << " mn=" << mn << " mp=" << mp
                                               << " thresh2=" << thresh2
                                               << " thresh1=" << thresh1
                                               << " leavingCond=" << leavingCond);

                          } // end of if (measReportIt->second.cellsTriggeredList.find (cellId)
                            //            != measReportIt->second.cellsTriggeredList.end ())

                      } // end of for (storedMeasIt)

                  } // end of else of if (reportConfigEutra.timeToTrigger == 0)

                NS_LOG_LOGIC (this << " event A5: serving cell " << m_cellId
                                   << " mp=" << mp << " thresh1=" << thresh1
                                   << " leavingCond=" << leavingCond);

              } // end of if (leavingCond)
            else
              {
                if (reportConfigEutra.timeToTrigger > 0)
                  {
                    CancelLeavingTrigger (measId);
                  }

                // check leaving condition #2

                for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
                     storedMeasIt != m_storedMeasValues.end ();
                     ++storedMeasIt)
                  {
                    uint16_t cellId = storedMeasIt->first;
                    if (cellId == m_cellId)
                      {
                        continue;
                      }

                    if (measReportIt->second.cellsTriggeredList.find (cellId)
                        != measReportIt->second.cellsTriggeredList.end ())
                      {
                        switch (reportConfigEutra.triggerQuantity)
                          {
                          case NrRrcSap::ReportConfigEutra::RSRP:
                            mn = storedMeasIt->second.rsrp;
                            break;
                          case NrRrcSap::ReportConfigEutra::RSRQ:
                            mn = storedMeasIt->second.rsrq;
                            break;
                          default:
                            NS_FATAL_ERROR ("unsupported triggerQuantity");
                            break;
                          }

                        // Inequality A5-4 (Leaving condition 2): Mn + Ofn + Ocn + Hys < Thresh2
                        leavingCond = mn + ofn + ocn + hys < thresh2;

                        if (leavingCond)
                          {
                            concernedCellsLeaving.push_back (cellId);
                            eventLeavingCondApplicable = true;
                          }

                        NS_LOG_LOGIC (this << " event A5: neighbor cell " << cellId
                                           << " mn=" << mn << " mp=" << mp
                                           << " thresh2=" << thresh2
                                           << " thresh1=" << thresh1
                                           << " leavingCond=" << leavingCond);

                      } // end of if (measReportIt->second.cellsTriggeredList.find (cellId)
                        //            != measReportIt->second.cellsTriggeredList.end ())

                  } // end of for (storedMeasIt)

              } // end of else of if (leavingCond)

          } // end of if (isMeasIdInReportList)

      } // end of case NrRrcSap::ReportConfigEutra::EVENT_A5

      break;

    default:
      NS_FATAL_ERROR ("unsupported eventId " << reportConfigEutra.eventId);
      break;

    } // switch (event type)

  NS_LOG_LOGIC (this << " eventEntryCondApplicable=" << eventEntryCondApplicable
                     << " eventLeavingCondApplicable=" << eventLeavingCondApplicable);

  if (eventEntryCondApplicable)
    {
      if (reportConfigEutra.timeToTrigger == 0)
        {
          VarMeasReportListAdd (measId, concernedCellsEntry);
        }
      else
        {
          PendingTrigger_t t;
          t.measId = measId;
          t.concernedCells = concernedCellsEntry;
          t.timer = Simulator::Schedule (MilliSeconds (reportConfigEutra.timeToTrigger),
                                         &NrUeRrc::VarMeasReportListAdd, this,
                                         measId, concernedCellsEntry);
          std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
            enteringTriggerIt = m_enteringTriggerQueue.find (measId);
          NS_ASSERT (enteringTriggerIt != m_enteringTriggerQueue.end ());
          enteringTriggerIt->second.push_back (t);
        }
    }

  if (eventLeavingCondApplicable)
    {
      // reportOnLeave will only be set when eventId = eventA3
      bool reportOnLeave = (reportConfigEutra.eventId == NrRrcSap::ReportConfigEutra::EVENT_A3)
        && reportConfigEutra.reportOnLeave;

      if (reportConfigEutra.timeToTrigger == 0)
        {
          VarMeasReportListErase (measId, concernedCellsLeaving, reportOnLeave);
        }
      else
        {
          PendingTrigger_t t;
          t.measId = measId;
          t.concernedCells = concernedCellsLeaving;
          t.timer = Simulator::Schedule (MilliSeconds (reportConfigEutra.timeToTrigger),
                                         &NrUeRrc::VarMeasReportListErase, this,
                                         measId, concernedCellsLeaving, reportOnLeave);
          std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
            leavingTriggerIt = m_leavingTriggerQueue.find (measId);
          NS_ASSERT (leavingTriggerIt != m_leavingTriggerQueue.end ());
          leavingTriggerIt->second.push_back (t);
        }
    }

} // end of void NrUeRrc::MeasurementReportTriggering (uint8_t measId)

void
NrUeRrc::CancelEnteringTrigger (uint8_t measId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    it1 = m_enteringTriggerQueue.find (measId);
  NS_ASSERT (it1 != m_enteringTriggerQueue.end ());

  if (!it1->second.empty ())
    {
      std::list<PendingTrigger_t>::iterator it2;
      for (it2 = it1->second.begin (); it2 != it1->second.end (); ++it2)
        {
          NS_ASSERT (it2->measId == measId);
          NS_LOG_LOGIC (this << " canceling entering time-to-trigger event at "
                             << Simulator::GetDelayLeft (it2->timer).GetSeconds ());
          Simulator::Cancel (it2->timer);
        }

      it1->second.clear ();
    }
}

void
NrUeRrc::CancelEnteringTrigger (uint8_t measId, uint16_t cellId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId << cellId);

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    it1 = m_enteringTriggerQueue.find (measId);
  NS_ASSERT (it1 != m_enteringTriggerQueue.end ());

  std::list<PendingTrigger_t>::iterator it2 = it1->second.begin ();
  while (it2 != it1->second.end ())
    {
      NS_ASSERT (it2->measId == measId);

      ConcernedCells_t::iterator it3;
      for (it3 = it2->concernedCells.begin ();
           it3 != it2->concernedCells.end (); ++it3)
        {
          if (*it3 == cellId)
            {
              it3 = it2->concernedCells.erase (it3);
            }
        }

      if (it2->concernedCells.empty ())
        {
          NS_LOG_LOGIC (this << " canceling entering time-to-trigger event at "
                             << Simulator::GetDelayLeft (it2->timer).GetSeconds ());
          Simulator::Cancel (it2->timer);
          it2 = it1->second.erase (it2);
        }
      else
        {
          it2++;
        }
    }
}

void
NrUeRrc::CancelLeavingTrigger (uint8_t measId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    it1 = m_leavingTriggerQueue.find (measId);
  NS_ASSERT (it1 != m_leavingTriggerQueue.end ());

  if (!it1->second.empty ())
    {
      std::list<PendingTrigger_t>::iterator it2;
      for (it2 = it1->second.begin (); it2 != it1->second.end (); ++it2)
        {
          NS_ASSERT (it2->measId == measId);
          NS_LOG_LOGIC (this << " canceling leaving time-to-trigger event at "
                             << Simulator::GetDelayLeft (it2->timer).GetSeconds ());
          Simulator::Cancel (it2->timer);
        }

      it1->second.clear ();
    }
}

void
NrUeRrc::CancelLeavingTrigger (uint8_t measId, uint16_t cellId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId << cellId);

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    it1 = m_leavingTriggerQueue.find (measId);
  NS_ASSERT (it1 != m_leavingTriggerQueue.end ());

  std::list<PendingTrigger_t>::iterator it2 = it1->second.begin ();
  while (it2 != it1->second.end ())
    {
      NS_ASSERT (it2->measId == measId);

      ConcernedCells_t::iterator it3;
      for (it3 = it2->concernedCells.begin ();
           it3 != it2->concernedCells.end (); ++it3)
        {
          if (*it3 == cellId)
            {
              it3 = it2->concernedCells.erase (it3);
            }
        }

      if (it2->concernedCells.empty ())
        {
          NS_LOG_LOGIC (this << " canceling leaving time-to-trigger event at "
                             << Simulator::GetDelayLeft (it2->timer).GetSeconds ());
          Simulator::Cancel (it2->timer);
          it2 = it1->second.erase (it2);
        }
      else
        {
          it2++;
        }
    }
}

void
NrUeRrc::VarMeasReportListAdd (uint8_t measId, ConcernedCells_t enteringCells)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);
  NS_ASSERT (!enteringCells.empty ());

  std::map<uint8_t, VarMeasReport>::iterator
    measReportIt = m_varMeasReportList.find (measId);

  if (measReportIt == m_varMeasReportList.end ())
    {
      VarMeasReport r;
      r.measId = measId;
      std::pair<uint8_t, VarMeasReport> val (measId, r);
      std::pair<std::map<uint8_t, VarMeasReport>::iterator, bool>
        ret = m_varMeasReportList.insert (val);
      NS_ASSERT_MSG (ret.second == true, "element already existed");
      measReportIt = ret.first;
    }

  NS_ASSERT (measReportIt != m_varMeasReportList.end ());

  for (ConcernedCells_t::const_iterator it = enteringCells.begin ();
       it != enteringCells.end ();
       ++it)
    {
      measReportIt->second.cellsTriggeredList.insert (*it);
    }

  NS_ASSERT (!measReportIt->second.cellsTriggeredList.empty ());
  measReportIt->second.numberOfReportsSent = 0;
  measReportIt->second.periodicReportTimer
    = Simulator::Schedule (UE_MEASUREMENT_REPORT_DELAY,
                           &NrUeRrc::SendMeasurementReport,
                           this, measId);

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    enteringTriggerIt = m_enteringTriggerQueue.find (measId);
  NS_ASSERT (enteringTriggerIt != m_enteringTriggerQueue.end ());
  if (!enteringTriggerIt->second.empty ())
    {
      /*
       * Assumptions at this point:
       *  - the call to this function was delayed by time-to-trigger;
       *  - the time-to-trigger delay is fixed (not adaptive/dynamic); and
       *  - the first element in the list is associated with this function call.
       */
      enteringTriggerIt->second.pop_front ();

      if (!enteringTriggerIt->second.empty ())
        {
          /*
           * To prevent the same set of cells triggering again in the future,
           * we clean up the time-to-trigger queue. This case might occur when
           * time-to-trigger > 200 ms.
           */
          for (ConcernedCells_t::const_iterator it = enteringCells.begin ();
               it != enteringCells.end (); ++it)
            {
              CancelEnteringTrigger (measId, *it);
            }
        }

    } // end of if (!enteringTriggerIt->second.empty ())

} // end of NrUeRrc::VarMeasReportListAdd

void
NrUeRrc::VarMeasReportListErase (uint8_t measId, ConcernedCells_t leavingCells,
                                  bool reportOnLeave)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);
  NS_ASSERT (!leavingCells.empty ());

  std::map<uint8_t, VarMeasReport>::iterator
    measReportIt = m_varMeasReportList.find (measId);
  NS_ASSERT (measReportIt != m_varMeasReportList.end ());

  for (ConcernedCells_t::const_iterator it = leavingCells.begin ();
       it != leavingCells.end ();
       ++it)
    {
      measReportIt->second.cellsTriggeredList.erase (*it);
    }

  if (reportOnLeave)
    {
      // runs immediately without UE_MEASUREMENT_REPORT_DELAY
      SendMeasurementReport (measId);
    }

  if (measReportIt->second.cellsTriggeredList.empty ())
    {
      measReportIt->second.periodicReportTimer.Cancel ();
      m_varMeasReportList.erase (measReportIt);
    }

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    leavingTriggerIt = m_leavingTriggerQueue.find (measId);
  NS_ASSERT (leavingTriggerIt != m_leavingTriggerQueue.end ());
  if (!leavingTriggerIt->second.empty ())
    {
      /*
       * Assumptions at this point:
       *  - the call to this function was delayed by time-to-trigger; and
       *  - the time-to-trigger delay is fixed (not adaptive/dynamic); and
       *  - the first element in the list is associated with this function call.
       */
      leavingTriggerIt->second.pop_front ();

      if (!leavingTriggerIt->second.empty ())
        {
          /*
           * To prevent the same set of cells triggering again in the future,
           * we clean up the time-to-trigger queue. This case might occur when
           * time-to-trigger > 200 ms.
           */
          for (ConcernedCells_t::const_iterator it = leavingCells.begin ();
               it != leavingCells.end (); ++it)
            {
              CancelLeavingTrigger (measId, *it);
            }
        }

    } // end of if (!leavingTriggerIt->second.empty ())

} // end of NrUeRrc::VarMeasReportListErase

void
NrUeRrc::VarMeasReportListClear (uint8_t measId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);

  // remove the measurement reporting entry for this measId from the VarMeasReportList
  std::map<uint8_t, VarMeasReport>::iterator
    measReportIt = m_varMeasReportList.find (measId);
  if (measReportIt != m_varMeasReportList.end ())
    {
      NS_LOG_LOGIC (this << " deleting existing report for measId " << (uint16_t) measId);
      measReportIt->second.periodicReportTimer.Cancel ();
      m_varMeasReportList.erase (measReportIt);
    }

  CancelEnteringTrigger (measId);
  CancelLeavingTrigger (measId);
}

void 
NrUeRrc::SendMeasurementReport (uint8_t measId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);
  //  3GPP TS 36.331 section 5.5.5 Measurement reporting

  std::map<uint8_t, NrRrcSap::MeasIdToAddMod>::iterator 
    measIdIt = m_varMeasConfig.measIdList.find (measId);
  NS_ASSERT (measIdIt != m_varMeasConfig.measIdList.end ());

  std::map<uint8_t, NrRrcSap::ReportConfigToAddMod>::iterator 
    reportConfigIt = m_varMeasConfig.reportConfigList.find (measIdIt->second.reportConfigId);
  NS_ASSERT (reportConfigIt != m_varMeasConfig.reportConfigList.end ());
  NrRrcSap::ReportConfigEutra& reportConfigEutra = reportConfigIt->second.reportConfigEutra;

  NrRrcSap::MeasurementReport measurementReport;
  NrRrcSap::MeasResults& measResults = measurementReport.measResults;
  measResults.measId = measId;

  bool cellHasMeasures = true; // measurements are available only for NR cells
  if(m_isMmWaveCellMap.find(m_cellId) != m_isMmWaveCellMap.end())
  {
    cellHasMeasures = !m_isMmWaveCellMap.find(m_cellId)->second;
  }
  if(cellHasMeasures)
  {
    std::map<uint16_t, MeasValues>::iterator servingMeasIt = m_storedMeasValues.find (m_cellId);
    NS_ASSERT (servingMeasIt != m_storedMeasValues.end ());
    measResults.rsrpResult = NrEutranMeasurementMapping::Dbm2RsrpRange (servingMeasIt->second.rsrp);
    measResults.rsrqResult = NrEutranMeasurementMapping::Db2RsrqRange (servingMeasIt->second.rsrq);
    NS_LOG_INFO (this << " reporting serving cell "
                 "RSRP " << (uint32_t) measResults.rsrpResult << " (" << servingMeasIt->second.rsrp << " dBm) "
                 "RSRQ " << (uint32_t) measResults.rsrqResult << " (" << servingMeasIt->second.rsrq << " dB)");
    measResults.haveMeasResultNeighCells = false;
    std::map<uint8_t, VarMeasReport>::iterator measReportIt = m_varMeasReportList.find (measId);
    if (measReportIt == m_varMeasReportList.end ())
    {
      NS_LOG_ERROR ("no entry found in m_varMeasReportList for measId " << (uint32_t) measId);
    }
    else
    {
      if (!(measReportIt->second.cellsTriggeredList.empty ()))
        {
          std::multimap<double, uint16_t> sortedNeighCells;
          for (std::set<uint16_t>::iterator cellsTriggeredIt = measReportIt->second.cellsTriggeredList.begin ();
               cellsTriggeredIt != measReportIt->second.cellsTriggeredList.end ();
               ++cellsTriggeredIt)
            {
              uint16_t cellId = *cellsTriggeredIt;
              if (cellId != m_cellId)
                {
                  std::map<uint16_t, MeasValues>::iterator neighborMeasIt = m_storedMeasValues.find (cellId);
                  double triggerValue;
                  switch (reportConfigEutra.triggerQuantity)
                    {
                    case NrRrcSap::ReportConfigEutra::RSRP:
                      triggerValue = neighborMeasIt->second.rsrp;
                      break;
                    case NrRrcSap::ReportConfigEutra::RSRQ:
                      triggerValue = neighborMeasIt->second.rsrq;
                      break;
                    default:
                      NS_FATAL_ERROR ("unsupported triggerQuantity");
                      break;
                    }
                  sortedNeighCells.insert (std::pair<double, uint16_t> (triggerValue, cellId));
                }
            }

          std::multimap<double, uint16_t>::reverse_iterator sortedNeighCellsIt;
          uint32_t count;
          for (sortedNeighCellsIt = sortedNeighCells.rbegin (), count = 0;
               sortedNeighCellsIt != sortedNeighCells.rend () && count < reportConfigEutra.maxReportCells;
               ++sortedNeighCellsIt, ++count)
            {
              uint16_t cellId = sortedNeighCellsIt->second;
              std::map<uint16_t, MeasValues>::iterator neighborMeasIt = m_storedMeasValues.find (cellId);
              NS_ASSERT (neighborMeasIt != m_storedMeasValues.end ());
              NrRrcSap::MeasResultEutra measResultEutra;
              measResultEutra.physCellId = cellId;
              measResultEutra.haveCgiInfo = false;
              measResultEutra.haveRsrpResult = true;
              measResultEutra.rsrpResult = NrEutranMeasurementMapping::Dbm2RsrpRange (neighborMeasIt->second.rsrp);
              measResultEutra.haveRsrqResult = true;
              measResultEutra.rsrqResult = NrEutranMeasurementMapping::Db2RsrqRange (neighborMeasIt->second.rsrq);
              NS_LOG_INFO (this << " reporting neighbor cell " << (uint32_t) measResultEutra.physCellId 
                                << " RSRP " << (uint32_t) measResultEutra.rsrpResult
                                << " (" << neighborMeasIt->second.rsrp << " dBm)"
                                << " RSRQ " << (uint32_t) measResultEutra.rsrqResult
                                << " (" << neighborMeasIt->second.rsrq << " dB)");
              measResults.measResultListEutra.push_back (measResultEutra);
              measResults.haveMeasResultNeighCells = true;
            }
        }
      else
        {
          NS_LOG_WARN (this << " cellsTriggeredList is empty");
        }

      /*
       * The current NrRrcSap implementation is broken in that it does not
       * allow for infinite values of reportAmount, which is probably the most
       * reasonable setting. So we just always assume infinite reportAmount.
       */
      measReportIt->second.numberOfReportsSent++;
      measReportIt->second.periodicReportTimer.Cancel ();

      Time reportInterval;
      switch (reportConfigEutra.reportInterval)
        {
        case NrRrcSap::ReportConfigEutra::MS120:
          reportInterval = MilliSeconds (120);
          break;
        case NrRrcSap::ReportConfigEutra::MS240:
          reportInterval = MilliSeconds (240);
          break;
        case NrRrcSap::ReportConfigEutra::MS480:
          reportInterval = MilliSeconds (480);
          break;
        case NrRrcSap::ReportConfigEutra::MS640:
          reportInterval = MilliSeconds (640);
          break;
        case NrRrcSap::ReportConfigEutra::MS1024:
          reportInterval = MilliSeconds (1024);
          break;
        case NrRrcSap::ReportConfigEutra::MS2048:
          reportInterval = MilliSeconds (2048);
          break;
        case NrRrcSap::ReportConfigEutra::MS5120:
          reportInterval = MilliSeconds (5120);
          break;
        case NrRrcSap::ReportConfigEutra::MS10240:
          reportInterval = MilliSeconds (10240);
          break;
        case NrRrcSap::ReportConfigEutra::MIN1:
          reportInterval = Seconds (60);
          break;
        case NrRrcSap::ReportConfigEutra::MIN6:
          reportInterval = Seconds (360);
          break;
        case NrRrcSap::ReportConfigEutra::MIN12:
          reportInterval = Seconds (720);
          break;
        case NrRrcSap::ReportConfigEutra::MIN30:
          reportInterval = Seconds (1800);
          break;
        case NrRrcSap::ReportConfigEutra::MIN60:
          reportInterval = Seconds (3600);
          break;
        default:
          NS_FATAL_ERROR ("Unsupported reportInterval " << (uint16_t) reportConfigEutra.reportInterval);
          break;
        }

      // schedule the next measurement reporting
      measReportIt->second.periodicReportTimer 
        = Simulator::Schedule (reportInterval,
                               &NrUeRrc::SendMeasurementReport,
                               this, measId);

      // send the measurement report to eNodeB
      m_rrcSapUser->SendMeasurementReport (measurementReport);
    } 
  }
}

void 
NrUeRrc::StartConnection ()
{
  NS_LOG_FUNCTION (this << m_imsi);
  NS_ASSERT (m_hasReceivedMib);
  NS_ASSERT (m_hasReceivedSib2);
  m_connectionPending = false; // reset the flag
  SwitchToState (IDLE_RANDOM_ACCESS);
  m_cmacSapProvider->StartContentionBasedRandomAccessProcedure ();
}

void 
NrUeRrc::LeaveConnectedMode ()
{
 std::cout<< "Leave Connected Mode ----->" <<m_imsi<<std::endl;
	NS_LOG_FUNCTION (this << m_imsi);
  m_asSapUser->NotifyConnectionReleased ();
  m_cmacSapProvider->RemoveLc (1);
  std::map<uint8_t, Ptr<NrDataRadioBearerInfo> >::iterator it;
  for (it = m_drbMap.begin (); it != m_drbMap.end (); ++it)
    {
      m_cmacSapProvider->RemoveLc (it->second->m_logicalChannelIdentity);
    }
  m_drbMap.clear ();
  m_bid2DrbidMap.clear ();
  m_srb1 = 0;
  SwitchToState (IDLE_CAMPED_NORMALLY);
}

void
NrUeRrc::ConnectionTimeout ()
{
  std::cout << "Connection time out  ---------->" <<m_imsi << std::endl;
	NS_LOG_FUNCTION (this << m_imsi);
  m_cmacSapProvider->Reset ();       // reset the MAC
  m_hasReceivedSib2 = false;         // invalidate the previously received SIB2
  SwitchToState (IDLE_START);  //IDLE_CAMPED_NORMALLY
  m_connectionTimeoutTrace (m_imsi, m_cellId, m_rnti);
  m_asSapUser->NotifyConnectionFailed ();  // inform upper layer
}

void
NrUeRrc::DisposeOldSrb1 ()
{
  NS_LOG_FUNCTION (this);
  m_srb1Old = 0;
}

uint8_t 
NrUeRrc::Bid2Drbid (uint8_t bid)
{
  std::map<uint8_t, uint8_t>::iterator it = m_bid2DrbidMap.find (bid);
  //NS_ASSERT_MSG (it != m_bid2DrbidMap.end (), "could not find BID " << bid);
  if (it == m_bid2DrbidMap.end ())
    {
      return 0;
    }
  else
    {
  return it->second;
    }
}

void 
NrUeRrc::SwitchToState (State newState)
{
  NS_LOG_FUNCTION (this << ToString (newState));
  State oldState = m_state;
  m_state = newState;
  NS_LOG_INFO (this << " IMSI " << m_imsi << " RNTI " << m_rnti << " CellId " << m_cellId << " UeRrc "
                    << ToString (oldState) << " --> " << ToString (newState));
  m_stateTransitionTrace (m_imsi, m_cellId, m_rnti, oldState, newState);

  switch (newState)
    {
    case IDLE_START:
    //  NS_FATAL_ERROR ("cannot switch to an initial state");
      break;

    case IDLE_CELL_SEARCH:
    case IDLE_WAIT_MIB_SIB1:
    case IDLE_WAIT_MIB:
    case IDLE_WAIT_SIB1:
      break;

    case IDLE_CAMPED_NORMALLY:
      if (m_connectionPending)
        {
          SwitchToState (IDLE_WAIT_SIB2);
        }
      break;

    case IDLE_WAIT_SIB2:
      if (m_hasReceivedSib2)
        {
          NS_ASSERT (m_connectionPending);
          StartConnection ();
        }
      break;

    case IDLE_RANDOM_ACCESS:
    case IDLE_CONNECTING:
    case CONNECTED_NORMALLY:
    case CONNECTED_HANDOVER:
    case CONNECTED_PHY_PROBLEM:
    case CONNECTED_REESTABLISHING:
      break;
 
    default:
      break;
    }
}




} // namespace ns3

