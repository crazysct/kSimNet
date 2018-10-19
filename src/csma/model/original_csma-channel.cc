/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Emmanuelle Laprise
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
 * Author: Emmanuelle Laprise <emmanuelle.laprise@bluekazoo.ca>
 */

#include "ns3/original_csma-channel.h"
#include "ns3/original_csma-net-device.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CsmaChannel1");

NS_OBJECT_ENSURE_REGISTERED (CsmaChannel1);

TypeId
CsmaChannel1::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CsmaChannel1")
    .SetParent<Channel1> ()
    .SetGroupName ("Csma")
    .AddConstructor<CsmaChannel1> ()
    .AddAttribute ("DataRate", 
                   "The transmission data rate to be provided to devices connected to the channel",
                   DataRateValue (DataRate (0xffffffff)),
                   MakeDataRateAccessor (&CsmaChannel1::m_bps),
                   MakeDataRateChecker ())
    .AddAttribute ("Delay", "Transmission delay through the channel",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&CsmaChannel1::m_delay),
                   MakeTimeChecker ())
  ;
  return tid;
}

CsmaChannel1::CsmaChannel1 ()
  :
    Channel1 ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_state = IDLE1;
  m_deviceList.clear ();
}

CsmaChannel1::~CsmaChannel1 ()
{
  NS_LOG_FUNCTION (this);
  m_deviceList.clear ();
}

int32_t
CsmaChannel1::Attach (Ptr<CsmaNetDevice1> device)
{
  NS_LOG_FUNCTION (this << device);
  NS_ASSERT (device != 0);

  CsmaDeviceRec1 rec (device);

  m_deviceList.push_back (rec);
  return (m_deviceList.size () - 1);
}

bool
CsmaChannel1::Reattach (Ptr<CsmaNetDevice1> device)
{
  NS_LOG_FUNCTION (this << device);
  NS_ASSERT (device != 0);

  std::vector<CsmaDeviceRec1>::iterator it;
  for (it = m_deviceList.begin (); it < m_deviceList.end ( ); it++)
    {
      if (it->devicePtr == device) 
        {
          if (!it->active) 
            {
              it->active = true;
              return true;
            } 
          else 
            {
              return false;
            }
        }
    }
  return false;
}

bool
CsmaChannel1::Reattach (uint32_t deviceId)
{
  NS_LOG_FUNCTION (this << deviceId);

  if (deviceId < m_deviceList.size ())
    {
      return false;
    }

  if (m_deviceList[deviceId].active)
    {
      return false;
    } 
  else 
    {
      m_deviceList[deviceId].active = true;
      return true;
    }
}

bool
CsmaChannel1::Detach (uint32_t deviceId)
{
  NS_LOG_FUNCTION (this << deviceId);

  if (deviceId < m_deviceList.size ())
    {
      if (!m_deviceList[deviceId].active)
        {
          NS_LOG_WARN ("CsmaChannel::Detach(): Device is already detached (" << deviceId << ")");
          return false;
        }

      m_deviceList[deviceId].active = false;

      if ((m_state == TRANSMITTING1) && (m_currentSrc == deviceId))
        {
          NS_LOG_WARN ("CsmaChannel::Detach(): Device is currently" << "transmitting (" << deviceId << ")");
        }

      return true;
    } 
  else 
    {
      return false;
    }
}

bool
CsmaChannel1::Detach (Ptr<CsmaNetDevice1> device)
{
  NS_LOG_FUNCTION (this << device);
  NS_ASSERT (device != 0);

  std::vector<CsmaDeviceRec1>::iterator it;
  for (it = m_deviceList.begin (); it < m_deviceList.end (); it++) 
    {
      if ((it->devicePtr == device) && (it->active)) 
        {
          it->active = false;
          return true;
        }
    }
  return false;
}

bool
CsmaChannel1::TransmitStart (Ptr<Packet> p, uint32_t srcId)
{
  NS_LOG_FUNCTION (this << p << srcId);
  NS_LOG_INFO ("UID is " << p->GetUid () << ")");

  if (m_state != IDLE1)
    {
      NS_LOG_WARN ("CsmaChannel::TransmitStart(): State is not IDLE1");
      return false;
    }

  if (!IsActive (srcId))
    {
      NS_LOG_ERROR ("CsmaChannel::TransmitStart(): Seclected source is not currently attached to network");
      return false;
    }

  NS_LOG_LOGIC ("switch to TRANSMITTING1");
  m_currentPkt = p;
  m_currentSrc = srcId;
  m_state = TRANSMITTING1;
  return true;
}

bool
CsmaChannel1::IsActive (uint32_t deviceId)
{
  return (m_deviceList[deviceId].active);
}

bool
CsmaChannel1::TransmitEnd ()
{
  NS_LOG_FUNCTION (this << m_currentPkt << m_currentSrc);
  NS_LOG_INFO ("UID is " << m_currentPkt->GetUid () << ")");

  NS_ASSERT (m_state == TRANSMITTING1);
  m_state = PROPAGATING1;

  bool retVal = true;

  if (!IsActive (m_currentSrc))
    {
      NS_LOG_ERROR ("CsmaChannel::TransmitEnd(): Seclected source was detached before the end of the transmission");
      retVal = false;
    }

  NS_LOG_LOGIC ("Schedule event in " << m_delay.GetSeconds () << " sec");


  NS_LOG_LOGIC ("Receive");

  std::vector<CsmaDeviceRec1>::iterator it;
  uint32_t devId = 0;
  for (it = m_deviceList.begin (); it < m_deviceList.end (); it++)
    {
      if (it->IsActive ())
        {
          // schedule reception events
          Simulator::ScheduleWithContext (it->devicePtr->GetNode ()->GetId (),
                                          m_delay,
                                          &CsmaNetDevice1::Receive, it->devicePtr,
                                          m_currentPkt->Copy (), m_deviceList[m_currentSrc].devicePtr);
        }
      devId++;
    }

  // also schedule for the tx side to go back to IDLE1
  Simulator::Schedule (m_delay, &CsmaChannel1::PropagationCompleteEvent,
                       this);
  return retVal;
}

void
CsmaChannel1::PropagationCompleteEvent ()
{
  NS_LOG_FUNCTION (this << m_currentPkt);
  NS_LOG_INFO ("UID is " << m_currentPkt->GetUid () << ")");

  NS_ASSERT (m_state == PROPAGATING1);
  m_state = IDLE1;
}

uint32_t
CsmaChannel1::GetNumActDevices (void)
{
  int numActDevices = 0;
  std::vector<CsmaDeviceRec1>::iterator it;
  for (it = m_deviceList.begin (); it < m_deviceList.end (); it++) 
    {
      if (it->active)
        {
          numActDevices++;
        }
    }
  return numActDevices;
}

uint32_t
CsmaChannel1::GetNDevices (void) const
{
  return (m_deviceList.size ());
}

Ptr<CsmaNetDevice1>
CsmaChannel1::GetCsmaDevice (uint32_t i) const
{
  Ptr<CsmaNetDevice1> netDevice = m_deviceList[i].devicePtr;
  return netDevice;
}

int32_t
CsmaChannel1::GetDeviceNum (Ptr<CsmaNetDevice1> device)
{
  std::vector<CsmaDeviceRec1>::iterator it;
  int i = 0;
  for (it = m_deviceList.begin (); it < m_deviceList.end (); it++) 
    {
      if (it->devicePtr == device)
        {
          if (it->active) 
            {
              return i;
            } 
          else 
            {
              return -2;
            }
        }
      i++;
    }
  return -1;
}

bool
CsmaChannel1::IsBusy (void)
{
  if (m_state == IDLE1) 
    {
      return false;
    } 
  else 
    {
      return true;
    }
}

DataRate
CsmaChannel1::GetDataRate (void)
{
  return m_bps;
}

Time
CsmaChannel1::GetDelay (void)
{
  return m_delay;
}

WireState1
CsmaChannel1::GetState (void)
{
  return m_state;
}

Ptr<NetDevice1>
CsmaChannel1::GetDevice (uint32_t i) const
{
  return GetCsmaDevice (i);
}

CsmaDeviceRec1::CsmaDeviceRec1 ()
{
  active = false;
}

CsmaDeviceRec1::CsmaDeviceRec1 (Ptr<CsmaNetDevice1> device)
{
  devicePtr = device; 
  active = true;
}

CsmaDeviceRec1::CsmaDeviceRec1 (CsmaDeviceRec1 const &deviceRec)
{
  devicePtr = deviceRec.devicePtr;
  active = deviceRec.active;
}

bool
CsmaDeviceRec1::IsActive () 
{
  return active;
}

} // namespace ns3
