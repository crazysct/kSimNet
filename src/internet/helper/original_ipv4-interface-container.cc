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
 * Author: Mathieu Lacage <mathieu.lacage@cutebugs.net>
 */

#include "original_ipv4-interface-container.h"
#include "ns3/node-list.h"
#include "ns3/names.h"

namespace ns3 {

Ipv4InterfaceContainer1::Ipv4InterfaceContainer1 ()
{
}

void
Ipv4InterfaceContainer1::Add (const Ipv4InterfaceContainer1& other)
{
  for (InterfaceVector::const_iterator i = other.m_interfaces.begin (); i != other.m_interfaces.end (); i++)
    {
      m_interfaces.push_back (*i);
    }
}

Ipv4InterfaceContainer1::Iterator
Ipv4InterfaceContainer1::Begin (void) const
{
  return m_interfaces.begin ();
}

Ipv4InterfaceContainer1::Iterator
Ipv4InterfaceContainer1::End (void) const
{
  return m_interfaces.end ();
}

uint32_t
Ipv4InterfaceContainer1::GetN (void) const
{
  return m_interfaces.size ();
}

Ipv4Address
Ipv4InterfaceContainer1::GetAddress (uint32_t i, uint32_t j) const
{
  Ptr<Ipv41> ipv4 = m_interfaces[i].first;
  uint32_t interface = m_interfaces[i].second;
  return ipv4->GetAddress (interface, j).GetLocal ();
}

void 
Ipv4InterfaceContainer1::SetMetric (uint32_t i, uint16_t metric)
{
  Ptr<Ipv41> ipv4 = m_interfaces[i].first;
  uint32_t interface = m_interfaces[i].second;
  ipv4->SetMetric (interface, metric);
}
void 
Ipv4InterfaceContainer1::Add (Ptr<Ipv41> ipv4, uint32_t interface)
{
  m_interfaces.push_back (std::make_pair (ipv4, interface));
}
void Ipv4InterfaceContainer1::Add (std::pair<Ptr<Ipv41>, uint32_t> a)
{
  Add (a.first, a.second);
}
void 
Ipv4InterfaceContainer1::Add (std::string ipv4Name, uint32_t interface)
{
  Ptr<Ipv41> ipv4 = Names::Find<Ipv41> (ipv4Name);
  m_interfaces.push_back (std::make_pair (ipv4, interface));
}

std::pair<Ptr<Ipv41>, uint32_t>
Ipv4InterfaceContainer1::Get (uint32_t i) const
{
  return m_interfaces[i];
}


} // namespace ns3
