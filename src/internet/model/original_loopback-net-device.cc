/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "original_loopback-net-device.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/channel.h"
#include "ns3/node.h"
#include "ns3/packet.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoopbackNetDevice1");

NS_OBJECT_ENSURE_REGISTERED (LoopbackNetDevice1);

TypeId 
LoopbackNetDevice1::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoopbackNetDevice1")
    .SetParent<NetDevice> ()
    .SetGroupName ("Internet")
    .AddConstructor<LoopbackNetDevice1> ()
  ;
  return tid;
}

LoopbackNetDevice1::LoopbackNetDevice1 ()
  : m_node (0),
    m_mtu (0xffff),
    m_ifIndex (0),
    m_address (Mac48Address ("00:00:00:00:00:00"))
{
  NS_LOG_FUNCTION_NOARGS ();
}

void 
LoopbackNetDevice1::Receive (Ptr<Packet> packet, uint16_t protocol,
                            Mac48Address to, Mac48Address from)
{
  NS_LOG_FUNCTION (packet << " " << protocol << " " << to << " " << from);
  NetDevice::PacketType packetType;
  if (to == m_address)
    {
      packetType = NetDevice::PACKET_HOST;
    }
  else if (to.IsBroadcast ())
    {
      packetType = NetDevice::PACKET_HOST;
    }
  else if (to.IsGroup ())
    {
      packetType = NetDevice::PACKET_MULTICAST;
    }
  else 
    {
      packetType = NetDevice::PACKET_OTHERHOST;
    }
  m_rxCallback (this, packet, protocol, from);
  if (!m_promiscCallback.IsNull ())
    {
      m_promiscCallback (this, packet, protocol, from, to, packetType);
    }
}

void 
LoopbackNetDevice1::SetIfIndex (const uint32_t index)
{
  m_ifIndex = index;
}

uint32_t 
LoopbackNetDevice1::GetIfIndex (void) const
{
  return m_ifIndex;
}

Ptr<Channel> 
LoopbackNetDevice1::GetChannel (void) const
{
  return 0;
}

void 
LoopbackNetDevice1::SetAddress (Address address)
{
  m_address = Mac48Address::ConvertFrom (address);
}

Address 
LoopbackNetDevice1::GetAddress (void) const
{
  return m_address;
}

bool 
LoopbackNetDevice1::SetMtu (const uint16_t mtu)
{
  m_mtu = mtu;
  return true;
}

uint16_t 
LoopbackNetDevice1::GetMtu (void) const
{
  return m_mtu;
}

bool 
LoopbackNetDevice1::IsLinkUp (void) const
{
  return true;
}

void 
LoopbackNetDevice1::AddLinkChangeCallback (Callback<void> callback)
{}

bool 
LoopbackNetDevice1::IsBroadcast (void) const
{
  return true;
}

Address
LoopbackNetDevice1::GetBroadcast (void) const
{
  // This is typically set to all zeros rather than all ones in real systems
  return Mac48Address ("00:00:00:00:00:00");
}

bool 
LoopbackNetDevice1::IsMulticast (void) const
{
  // Multicast loopback will need to be supported for outgoing 
  // datagrams but this will probably be handled in multicast sockets
  return false;
}

Address 
LoopbackNetDevice1::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address::GetMulticast (multicastGroup);
}

Address LoopbackNetDevice1::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address::GetMulticast (addr);
}

bool 
LoopbackNetDevice1::IsPointToPoint (void) const
{
  return false;
}

bool 
LoopbackNetDevice1::IsBridge (void) const
{
  return false;
}

bool 
LoopbackNetDevice1::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << " " << dest << " " << protocolNumber);
  Mac48Address to = Mac48Address::ConvertFrom (dest);
  NS_ASSERT_MSG (to == GetBroadcast () || to == m_address, "Invalid destination address");
  Simulator::ScheduleWithContext (m_node->GetId (), Seconds (0.0), &LoopbackNetDevice::Receive, this, packet, protocolNumber, to, m_address);
  return true;
}

bool 
LoopbackNetDevice1::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << " " << source << " " << dest << " " << protocolNumber);
  Mac48Address to = Mac48Address::ConvertFrom (dest);
  Mac48Address from = Mac48Address::ConvertFrom (source);
  NS_ASSERT_MSG (to.IsBroadcast () || to == m_address, "Invalid destination address");
  Simulator::ScheduleWithContext (m_node->GetId (), Seconds (0.0), &LoopbackNetDevice::Receive, this, packet, protocolNumber, to, from);
  return true;
}

Ptr<Node> 
LoopbackNetDevice1::GetNode (void) const
{
  return m_node;
}

void 
LoopbackNetDevice1::SetNode (Ptr<Node> node)
{
  m_node = node;
}

bool 
LoopbackNetDevice1::NeedsArp (void) const
{
  return false;
}

void 
LoopbackNetDevice1::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_rxCallback = cb;
}

void
LoopbackNetDevice1::DoDispose (void)
{
  m_node = 0;
  NetDevice::DoDispose ();
}


void
LoopbackNetDevice1::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  m_promiscCallback = cb;
}

bool
LoopbackNetDevice1::SupportsSendFrom (void) const
{
  return true;
}

} // namespace ns3
