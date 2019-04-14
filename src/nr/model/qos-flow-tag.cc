/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011,2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *         Nicola Baldo <nbaldo@cttc.es>
 */


#include "qos-flow-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (QosFlowTag);

TypeId
QosFlowTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QosFlowTag")
    .SetParent<Tag> ()
    .SetGroupName("Nr")
    .AddConstructor<QosFlowTag> ()
    .AddAttribute ("rnti", "The rnti that indicates the UE which packet belongs",
                   UintegerValue (0),
                   MakeUintegerAccessor (&QosFlowTag::GetRnti),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("bid", "The QoS Flow id within the UE to which the packet belongs",
                   UintegerValue (0),
                   MakeUintegerAccessor (&QosFlowTag::GetBid),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

TypeId
QosFlowTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

QosFlowTag::QosFlowTag ()
  : m_rnti (0),
    m_bid (0)
{
}
QosFlowTag::QosFlowTag (uint16_t rnti, uint8_t bid)
  : m_rnti (rnti),
    m_bid (bid)
{
}

void
QosFlowTag::SetRnti (uint16_t rnti)
{
  m_rnti = rnti;
}

void
QosFlowTag::SetBid (uint8_t bid)
{
  m_bid = bid;
}

uint32_t
QosFlowTag::GetSerializedSize (void) const
{
  return 3;
}

void
QosFlowTag::Serialize (TagBuffer i) const
{
  i.WriteU16 (m_rnti);
  i.WriteU8 (m_bid);
}

void
QosFlowTag::Deserialize (TagBuffer i)
{
  m_rnti = (uint16_t) i.ReadU16 ();
  m_bid = (uint8_t) i.ReadU8 ();
}

uint16_t
QosFlowTag::GetRnti () const
{
  return m_rnti;
}

uint8_t
QosFlowTag::GetBid () const
{
  return m_bid;
}

void
QosFlowTag::Print (std::ostream &os) const
{
  os << "rnti=" << m_rnti << ", bid=" << (uint16_t) m_bid;
}

} // namespace ns3
