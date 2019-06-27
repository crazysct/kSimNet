/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 *         Nicola Baldo <nbaldo@cttc.es>
 */

#include "ns3/llc-snap-header.h"
#include "ns3/simulator.h"
#include "ns3/callback.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "nr-net-device.h"
#include "ns3/packet-burst.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "nr-amc.h"
#include "ns3/ipv4-header.h"
#include <ns3/nr-radio-bearer-tag.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrNetDevice");

NS_OBJECT_ENSURE_REGISTERED ( NrNetDevice);

////////////////////////////////
// NrNetDevice
////////////////////////////////

TypeId NrNetDevice::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::NrNetDevice")

    .SetParent<NetDevice> ()

    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (30000),
                   MakeUintegerAccessor (&NrNetDevice::SetMtu,
                                         &NrNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> ())
  ;
  return tid;
}

NrNetDevice::NrNetDevice (void)
{
  NS_LOG_FUNCTION (this);
}


NrNetDevice::~NrNetDevice (void)
{
  NS_LOG_FUNCTION (this);
}


void
NrNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_node = 0;
  NetDevice::DoDispose ();
}


Ptr<Channel>
NrNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  // we can't return a meaningful channel here, because NR devices using FDD have actually two channels.
  return 0;
}


void
NrNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
}


Address
NrNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_address;
}


void
NrNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}


Ptr<Node>
NrNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}


void
NrNetDevice::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_rxCallback = cb;
}


bool
NrNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_FATAL_ERROR ("SendFrom () not supported");
  return false;
}


bool
NrNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}



bool
NrNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  m_mtu = mtu;
  return true;
}

uint16_t
NrNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}


void
NrNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}

uint32_t
NrNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}


bool
NrNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_linkUp;
}


bool
NrNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
NrNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address::GetBroadcast ();
}

bool
NrNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}


bool
NrNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}


bool
NrNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}


bool
NrNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

Address
NrNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this << multicastGroup);

  Mac48Address ad = Mac48Address::GetMulticast (multicastGroup);

  //
  // Implicit conversion (operator Address ()) is defined for Mac48Address, so
  // use it by just returning the EUI-48 address which is automagically converted
  // to an Address.
  //
  NS_LOG_LOGIC ("multicast address is " << ad);

  return ad;
}

Address
NrNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  Mac48Address ad = Mac48Address::GetMulticast (addr);

  NS_LOG_LOGIC ("MAC IPv6 multicast address is " << ad);
  return ad;
}

void
NrNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  m_linkChangeCallbacks.ConnectWithoutContext (callback);
}


void
NrNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Promisc mode not supported");
}



void
NrNetDevice::Receive (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  m_rxCallback (this, p, Ipv4L3Protocol::PROT_NUMBER, Address ());
}


}
