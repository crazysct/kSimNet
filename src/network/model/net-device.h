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
 * Modified by Emmanuelle Laprise to remove dependence on LLC headers
 */
#ifndef NET_DEVICE_H
#define NET_DEVICE_H

#include <stdint.h>
#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/ptr.h"
#include "packet.h"
#include "address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"

#include "ns3/queue-item.h" // jhlim

namespace ns3 {

class Node;
class Channel;

/**
 * \ingroup network
 * \defgroup netdevice Network Device
 */

/**
 * \ingroup netdevice
 *
 * \brief Network layer to device interface
 *
 * This interface defines the API which the IP and ARP
 * layers need to access to manage an instance of a network device 
 * layer. It currently does not support MAC-level 
 * multicast but this should not be too hard to add by adding
 * extra methods to register MAC multicast addresses to
 * filter out unwanted packets before handing them to the
 * higher layers.
 *
 * In Linux, this interface is analogous to the interface
 * just above dev_queue_xmit() (i.e., IP packet is fully
 * constructed with destination MAC address already selected).
 * 
 * If you want to write a new MAC layer, you need to subclass
 * this base class and implement your own version of the
 * pure virtual methods in this class.
 *
 * This class was designed to hide as many MAC-level details as 
 * possible from the perspective of layer 3 to allow a single layer 3
 * to work with any kind of MAC layer. Specifically, this class 
 * encapsulates the specific format of MAC addresses used by a
 * device such that the layer 3 does not need any modification
 * to handle new address formats. This means obviously that the
 * NetDevice class must know about the address format of all potential 
 * layer 3 protocols through its GetMulticast methods: the current
 * API has been optimized to make it easy to add new MAC protocols,
 * not to add new layer 3 protocols.
 *
 * Devices aiming to support flow control and dynamic queue limits must perform
 * the following operations:
 *   - in the NotifyNewAggregate method
 *     + cache the pointer to the netdevice queue interface aggregated to the
 *       device
 *     + set the select queue callback through the netdevice queue interface,
 *       if the device is multi-queue
 *   - anytime before initialization
 *     + set the number of device transmission queues (and optionally create them)
 *       through the netdevice queue interface, if the device is multi-queue
 *   - when the device queues have been created, invoke
 *     NetDeviceQueueInterface::ConnectQueueTraces, which
 *     + connects the Enqueue traced callback of the device queues to the
 *       PacketEnqueued static method of the NetDeviceQueue class
 *     + connects the Dequeue and DropAfterDequeue traced callback of the device
 *       queues to the PacketDequeued static method of the NetDeviceQueue
 *       class
 *     + connects the DropBeforeEnqueue traced callback of the device queues to
 *       the PacketDiscarded static method of the NetDeviceQueue class
 */

/* jhlim 
class QueueItem : public SimpleRefCount<QueueItem>
{
public:
  QueueItem (Ptr<Packet> p);
  virtual ~QueueItem ();

  Ptr<Packet> GetPacket (void) const;

  virtual uint32_t GetPacketSize (void) const;

  enum Uint8Values
  {
	  IP_DSFIELD
  };

  virtual bool GetUint8Value (Uint8Values field, uint8_t &value) const;

  virtual void Pritn (std::ostream &os) const;

  typedef void (* TracedCallback) (Ptr<const QueueItem> item);

private:
  QueueItem ();
  QueueItem (const QueueItem &);
  QueueItem &operator = (const QueueItem &);

  Ptr<Packet> m_packet;
};

std::ostream& operator<< (std::ostream& os, const QueueItem &item);
*/

/* jhlim 
class NetDeviceQueue : public SimpleRefCount<NetDeviceQueue>
{
public:
	NetDeviceQueue ();
	virtual ~NetDeviceQueue();

	virtual void Start (void);
	virtual void Stop (void);
	virtual void Wake (void);
	bool IsStopped (void) const;
	typedef Callback< void > WakeCallback;
	virtual void SetWakeCallback (WakeCallback cb);
	void NotifyQueueBytes (uint32_t bytes);
	void NotifyTransmittedBytes (uint32_t bytes);
	void ResetQueueLimits ();
	void SetQueueLimits (Ptr<QueueLimits> ql);
	Ptr<QueueLimits> GetQueueLimits ();

private:
	bool m_stoppedByDevice;
	bool m_stoppedByQueueLimits;
	Ptr<QueueLimits> m_queueLimits;
	WakeCallback m_wakeCallback;
};
*/

/* jhlim 
class NetDeviceQueueInterface : public Object
{
public:
	static TypeId GetTypeId (void);
	NetDeviceQueueInterface ();
	virtual ~NetDeviceQueueInterface ();
	Ptr<NetDeviceQueue> GetTxQueue (uint8_t i) const;
	uint8_t GetNTxQueues (void) const;
	void SetTxQueuesN (uint8_t numTxQueues);
	void CreateTxQueues (void);
	typedef Callback< uint8_t, Ptr<QueueItem> > SelectQueueCallback;
	void SetSelectQueueCallback (SelectQueueCallback cb);
	SelectQueueCallback GetSelectQueueCallback (void) const;
protected:
	virtual void DoDispose (void);
 
private:
   std::vector< Ptr<NetDeviceQueue> > m_txQueuesVector;
   SelectQueueCallback m_selectQueueCallback;
   uint8_t m_numTxQueues;
};
*/

class NetDevice : public Object
{
public:
  static TypeId GetTypeId (void);
  virtual ~NetDevice();

  virtual void SetIfIndex (const uint32_t index) = 0;
  virtual uint32_t GetIfIndex (void) const = 0;
  virtual Ptr<Channel> GetChannel (void) const = 0;
  virtual void SetAddress (Address address) = 0;
  virtual Address GetAddress (void) const = 0;
  virtual bool SetMtu (const uint16_t mtu) = 0;
  virtual uint16_t GetMtu (void) const = 0;
  virtual bool IsLinkUp (void) const = 0;
  typedef void (* LinkChangeTracedCallback) (void);
  virtual void AddLinkChangeCallback (Callback<void> callback) = 0;
  virtual bool IsBroadcast (void) const = 0;
  virtual Address GetBroadcast (void) const = 0;
  virtual bool IsMulticast (void) const = 0;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const = 0;
  virtual Address GetMulticast (Ipv6Address addr) const = 0;
  virtual bool IsBridge (void) const = 0;
  virtual bool IsPointToPoint (void) const = 0;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) = 0;
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber) = 0;
  virtual Ptr<Node> GetNode (void) const = 0;
  virtual void SetNode (Ptr<Node> node) = 0;
  virtual bool NeedsArp (void) const = 0;
  enum PacketType
  {
    PACKET_HOST = 1,   
    NS3_PACKET_HOST = PACKET_HOST,
    PACKET_BROADCAST,   
    NS3_PACKET_BROADCAST = PACKET_BROADCAST,
    PACKET_MULTICAST,   
    NS3_PACKET_MULTICAST = PACKET_MULTICAST,
    PACKET_OTHERHOST,   
    NS3_PACKET_OTHERHOST = PACKET_OTHERHOST,
  };

  typedef Callback< bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t, const Address & > ReceiveCallback;
  virtual void SetReceiveCallback (ReceiveCallback cb) = 0;
  typedef Callback< bool, Ptr<NetDevice>, Ptr<const Packet>, uint16_t,
                    const Address &, const Address &, enum PacketType > PromiscReceiveCallback;

  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb) = 0;
  virtual bool SupportsSendFrom (void) const = 0;

  bool isEnbTypeForDc; //sjkang

};

} // namespace ns3

#endif /* NET_DEVICE_H */
