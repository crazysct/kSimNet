/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 */

#include "original_channel.h"
#include "original_channel-list.h" //sjkang_x
#include "original_net-device.h"

#include "ns3/log.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Channel1");

NS_OBJECT_ENSURE_REGISTERED (Channel1);

TypeId 
Channel1::GetTypeId (void) //sjkang_x
//Channel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Channel1")
    .SetParent<Object> ()
    .SetGroupName("Network")
    .AddAttribute ("Id", "The id (unique integer) of this Channel.",
                   TypeId::ATTR_GET,
                   UintegerValue (0),
                   MakeUintegerAccessor (&Channel1::m_id),
                   MakeUintegerChecker<uint32_t> ());
  return tid;
}

Channel1::Channel1 ()
  : m_id (0)
{
  NS_LOG_FUNCTION (this);
 //m_id = ChannelList::Add(this);
  m_id = ChannelList1::Add (this); 
}

Channel1::~Channel1 ()
{
  NS_LOG_FUNCTION (this);
}

uint32_t 
Channel1::GetId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_id;
}

} // namespace ns3
