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

#include "original_net-device-container.h"
#include "ns3/names.h"

namespace ns3 {

NetDeviceContainer1::NetDeviceContainer1 ()
{
}
NetDeviceContainer1::NetDeviceContainer1 (Ptr<NetDevice1> dev)
{
  m_devices.push_back (dev);
}
NetDeviceContainer1::NetDeviceContainer1 (std::string devName)
{
  Ptr<NetDevice1> dev = Names::Find<NetDevice1> (devName);
  m_devices.push_back (dev);
}
NetDeviceContainer1::NetDeviceContainer1 (const NetDeviceContainer1 &a, const NetDeviceContainer1 &b)
{
  *this = a;
  Add (b);
}


NetDeviceContainer1::Iterator 
NetDeviceContainer1::Begin (void) const
{
  return m_devices.begin ();
}
NetDeviceContainer1::Iterator 
NetDeviceContainer1::End (void) const
{
  return m_devices.end ();
}

uint32_t 
NetDeviceContainer1::GetN (void) const
{
  return m_devices.size ();
}
Ptr<NetDevice1> 
NetDeviceContainer1::Get (uint32_t i) const
{
  return m_devices[i];
}
void 
NetDeviceContainer1::Add (NetDeviceContainer1 other)
{
  for (Iterator i = other.Begin (); i != other.End (); i++)
    {
      m_devices.push_back (*i);
    }
}
void 
NetDeviceContainer1::Add (Ptr<NetDevice1> device)
{
  m_devices.push_back (device);
}
void 
NetDeviceContainer1::Add (std::string deviceName)
{
  Ptr<NetDevice1> device = Names::Find<NetDevice1> (deviceName);
  m_devices.push_back (device);
}

} // namespace ns3
