
//de:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "original_queue.h"
#include "ns3/net-device.h" //jhlim

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Queue1");

NS_OBJECT_ENSURE_REGISTERED (Queue1);

TypeId
Queue1::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Queue1")
    .SetParent<Object> ()
    .SetGroupName ("Network")
    .AddAttribute ("Mode",
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (QUEUE_MODE_PACKETS),
                   MakeEnumAccessor (&Queue1::SetMode,
                                     &Queue1::GetMode),
                   MakeEnumChecker (QUEUE_MODE_BYTES, "QUEUE_MODE_BYTES",
                                    QUEUE_MODE_PACKETS, "QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets",
                   "The maximum number of packets accepted by this queue.",
                   UintegerValue (100),
                   MakeUintegerAccessor (&Queue1::SetMaxPackets,
                                         &Queue1::GetMaxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes",
                   "The maximum number of bytes accepted by this queue.",
                   UintegerValue (100 * 65535),
                   MakeUintegerAccessor (&Queue1::SetMaxBytes,
                                         &Queue1::GetMaxBytes),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Enqueue", "Enqueue a packet in the queue.",
                     MakeTraceSourceAccessor (&Queue1::m_traceEnqueue),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Dequeue", "Dequeue a packet from the queue.",
                     MakeTraceSourceAccessor (&Queue1::m_traceDequeue),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("Drop", "Drop a packet (for whatever reason).",
                     MakeTraceSourceAccessor (&Queue1::m_traceDrop),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PacketsInQueue",
                     "Number of packets currently stored in the queue",
                     MakeTraceSourceAccessor (&Queue1::m_nPackets),
                     "ns3::TracedValueCallback::Uint32")
    .AddTraceSource ("BytesInQueue",
                     "Number of bytes currently stored in the queue",
                     MakeTraceSourceAccessor (&Queue1::m_nBytes),
                     "ns3::TracedValueCallback::Uint32")
  ;
  return tid;
}

Queue1::Queue1() :
  m_nBytes (0),
  m_nTotalReceivedBytes (0),
  m_nPackets (0),
  m_nTotalReceivedPackets (0),
  m_nTotalDroppedBytes (0),
  m_nTotalDroppedPackets (0),
  m_mode (QUEUE_MODE_PACKETS)
{
  NS_LOG_FUNCTION (this);
}

Queue1::~Queue1()
{
  NS_LOG_FUNCTION (this);
}


bool
Queue1::Enqueue (Ptr<QueueItem> item)
{
  NS_LOG_UNCOND(this << item);

  if (m_mode == QUEUE_MODE_PACKETS && (m_nPackets.Get () >= m_maxPackets))
    {
      NS_LOG_LOGIC ("Queue full (at max packets) -- dropping pkt");
      Drop (item);
      return false;
    }

  if (m_mode == QUEUE_MODE_BYTES && (m_nBytes.Get () + item->GetSize () > m_maxBytes))
    {
      NS_LOG_LOGIC ("Queue full (packet would exceed max bytes) -- dropping pkt");
      Drop (item);
      return false;
    }

  //
  // If DoEnqueue fails, Queue::Drop is called by the subclass
  //
  bool retval = DoEnqueue (item);
  if (retval)
    {
      NS_LOG_LOGIC ("m_traceEnqueue (p)");
      m_traceEnqueue (item->GetPacket ());

      uint32_t size = item->GetSize ();
      m_nBytes += size;
      m_nTotalReceivedBytes += size;

      m_nPackets++;
      m_nTotalReceivedPackets++;
    }
  return retval;
}

Ptr<QueueItem>
Queue1::Dequeue (void)
{
  NS_LOG_FUNCTION (this);

  if (m_nPackets.Get () == 0)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<QueueItem> item = DoDequeue ();

  if (item != 0)
    {
      NS_ASSERT (m_nBytes.Get () >= item->GetSize ());
      NS_ASSERT (m_nPackets.Get () > 0);

      m_nBytes -= item->GetSize ();
      m_nPackets--;

      NS_LOG_LOGIC ("m_traceDequeue (packet)");
      m_traceDequeue (item->GetPacket ());
    }
  return item;
}

Ptr<QueueItem>
Queue1::Remove (void)
{
  NS_LOG_FUNCTION (this);

  if (m_nPackets.Get () == 0)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  Ptr<QueueItem> item = DoRemove ();

  if (item != 0)
    {
      NS_ASSERT (m_nBytes.Get () >= item->GetSize ());
      NS_ASSERT (m_nPackets.Get () > 0);

      m_nBytes -= item->GetSize ();
      m_nPackets--;

      Drop (item);
    }
  return item;
}

void
Queue1::DequeueAll (void)
{
  NS_LOG_FUNCTION (this);
  while (!IsEmpty ())
    {
      Dequeue ();
    }
}

Ptr<const QueueItem>
Queue1::Peek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_nPackets.Get () == 0)
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  return DoPeek ();
}


uint32_t
Queue1::GetNPackets (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << m_nPackets);
  return m_nPackets;
}

uint32_t
Queue1::GetNBytes (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (" returns " << m_nBytes);
  return m_nBytes;
}

bool
Queue1::IsEmpty (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << (m_nPackets.Get () == 0));
  return m_nPackets.Get () == 0;
}

uint32_t
Queue1::GetTotalReceivedBytes (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << m_nTotalReceivedBytes);
  return m_nTotalReceivedBytes;
}

uint32_t
Queue1::GetTotalReceivedPackets (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << m_nTotalReceivedPackets);
  return m_nTotalReceivedPackets;
}

uint32_t
Queue1:: GetTotalDroppedBytes (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << m_nTotalDroppedBytes);
  return m_nTotalDroppedBytes;
}

uint32_t
Queue1::GetTotalDroppedPackets (void) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("returns " << m_nTotalDroppedPackets);
  return m_nTotalDroppedPackets;
}

void
Queue1::ResetStatistics (void)
{
  NS_LOG_FUNCTION (this);
  m_nTotalReceivedBytes = 0;
  m_nTotalReceivedPackets = 0;
  m_nTotalDroppedBytes = 0;
  m_nTotalDroppedPackets = 0;
}

void
Queue1::SetMode (Queue1::QueueMode mode)
{
  NS_LOG_FUNCTION (this << mode);

  if (mode == QUEUE_MODE_BYTES && m_mode == QUEUE_MODE_PACKETS)
    {
      NS_ABORT_MSG_IF (m_nPackets.Get () != 0,
                       "Cannot change queue mode in a queue with packets.");
    }
  else if (mode == QUEUE_MODE_PACKETS && m_mode == QUEUE_MODE_BYTES)
    {
      NS_ABORT_MSG_IF (m_nBytes.Get () != 0,
                       "Cannot change queue mode in a queue with packets.");
    }

  m_mode = mode;
}

Queue1::QueueMode
Queue1::GetMode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mode;
}

void
Queue1::SetMaxPackets (uint32_t maxPackets)
{
  NS_LOG_FUNCTION (this << maxPackets);

  if (m_mode == QUEUE_MODE_PACKETS)
    {
      NS_ABORT_MSG_IF (maxPackets < m_nPackets.Get (),
                       "The new queue size cannot be less than the number of currently stored packets.");
    }

  m_maxPackets = maxPackets;
}

uint32_t
Queue1::GetMaxPackets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_maxPackets;
}

void
Queue1::SetMaxBytes (uint32_t maxBytes)
{
  NS_LOG_FUNCTION (this << maxBytes);

  if (m_mode == QUEUE_MODE_BYTES)
    {
      NS_ABORT_MSG_IF (maxBytes < m_nBytes.Get (),
                       "The new queue size cannot be less than the amount of bytes of currently stored packets.");
    }

  m_maxBytes = maxBytes;
}

uint32_t
Queue1::GetMaxBytes (void) const
{
  NS_LOG_FUNCTION (this);
  return m_maxBytes;
}

void
Queue1::SetDropCallback (DropCallback cb)
{
  m_dropCallback = cb;
}

void
Queue1::NotifyDrop (Ptr<QueueItem> item)
{
  NS_LOG_FUNCTION (this << item);

  if (!m_dropCallback.IsNull ())
    {
      m_dropCallback (item);
    }
}

void
Queue1::Drop (Ptr<QueueItem> item)
{
  NS_LOG_FUNCTION (this << item);

  m_nTotalDroppedPackets++;
  m_nTotalDroppedBytes += item->GetSize ();

  NS_LOG_LOGIC ("m_traceDrop (p)");
  m_traceDrop (item->GetPacket ());
  NotifyDrop (item);
}

} // namespace ns3
