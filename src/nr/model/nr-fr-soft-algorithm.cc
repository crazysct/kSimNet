/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Piotr Gawlowicz
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
 * Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>
 *
 */

#include "nr-fr-soft-algorithm.h"
#include <ns3/log.h>
#include "ns3/boolean.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrFrSoftAlgorithm");

NS_OBJECT_ENSURE_REGISTERED (NrFrSoftAlgorithm);

static const struct FrSoftDownlinkDefaultConfiguration
{
  uint8_t cellId;
  uint8_t dlBandwidth;
  uint8_t dlEgdeSubBandOffset;
  uint8_t dlEdgeSubBandwidth;
} g_frSoftDownlinkDefaultConfiguration[] = {
  { 1, 15, 0, 4},
  { 2, 15, 4, 4},
  { 3, 15, 8, 6},
  { 1, 25, 0, 8},
  { 2, 25, 8, 8},
  { 3, 25, 16, 9},
  { 1, 50, 0, 16},
  { 2, 50, 16, 16},
  { 3, 50, 32, 18},
  { 1, 75, 0, 24},
  { 2, 75, 24, 24},
  { 3, 75, 48, 27},
  { 1, 100, 0, 32},
  { 2, 100, 32, 32},
  { 3, 100, 64, 36}
};

static const struct FrSoftUplinkDefaultConfiguration
{
  uint8_t cellId;
  uint8_t ulBandwidth;
  uint8_t ulEgdeSubBandOffset;
  uint8_t ulEdgeSubBandwidth;
} g_frSoftUplinkDefaultConfiguration[] = {
  { 1, 15, 0, 5},
  { 2, 15, 5, 5},
  { 3, 15, 10, 5},
  { 1, 25, 0, 8},
  { 2, 25, 8, 8},
  { 3, 25, 16, 9},
  { 1, 50, 0, 16},
  { 2, 50, 16, 16},
  { 3, 50, 32, 18},
  { 1, 75, 0, 24},
  { 2, 75, 24, 24},
  { 3, 75, 48, 27},
  { 1, 100, 0, 32},
  { 2, 100, 32, 32},
  { 3, 100, 64, 36}
};

const uint16_t NUM_DOWNLINK_CONFS (sizeof (g_frSoftDownlinkDefaultConfiguration) / sizeof (FrSoftDownlinkDefaultConfiguration));
const uint16_t NUM_UPLINK_CONFS (sizeof (g_frSoftUplinkDefaultConfiguration) / sizeof (FrSoftUplinkDefaultConfiguration));


NrFrSoftAlgorithm::NrFrSoftAlgorithm ()
  : m_ffrSapUser (0),
    m_ffrRrcSapUser (0),
    m_dlEgdeSubBandOffset (0),
    m_dlEdgeSubBandwidth (0),
    m_ulEgdeSubBandOffset (0),
    m_ulEdgeSubBandwidth (0),
    m_measId (0)
{
  NS_LOG_FUNCTION (this);
  m_ffrSapProvider = new MemberNrFfrSapProvider<NrFrSoftAlgorithm> (this);
  m_ffrRrcSapProvider = new MemberNrFfrRrcSapProvider<NrFrSoftAlgorithm> (this);
}


NrFrSoftAlgorithm::~NrFrSoftAlgorithm ()
{
  NS_LOG_FUNCTION (this);
}


void
NrFrSoftAlgorithm::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ffrSapProvider;
  delete m_ffrRrcSapProvider;
}


TypeId
NrFrSoftAlgorithm::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrFrSoftAlgorithm")
    .SetParent<NrFfrAlgorithm> ()
    .SetGroupName("Nr")
    .AddConstructor<NrFrSoftAlgorithm> ()
    .AddAttribute ("UlEdgeSubBandOffset",
                   "Uplink Edge SubBand Offset in number of Resource Block Groups",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrFrSoftAlgorithm::m_ulEgdeSubBandOffset),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("UlEdgeSubBandwidth",
                   "Uplink Edge SubBandwidth Configuration in number of Resource Block Groups",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrFrSoftAlgorithm::m_ulEdgeSubBandwidth),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlEdgeSubBandOffset",
                   "Downlink Edge SubBand Offset in number of Resource Block Groups",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrFrSoftAlgorithm::m_dlEgdeSubBandOffset),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlEdgeSubBandwidth",
                   "Downlink Edge SubBandwidth Configuration in number of Resource Block Groups",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrFrSoftAlgorithm::m_dlEdgeSubBandwidth),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("AllowCenterUeUseEdgeSubBand",
                   "If true center UEs can receive on Edge SubBand RBGs",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrFrSoftAlgorithm::m_isEdgeSubBandForCenterUe),
                   MakeBooleanChecker ())
    .AddAttribute ("RsrqThreshold",
                   "If the RSRQ of is worse than this threshold, UE should be served in Edge sub-band",
                   UintegerValue (20),
                   MakeUintegerAccessor (&NrFrSoftAlgorithm::m_egdeSubBandThreshold),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("CenterPowerOffset",
                   "PdschConfigDedicated::Pa value for Edge Sub-band, default value dB0",
                   UintegerValue (5),
                   MakeUintegerAccessor (&NrFrSoftAlgorithm::m_centerPowerOffset),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("EdgePowerOffset",
                   "PdschConfigDedicated::Pa value for Edge Sub-band, default value dB0",
                   UintegerValue (5),
                   MakeUintegerAccessor (&NrFrSoftAlgorithm::m_edgePowerOffset),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("CenterAreaTpc",
                   "TPC value which will be set in DL-DCI for UEs in center area"
                   "Absolute mode is used, default value 1 is mapped to -1 according to"
                   "TS36.213 Table 5.1.1.1-2",
                   UintegerValue (1),
                   MakeUintegerAccessor (&NrFrSoftAlgorithm::m_centerAreaTpc),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("EdgeAreaTpc",
                   "TPC value which will be set in DL-DCI for UEs in edge area"
                   "Absolute mode is used, default value 1 is mapped to -1 according to"
                   "TS36.213 Table 5.1.1.1-2",
                   UintegerValue (1),
                   MakeUintegerAccessor (&NrFrSoftAlgorithm::m_edgeAreaTpc),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}


void
NrFrSoftAlgorithm::SetNrFfrSapUser (NrFfrSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ffrSapUser = s;
}


NrFfrSapProvider*
NrFrSoftAlgorithm::GetNrFfrSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrSapProvider;
}

void
NrFrSoftAlgorithm::SetNrFfrRrcSapUser (NrFfrRrcSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ffrRrcSapUser = s;
}


NrFfrRrcSapProvider*
NrFrSoftAlgorithm::GetNrFfrRrcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrRrcSapProvider;
}


void
NrFrSoftAlgorithm::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  NrFfrAlgorithm::DoInitialize ();

  NS_ASSERT_MSG (m_dlBandwidth > 14,"DlBandwidth must be at least 15 to use FFR algorithms");
  NS_ASSERT_MSG (m_ulBandwidth > 14,"UlBandwidth must be at least 15 to use FFR algorithms");

  if (m_frCellTypeId != 0)
    {
      SetDownlinkConfiguration (m_frCellTypeId, m_dlBandwidth);
      SetUplinkConfiguration (m_frCellTypeId, m_ulBandwidth);
    }

  NS_LOG_LOGIC (this << " requesting Event A1 measurements"
                     << " (threshold = 0" << ")");
  NrRrcSap::ReportConfigEutra reportConfig;
  reportConfig.eventId = NrRrcSap::ReportConfigEutra::EVENT_A1;
  reportConfig.threshold1.choice = NrRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
  reportConfig.threshold1.range = 0;
  reportConfig.triggerQuantity = NrRrcSap::ReportConfigEutra::RSRQ;
  reportConfig.reportInterval = NrRrcSap::ReportConfigEutra::MS120;
  m_measId = m_ffrRrcSapUser->AddUeMeasReportConfigForFfr (reportConfig);
}

void
NrFrSoftAlgorithm::Reconfigure ()
{
  NS_LOG_FUNCTION (this);
  if (m_frCellTypeId != 0)
    {
      SetDownlinkConfiguration (m_frCellTypeId, m_dlBandwidth);
      SetUplinkConfiguration (m_frCellTypeId, m_ulBandwidth);
    }
  InitializeDownlinkRbgMaps ();
  InitializeUplinkRbgMaps ();
  m_needReconfiguration = false;
}

void
NrFrSoftAlgorithm::SetDownlinkConfiguration (uint16_t cellId, uint8_t bandwidth)
{
  NS_LOG_FUNCTION (this);
  for (uint16_t i = 0; i < NUM_DOWNLINK_CONFS; ++i)
    {
      if ((g_frSoftDownlinkDefaultConfiguration[i].cellId == cellId)
          && g_frSoftDownlinkDefaultConfiguration[i].dlBandwidth == m_dlBandwidth)
        {
          m_dlEgdeSubBandOffset = g_frSoftDownlinkDefaultConfiguration[i].dlEgdeSubBandOffset;
          m_dlEdgeSubBandwidth = g_frSoftDownlinkDefaultConfiguration[i].dlEdgeSubBandwidth;
        }
    }
}

void
NrFrSoftAlgorithm::SetUplinkConfiguration (uint16_t cellId, uint8_t bandwidth)
{
  NS_LOG_FUNCTION (this);
  for (uint16_t i = 0; i < NUM_UPLINK_CONFS; ++i)
    {
      if ((g_frSoftUplinkDefaultConfiguration[i].cellId == cellId)
          && g_frSoftUplinkDefaultConfiguration[i].ulBandwidth == m_ulBandwidth)
        {
          m_ulEgdeSubBandOffset = g_frSoftUplinkDefaultConfiguration[i].ulEgdeSubBandOffset;
          m_ulEdgeSubBandwidth = g_frSoftUplinkDefaultConfiguration[i].ulEdgeSubBandwidth;
        }
    }
}

void
NrFrSoftAlgorithm::InitializeDownlinkRbgMaps ()
{
  m_dlRbgMap.clear ();
  m_dlEdgeRbgMap.clear ();

  int rbgSize = GetRbgSize (m_dlBandwidth);
  m_dlRbgMap.resize (m_dlBandwidth / rbgSize, false);
  m_dlEdgeRbgMap.resize (m_dlBandwidth / rbgSize, false);

  NS_ASSERT_MSG (m_dlEgdeSubBandOffset <= m_dlBandwidth,"DlEgdeSubBandOffset higher than DlBandwidth");
  NS_ASSERT_MSG (m_dlEdgeSubBandwidth <= m_dlBandwidth,"DlEdgeSubBandwidth higher than DlBandwidth");
  NS_ASSERT_MSG ((m_dlEgdeSubBandOffset + m_dlEdgeSubBandwidth) <= m_dlBandwidth,
                 "(DlEgdeSubBandOffset+DlEdgeSubBandwidth) higher than DlBandwidth");

  for (uint8_t i = m_dlEgdeSubBandOffset / rbgSize;
       i < (m_dlEgdeSubBandOffset + m_dlEdgeSubBandwidth) / rbgSize; i++)
    {
      m_dlEdgeRbgMap[i] = true;
    }
}

void
NrFrSoftAlgorithm::InitializeUplinkRbgMaps ()
{
  m_ulRbgMap.clear ();
  m_ulEdgeRbgMap.clear ();

  m_ulRbgMap.resize (m_ulBandwidth, false);
  m_ulEdgeRbgMap.resize (m_ulBandwidth, false);

  NS_ASSERT_MSG (m_ulEgdeSubBandOffset <= m_dlBandwidth,"UlEgdeSubBandOffset higher than DlBandwidth");
  NS_ASSERT_MSG (m_ulEdgeSubBandwidth <= m_dlBandwidth,"UlEdgeSubBandwidth higher than DlBandwidth");
  NS_ASSERT_MSG ((m_ulEgdeSubBandOffset + m_ulEdgeSubBandwidth) <= m_dlBandwidth,
                 "(UlEgdeSubBandOffset+UlEdgeSubBandwidth) higher than DlBandwidth");

  for (uint8_t i = m_ulEgdeSubBandOffset; i < (m_ulEgdeSubBandOffset + m_ulEdgeSubBandwidth); i++)
    {
      m_ulEdgeRbgMap[i] = true;
    }
}

std::vector <bool>
NrFrSoftAlgorithm::DoGetAvailableDlRbg ()
{
  NS_LOG_FUNCTION (this);

  if (m_needReconfiguration)
    {
      Reconfigure ();
    }

  if (m_dlRbgMap.empty ())
    {
      InitializeDownlinkRbgMaps ();
    }

  return m_dlRbgMap;
}

bool
NrFrSoftAlgorithm::DoIsDlRbgAvailableForUe (int rbgId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);

  bool edgeRbg = m_dlEdgeRbgMap[rbgId];

  std::map< uint16_t, uint8_t >::iterator it = m_ues.find (rnti);
  if (it == m_ues.end ())
    {
      m_ues.insert (std::pair< uint16_t, uint8_t > (rnti, AreaUnset));
      return !edgeRbg;
    }

  bool edgeUe = false;
  if (it->second == CellEdge )
    {
      edgeUe = true;
    }

  if (!edgeUe && m_isEdgeSubBandForCenterUe)
    {
      return true;
    }

  return (edgeRbg && edgeUe) || (!edgeRbg && !edgeUe);
}

std::vector <bool>
NrFrSoftAlgorithm::DoGetAvailableUlRbg ()
{
  NS_LOG_FUNCTION (this);

  if (m_ulRbgMap.empty ())
    {
      InitializeUplinkRbgMaps ();
    }

  return m_ulRbgMap;
}

bool
NrFrSoftAlgorithm::DoIsUlRbgAvailableForUe (int rbgId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);

  if (!m_enabledInUplink)
    {
      return true;
    }

  bool edgeRbg = m_ulEdgeRbgMap[rbgId];

  std::map< uint16_t, uint8_t >::iterator it = m_ues.find (rnti);
  if (it == m_ues.end ())
    {
      m_ues.insert (std::pair< uint16_t, uint8_t > (rnti, AreaUnset));
      return !edgeRbg;
    }

  bool edgeUe = false;
  if (it->second == CellEdge )
    {
      edgeUe = true;
    }

  if (!edgeUe && m_isEdgeSubBandForCenterUe)
    {
      return true;
    }

  return (edgeRbg && edgeUe) || (!edgeRbg && !edgeUe);
}

void
NrFrSoftAlgorithm::DoReportDlCqiInfo (const struct NrFfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
NrFrSoftAlgorithm::DoReportUlCqiInfo (const struct NrFfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
NrFrSoftAlgorithm::DoReportUlCqiInfo (std::map <uint16_t, std::vector <double> > ulCqiMap)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

uint8_t
NrFrSoftAlgorithm::DoGetTpc (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);

  if (!m_enabledInUplink)
    {
      return 1;     // 1 is mapped to 0 for Accumulated mode, and to -1 in Absolute mode TS36.213 Table 5.1.1.1-2
    }

  //TS36.213 Table 5.1.1.1-2
  //   TPC   |   Accumulated Mode  |  Absolute Mode
  //------------------------------------------------
  //    0    |         -1          |      -4
  //    1    |          0          |      -1
  //    2    |          1          |       1
  //    3    |          3          |       4
  //------------------------------------------------
  // here Absolute mode is used

  std::map< uint16_t, uint8_t >::iterator it = m_ues.find (rnti);
  if (it == m_ues.end ())
    {
      return 1;
    }

  if (it->second == CellEdge )
    {
      return m_edgeAreaTpc;
    }
  else if (it->second == CellCenter )
    {
      return m_centerAreaTpc;
    }

  return 1;
}

uint8_t
NrFrSoftAlgorithm::DoGetMinContinuousUlBandwidth ()
{
  NS_LOG_FUNCTION (this);

  uint8_t minContinuousUlBandwidth = m_ulBandwidth;

  if (!m_enabledInUplink)
    {
      return minContinuousUlBandwidth;
    }

  uint8_t leftBandwidth = m_ulEgdeSubBandOffset;
  uint8_t centerBandwidth = m_ulEdgeSubBandwidth;
  uint8_t rightBandwidth = m_ulBandwidth - m_ulEdgeSubBandwidth - m_ulEgdeSubBandOffset;

  minContinuousUlBandwidth =
    ((leftBandwidth > 0 ) && (leftBandwidth < minContinuousUlBandwidth)) ? leftBandwidth : minContinuousUlBandwidth;

  minContinuousUlBandwidth =
    ((centerBandwidth > 0 ) && (centerBandwidth < minContinuousUlBandwidth)) ? centerBandwidth : minContinuousUlBandwidth;

  minContinuousUlBandwidth =
    ((rightBandwidth > 0 ) && (rightBandwidth < minContinuousUlBandwidth)) ? rightBandwidth : minContinuousUlBandwidth;

  NS_LOG_INFO ("minContinuousUlBandwidth: " << (int)minContinuousUlBandwidth);

  return minContinuousUlBandwidth;
}

void
NrFrSoftAlgorithm::DoReportUeMeas (uint16_t rnti,
                                    NrRrcSap::MeasResults measResults)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) measResults.measId);
  NS_LOG_INFO ("RNTI :" << rnti << " MeasId: " << (uint16_t) measResults.measId
                        << " RSRP: " << (uint16_t)measResults.rsrpResult
                        << " RSRQ: " << (uint16_t)measResults.rsrqResult);

  if (measResults.measId != m_measId)
    {
      NS_LOG_WARN ("Ignoring measId " << (uint16_t) measResults.measId);
    }
  else
    {
      std::map< uint16_t, uint8_t >::iterator it = m_ues.find (rnti);
      if (it == m_ues.end ())
        {
          m_ues.insert (std::pair< uint16_t, uint8_t > (rnti, AreaUnset));
        }
      it = m_ues.find (rnti);

      if (measResults.rsrqResult < m_egdeSubBandThreshold)
        {
          if (it->second != CellEdge)
            {
              NS_LOG_INFO ("UE RNTI: " << rnti << " will be served in Edge sub-band");
              it->second = CellEdge;

              NrRrcSap::PdschConfigDedicated pdschConfigDedicated;
              pdschConfigDedicated.pa = m_edgePowerOffset;
              m_ffrRrcSapUser->SetPdschConfigDedicated (rnti, pdschConfigDedicated);
            }
        }
      else
        {
          if (it->second != CellCenter)
            {
              NS_LOG_INFO ("UE RNTI: " << rnti << " will be served in Center sub-band");
              it->second = CellCenter;

              NrRrcSap::PdschConfigDedicated pdschConfigDedicated;
              pdschConfigDedicated.pa = m_centerPowerOffset;
              m_ffrRrcSapUser->SetPdschConfigDedicated (rnti, pdschConfigDedicated);
            }
        }
    }
}

void
NrFrSoftAlgorithm::DoRecvLoadInformation (NgcX2Sap::LoadInformationParams params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

} // end of namespace ns3
