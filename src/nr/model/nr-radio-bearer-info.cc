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

#include "nr-radio-bearer-info.h"
#include "nr-ue-rrc.h"
#include "nr-rlc.h"
#include "nr-pdcp.h"

#include <ns3/log.h>



namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrRadioBearerInfo);

NrRadioBearerInfo::NrRadioBearerInfo (void)
{
}

NrRadioBearerInfo::~NrRadioBearerInfo (void)
{
}
  
TypeId 
NrRadioBearerInfo::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::NrRadioBearerInfo")
    .SetParent<Object> ()
    .AddConstructor<NrRadioBearerInfo> ()
    ;
  return tid;
}
  
  
TypeId 
NrDataRadioBearerInfo::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::NrDataRadioBearerInfo")
    .SetParent<NrRadioBearerInfo> ()
    .AddConstructor<NrDataRadioBearerInfo> ()
    .AddAttribute ("DrbIdentity", "The id of this Data Radio Bearer",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0), // unused (attribute is read-only
                   MakeUintegerAccessor (&NrDataRadioBearerInfo::m_drbIdentity),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("EpsBearerIdentity", "The id of the EPS bearer corresponding to this Data Radio Bearer",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0), // unused (attribute is read-only
                   MakeUintegerAccessor (&NrDataRadioBearerInfo::m_epsBearerIdentity),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("logicalChannelIdentity", "The id of the Logical Channel corresponding to this Data Radio Bearer",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0), // unused (attribute is read-only
                   MakeUintegerAccessor (&NrDataRadioBearerInfo::m_logicalChannelIdentity),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("NrRlc", "RLC instance of the radio bearer.",
                   PointerValue (),
                   MakePointerAccessor (&NrRadioBearerInfo::m_rlc),
                   MakePointerChecker<NrRlc> ())
    .AddAttribute ("NrPdcp", "PDCP instance of the radio bearer.",
                   PointerValue (),
                   MakePointerAccessor (&NrRadioBearerInfo::m_pdcp),
                   MakePointerChecker<NrPdcp> ())
    ;
  return tid;
}


TypeId 
NrSignalingRadioBearerInfo::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::NrSignalingRadioBearerInfo")
    .SetParent<NrRadioBearerInfo> ()
    .AddConstructor<NrSignalingRadioBearerInfo> ()
    .AddAttribute ("SrbIdentity", "The id of this Signaling Radio Bearer",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0), // unused (attribute is read-only
                   MakeUintegerAccessor (&NrSignalingRadioBearerInfo::m_srbIdentity),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("NrRlc", "RLC instance of the radio bearer.",
                   PointerValue (),
                   MakePointerAccessor (&NrRadioBearerInfo::m_rlc),
                   MakePointerChecker<NrRlc> ())
    .AddAttribute ("NrPdcp", "PDCP instance of the radio bearer.",
                   PointerValue (),
                   MakePointerAccessor (&NrRadioBearerInfo::m_pdcp),
                   MakePointerChecker<NrPdcp> ())
    ;
  return tid;
}

RlcBearerInfo::RlcBearerInfo (void)
{
}

RlcBearerInfo::~RlcBearerInfo (void)
{
}
  
TypeId 
RlcBearerInfo::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::RlcBearerInfo")
    .SetParent<Object> ()
    .AddConstructor<RlcBearerInfo> ()
    .AddAttribute ("NrRlc", "RLC instance of the secondary connection.",
                   PointerValue (),
                   MakePointerAccessor (&RlcBearerInfo::m_rlc),
                   MakePointerChecker<NrRlc> ())
    ;
  return tid;
}




} // namespace ns3
