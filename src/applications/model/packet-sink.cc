/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 * Author:  Tom Henderson (tomhend@u.washington.edu)
 */
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "packet-sink.h"
#include "ns3/uinteger.h"
#include "packet-loss-counter.h"
#include <ns3/tag.h>
#include "seq-ts-header.h"
#include "ns3/MyAppTag.h"
#include "ns3/boolean.h"
namespace ns3 {
/*class MyAppTag : public Tag
{
public:
  MyAppTag ()
  {
  }

  MyAppTag (Time sendTs) : m_sendTs (sendTs)
  {
  }

  static TypeId GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::MyAppTag_e")
      .SetParent<Tag> ()
      .AddConstructor<MyAppTag> ();
    return tid;
  }

  virtual TypeId  GetInstanceTypeId (void) const
  {
    return GetTypeId ();
  }

  virtual void  Serialize (TagBuffer i) const
  {
    i.WriteU64 (m_sendTs.GetNanoSeconds());
  }

  virtual void  Deserialize (TagBuffer i)
  {
    m_sendTs = NanoSeconds (i.ReadU64 ());
  }

  virtual uint32_t  GetSerializedSize () const
  {
    return sizeof (m_sendTs);
  }

  virtual void Print (std::ostream &os) const
  {
    std::cout << m_sendTs;
  }

  Time m_sendTs;
};
*/

NS_LOG_COMPONENT_DEFINE ("PacketSink");

NS_OBJECT_ENSURE_REGISTERED (PacketSink);

TypeId 
PacketSink::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PacketSink")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<PacketSink> ()
    .AddAttribute ("Local",
                   "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&PacketSink::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol",
                   "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&PacketSink::m_tid),
                   MakeTypeIdChecker ())
	.AddAttribute ("PacketWindowSize",
				   "The size of the window used to compute the packet loss. This value should be a multiple of 8.",
				    UintegerValue (32),
				   MakeUintegerAccessor (&PacketSink::GetPacketWindowSize,
				                          &PacketSink::SetPacketWindowSize),
				    MakeUintegerChecker<uint16_t> (8,256))
	.AddAttribute("MessgeTransmission", "The case of sending periodic message from server", BooleanValue(false),
			MakeBooleanAccessor(&PacketSink::messageTransmission),  MakeBooleanChecker ())

	.AddAttribute("PacketNumber", "number of packets transmitted from server periodically in case of sending messages",
			UintegerValue(500), MakeUintegerAccessor(&PacketSink::numberPackets),
			MakeUintegerChecker<uint32_t>())

    .AddTraceSource ("Rx",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&PacketSink::m_rxTrace),
                     "ns3::Packet::AddressTracedCallback")
	.AddTraceSource("Loss", "Udp packet loss rate", MakeTraceSourceAccessor(&PacketSink::m_loss)
					     		, "ns3::Packet::UdpLossRate")
	.AddTraceSource("LinkDelay", "this is link dealy between apps", MakeTraceSourceAccessor(&PacketSink::m_delay)
			,"ns3::Packet::LinkDelay");
  ;
  return tid;
}

PacketSink::PacketSink ()
:m_lossCounter(0)
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_totalRx = 0;
  m_received =0;
  m_receivedSN =0;
  success = false;
  numberPackets = 0;
}

PacketSink::~PacketSink()
{
  NS_LOG_FUNCTION (this);
}

uint64_t PacketSink::GetTotalRx () const
{
  NS_LOG_FUNCTION (this);
  return m_totalRx;
}

Ptr<Socket>
PacketSink::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}
bool
PacketSink::GetSuccess(){
	return success;
}
std::list<Ptr<Socket> >
PacketSink::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}

void PacketSink::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socketList.clear ();

  // chain up
  Application::DoDispose ();
}
uint32_t
PacketSink::GetLost() const{
	NS_LOG_FUNCTION (this);
	 return m_lossCounter.GetLost ();
}
uint64_t PacketSink::GetReceived() const{
	NS_LOG_FUNCTION (this);
	  return m_received;
}
void PacketSink::SetReceived(uint32_t received){
	m_received = received;
}
uint32_t PacketSink::GetReceivdSN() const{
	return m_receivedSN;
}
uint16_t PacketSink::GetPacketWindowSize()const{
	 NS_LOG_FUNCTION (this);
	  return m_lossCounter.GetBitMapSize ();
}
void PacketSink::SetPacketWindowSize(uint16_t size){
	 NS_LOG_FUNCTION (this);
	 m_lossCounter.SetBitMapSize(size);
}

// Application Methods
void PacketSink::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->Bind (m_local);
      m_socket->Listen ();
      m_socket->ShutdownSend ();
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&PacketSink::HandleRead, this));
  m_socket->SetAcceptCallback (
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&PacketSink::HandleAccept, this));
  m_socket->SetCloseCallbacks (
    MakeCallback (&PacketSink::HandlePeerClose, this),
    MakeCallback (&PacketSink::HandlePeerError, this));
}

void PacketSink::StopApplication ()     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  while(!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
  if (m_socket) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void PacketSink::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {

	  MyAppTag tag;
//	  uint32_t delay;
	  double delay;

	//  	packet->FindFirstMatchingByteTag(tag);
	 // std::cout << tag.GetTime().GetSeconds() << std::endl;


	   	    	     // std::cout<<tag.m_sendTs<<std::endl;
	  SeqTsHeader seqTs;
	  packet->RemoveHeader (seqTs);
//std::cout << seqTs.GetSeq() <<std::endl;
	   uint32_t currentSequenceNumber = seqTs.GetSeq ();
      if (packet->GetSize () == 0)
        { //EOF
          break;
        }

    	      //std::cout << delay;

      m_totalRx += packet->GetSize ();
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("*UE, " << Simulator::Now ().GetSeconds () << "s "// woody, logging
                       <<  packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4 ()
                       << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
          packet->PeekPacketTag(tag);

          if ( packet->FindFirstMatchingByteTag(tag)){
        	  	  	  	  	  packet->RemoveAllByteTags();
     	   	    	       delay = Simulator::Now().GetSeconds()-tag.GetTime().GetSeconds();
     	   	    	       packet->RemoveAllPacketTags();
     	    	    	  }

/*          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4 ()
                       << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");*/
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_LOGIC ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << Inet6SocketAddress::ConvertFrom(from).GetIpv6 ()
                       << " port " << Inet6SocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
      m_lossCounter.NotifyReceived(currentSequenceNumber);
      m_received++;
      m_rxTrace (packet, from);
      m_loss(seqTs.GetSeq(),GetLost());
      m_receivedSN = seqTs.GetSeq(); //sjkang
       m_delay( delay);

       if (messageTransmission){ //sjkang
    	   SN.push_back(m_receivedSN);
       if (SN.size()==numberPackets){
    	  success = true;
    	  ArrivalTime = Simulator::Now();
    	  SN.clear();
       	   }
       else
    	   success = false;
       }
    }
}

Time
PacketSink::GetArrivalTime(){
	return ArrivalTime;
}
void
PacketSink::SetClearSnBuffer(){
	SN.clear();
}
void PacketSink::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
void PacketSink::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 

void PacketSink::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&PacketSink::HandleRead, this));
  m_socketList.push_back (s);
}

} // Namespace ns3
