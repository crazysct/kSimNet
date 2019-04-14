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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */


#include "nr-ff-mac-scheduler.h"
#include <ns3/log.h>
#include <ns3/enum.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrFfMacScheduler");

NS_OBJECT_ENSURE_REGISTERED (NrFfMacScheduler);


NrFfMacScheduler::NrFfMacScheduler ()
: m_ulCqiFilter (ALL_UL_CQI)
{
  NS_LOG_FUNCTION (this);
}


NrFfMacScheduler::~NrFfMacScheduler ()
{
  NS_LOG_FUNCTION (this);
}

void
NrFfMacScheduler::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrFfMacScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrFfMacScheduler")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddAttribute ("UlCqiFilter",
                   "The filter to apply on UL CQIs received",
                   EnumValue (NrFfMacScheduler::ALL_UL_CQI),
                   MakeEnumAccessor (&NrFfMacScheduler::m_ulCqiFilter),
                   MakeEnumChecker (NrFfMacScheduler::SRS_UL_CQI, "SRS_UL_CQI",
                                    NrFfMacScheduler::PUSCH_UL_CQI, "PUSCH_UL_CQI",
                                    NrFfMacScheduler::ALL_UL_CQI, "ALL_UL_CQI"))
    ;
  return tid;
}


} // namespace ns3



