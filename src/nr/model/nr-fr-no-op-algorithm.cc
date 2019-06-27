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

#include "nr-fr-no-op-algorithm.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrFrNoOpAlgorithm");

NS_OBJECT_ENSURE_REGISTERED (NrFrNoOpAlgorithm);


NrFrNoOpAlgorithm::NrFrNoOpAlgorithm ()
  : m_ffrSapUser (0), m_ffrRrcSapUser (0)
{
  NS_LOG_FUNCTION (this);
  m_ffrSapProvider = new MemberNrFfrSapProvider<NrFrNoOpAlgorithm> (this);
  m_ffrRrcSapProvider = new MemberNrFfrRrcSapProvider<NrFrNoOpAlgorithm> (this);
}


NrFrNoOpAlgorithm::~NrFrNoOpAlgorithm ()
{
  NS_LOG_FUNCTION (this);
}


void
NrFrNoOpAlgorithm::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ffrSapProvider;
  delete m_ffrRrcSapProvider;
}


TypeId
NrFrNoOpAlgorithm::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrFrNoOpAlgorithm")
    .SetParent<NrFfrAlgorithm> ()
    .SetGroupName("Nr")
    .AddConstructor<NrFrNoOpAlgorithm> ()
  ;
  return tid;
}


void
NrFrNoOpAlgorithm::SetNrFfrSapUser (NrFfrSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ffrSapUser = s;
}


NrFfrSapProvider*
NrFrNoOpAlgorithm::GetNrFfrSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrSapProvider;
}

void
NrFrNoOpAlgorithm::SetNrFfrRrcSapUser (NrFfrRrcSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ffrRrcSapUser = s;
}


NrFfrRrcSapProvider*
NrFrNoOpAlgorithm::GetNrFfrRrcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrRrcSapProvider;
}


void
NrFrNoOpAlgorithm::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  NrFfrAlgorithm::DoInitialize ();
}

void
NrFrNoOpAlgorithm::Reconfigure ()
{
  NS_LOG_FUNCTION (this);
}

std::vector <bool>
NrFrNoOpAlgorithm::DoGetAvailableDlRbg ()
{
  NS_LOG_FUNCTION (this);
  std::vector <bool> rbgMap;
  int rbgSize = GetRbgSize (m_dlBandwidth);
  rbgMap.resize (m_dlBandwidth/rbgSize, false);
  return rbgMap;
}

bool
NrFrNoOpAlgorithm::DoIsDlRbgAvailableForUe (int i, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  return true;
}

std::vector <bool>
NrFrNoOpAlgorithm::DoGetAvailableUlRbg ()
{
  NS_LOG_FUNCTION (this);
  std::vector <bool> rbgMap;
  rbgMap.resize (m_ulBandwidth, false);
  return rbgMap;
}

bool
NrFrNoOpAlgorithm::DoIsUlRbgAvailableForUe (int i, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
NrFrNoOpAlgorithm::DoReportDlCqiInfo (const struct NrFfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
NrFrNoOpAlgorithm::DoReportUlCqiInfo (const struct NrFfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
NrFrNoOpAlgorithm::DoReportUlCqiInfo (std::map <uint16_t, std::vector <double> > ulCqiMap)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

uint8_t
NrFrNoOpAlgorithm::DoGetTpc (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  return 1; // 1 is mapped to 0 for Accumulated mode, and to -1 in Absolute mode TS36.213 Table 5.1.1.1-2
}

uint8_t
NrFrNoOpAlgorithm::DoGetMinContinuousUlBandwidth ()
{
  NS_LOG_FUNCTION (this);
  return m_ulBandwidth;
}

void
NrFrNoOpAlgorithm::DoReportUeMeas (uint16_t rnti,
                                    NrRrcSap::MeasResults measResults)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) measResults.measId);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
NrFrNoOpAlgorithm::DoRecvLoadInformation (NgcX2Sap::LoadInformationParams params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

} // end of namespace ns3
