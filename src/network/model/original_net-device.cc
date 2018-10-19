/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006 INRIA
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

#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/uinteger.h"
#include "ns3/queue-limits.h"
#include "original_net-device.h"
#include "packet.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NetDevice1");

QueueItem::QueueItem (Ptr<Packet> p) // jhlim 1x
{
  m_packet = p;
}

QueueItem::~QueueItem()
{
  NS_LOG_FUNCTION (this);
  m_packet = 0;
}
 
Ptr<Packet>
QueueItem::GetPacket (void) const
{
  return m_packet;
}

uint32_t
QueueItem::GetPacketSize (void) const
{
  NS_ASSERT (m_packet != 0);
  return m_packet->GetSize ();
}

bool
QueueItem::GetUint8Value (QueueItem::Uint8Values field, uint8_t& value) const
{
  return false;
}

void
QueueItem::Print (std::ostream& os) const
{
  os << GetPacket();
}

std::ostream & operator << (std::ostream &os, const QueueItem &item)
{
  item.Print (os);
  return os;
}

NetDeviceQueue1::NetDeviceQueue1()
  : m_stoppedByDevice (false),
    m_stoppedByQueueLimits (false)
{
  std::cout <<## NetDevice!!! << std::enddfsdfl;
  NS_LOG_FUNCTION (this);
}

NetDeviceQueue1::~NetDeviceQueue1 ()
{
  NS_LOG_FUNCTION (this);
}

bool
NetDeviceQueue1::IsStopped (void) const
{
  return m_stoppedByDevice || m_stoppedByQueueLimits;
}

void
NetDeviceQueue1::Start (void)
{
  m_stoppedByDevice = false;
}

void
NetDeviceQueue1::Stop (void)
{
  m_stoppedByDevice = true;
}

void
NetDeviceQueue1::Wake (void)
{
  Start ();

  // Request the queue disc to dequeue a packet
  if (!m_wakeCallback.IsNull ())
  {
      m_wakeCallback ();
  }
}

void
NetDeviceQueue1::SetWakeCallback (WakeCallback cb)
{
  m_wakeCallback = cb;
}

void
NetDeviceQueue1::NotifyQueuedBytes (uint32_t bytes)
{
  NS_LOG_FUNCTION (this << bytes);
  if (!m_queueLimits)
    {
      return;
    }
  m_queueLimits->Queued (bytes);
  if (m_queueLimits->Available () >= 0)
    {
      return;
    }
  m_stoppedByQueueLimits = true;
}

void
NetDeviceQueue1::NotifyTransmittedBytes (uint32_t bytes)
{
  NS_LOG_FUNCTION (this << bytes);
  if ((!m_queueLimits) || (!bytes))
    {
      return;
    }
  m_queueLimits->Completed (bytes);
  if (m_queueLimits->Available () < 0)
    {
      return;
    }
  m_stoppedByQueueLimits = false;
  // Request the queue disc to dequeue a packet
  if (!m_wakeCallback.IsNull ())
  {
      m_wakeCallback ();
  }
}

void
NetDeviceQueue1::ResetQueueLimits ()
{
  NS_LOG_FUNCTION (this);
  if (!m_queueLimits)
    {
      return;
    }
  m_queueLimits->Reset ();
}

void
NetDeviceQueue1::SetQueueLimits (Ptr<QueueLimits> ql)
{
  NS_LOG_FUNCTION (this << ql);
  m_queueLimits = ql;
}

Ptr<QueueLimits>
NetDeviceQueue1::GetQueueLimits ()
{
  NS_LOG_FUNCTION (this);
  return m_queueLimits;
}


NS_OBJECT_ENSURE_REGISTERED (NetDeviceQueueInterface1);

TypeId NetDeviceQueueInterface1::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NetDeviceQueueInterface")
    .SetParent<Object> ()
    .SetGroupName("Network")
  ;
  return tid;
}

NetDeviceQueueInterface1::NetDeviceQueueInterface1 ()
  : m_numTxQueues (1)
{
  NS_LOG_FUNCTION (this);
}

NetDeviceQueueInterface1::~NetDeviceQueueInterface1 ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<NetDeviceQueue1>
NetDeviceQueueInterface1::GetTxQueue (uint8_t i) const
{
  NS_ASSERT (i < m_txQueuesVector.size ());
  return m_txQueuesVector[i];
}

uint8_t
NetDeviceQueueInterface1::GetNTxQueues (void) const
{
  return m_txQueuesVector.size ();
}

void
NetDeviceQueueInterface1::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_txQueuesVector.clear ();
  Object::DoDispose ();
}

void
NetDeviceQueueInterface1::SetTxQueuesN (uint8_t numTxQueues)
{
  NS_LOG_FUNCTION (this << numTxQueues);
  NS_ASSERT (numTxQueues > 0);

  NS_ABORT_MSG_IF (m_txQueuesVector.size (), "Cannot change the number of"
                   " device transmission queues once they have been created.");

  m_numTxQueues = numTxQueues;
}

void
NetDeviceQueueInterface1::CreateTxQueues (void)
{
  NS_LOG_FUNCTION (this);

  NS_ABORT_MSG_IF (m_txQueuesVector.size (), "The device transmission queues"
                   " have been already created.");

  for (uint8_t i = 0; i < m_numTxQueues; i++)
    {
      Ptr<NetDeviceQueue1> devQueue = Create<NetDeviceQueue1> ();
      m_txQueuesVector.push_back (devQueue);
    }
}

void
NetDeviceQueueInterface1::SetSelectQueueCallback (SelectQueueCallback cb)
{
  m_selectQueueCallback = cb;
}

NetDeviceQueueInterface1::SelectQueueCallback
NetDeviceQueueInterface1::GetSelectQueueCallback (void) const
{
  return m_selectQueueCallback;
}

NS_OBJECT_ENSURE_REGISTERED (NetDevice1);

TypeId NetDevice1::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NetDevice")
    .SetParent<Object> ()
    .SetGroupName("Network")
  ;
  return tid;
}

NetDevice1::~NetDevice1 ()
{
  NS_LOG_FUNCTION (this);
}

} // namespace ns3
