/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 CTTC
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
 * Author: Jaume Nin <jaume.nin@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#include "ngc-x2-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NgcX2Tag);

NgcX2Tag::NgcX2Tag ()
  : m_senderTimestamp (Seconds (0))
{
  // Nothing to do here
}


NgcX2Tag::NgcX2Tag (Time senderTimestamp)
  : m_senderTimestamp (senderTimestamp)

{
  // Nothing to do here
}

TypeId
NgcX2Tag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcX2Tag")
    .SetParent<Tag> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcX2Tag> ();
  return tid;
}

TypeId
NgcX2Tag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcX2Tag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
NgcX2Tag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
NgcX2Tag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderTimestamp   = NanoSeconds (senderTimestamp);

}

void
NgcX2Tag::Print (std::ostream &os) const
{
  os << m_senderTimestamp;
}

} // namespace ns3

