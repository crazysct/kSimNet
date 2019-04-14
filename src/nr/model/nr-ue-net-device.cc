/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 *         Marco Miozzo <mmiozzo@cttc.es>
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
#include "ns3/nr-enb-net-device.h"
#include "nr-ue-net-device.h"
#include "nr-ue-mac.h"
#include "nr-ue-rrc.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4.h"
#include "nr-amc.h"
#include "nr-ue-phy.h"
#include "ngc-ue-nas.h"
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/log.h>
#include "ngc-tft.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrUeNetDevice");

NS_OBJECT_ENSURE_REGISTERED ( NrUeNetDevice);


TypeId NrUeNetDevice::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::NrUeNetDevice")
    .SetParent<NrNetDevice> ()
    .AddConstructor<NrUeNetDevice> ()
    .AddAttribute ("NgcUeNas",
                   "The NAS associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&NrUeNetDevice::m_nas),
                   MakePointerChecker <NgcUeNas> ())
    .AddAttribute ("NrUeRrc",
                   "The RRC associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&NrUeNetDevice::m_rrc),
                   MakePointerChecker <NrUeRrc> ())
    .AddAttribute ("NrUeMac",
                   "The MAC associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&NrUeNetDevice::m_mac),
                   MakePointerChecker <NrUeMac> ())
    .AddAttribute ("NrUePhy",
                   "The PHY associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&NrUeNetDevice::m_phy),
                   MakePointerChecker <NrUePhy> ())
    .AddAttribute ("Imsi",
                   "International Mobile Subscriber Identity assigned to this UE",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrUeNetDevice::m_imsi),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("DlEarfcn",
                   "Downlink E-UTRA Absolute Radio Frequency Channel Number (EARFCN) "
                   "as per 3GPP 36.101 Section 5.7.3. ",
                   UintegerValue (100),
                   MakeUintegerAccessor (&NrUeNetDevice::SetDlEarfcn,
                                         &NrUeNetDevice::GetDlEarfcn),
                   MakeUintegerChecker<uint16_t> (0, 6149))
    .AddAttribute ("CsgId",
                   "The Closed Subscriber Group (CSG) identity that this UE is associated with, "
                   "i.e., giving the UE access to cells which belong to this particular CSG. "
                   "This restriction only applies to initial cell selection and NGC-enabled simulation. "
                   "This does not revoke the UE's access to non-CSG cells. ",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrUeNetDevice::SetCsgId,
                                         &NrUeNetDevice::GetCsgId),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}


NrUeNetDevice::NrUeNetDevice (void)
  : m_isConstructed (false)
{
  NS_LOG_FUNCTION (this);
}

NrUeNetDevice::~NrUeNetDevice (void)
{
  NS_LOG_FUNCTION (this);
}

void
NrUeNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_targetEnb = 0;
  m_mac->Dispose ();
  m_mac = 0;
  m_rrc->Dispose ();
  m_rrc = 0;
  m_phy->Dispose ();
  m_phy = 0;
  m_nas->Dispose ();
  m_nas = 0;
  NrNetDevice::DoDispose ();
}

void
NrUeNetDevice::UpdateConfig (void)
{
  NS_LOG_FUNCTION (this);

  if (m_isConstructed)
    {
      NS_LOG_LOGIC (this << " Updating configuration: IMSI " << m_imsi
                         << " CSG ID " << m_csgId);
      m_nas->SetImsi (m_imsi);
      m_rrc->SetImsi (m_imsi);
      m_nas->SetCsgId (m_csgId); // this also handles propagation to RRC
    }
  else
    {
      /*
       * NAS and RRC instances are not be ready yet, so do nothing now and
       * expect ``DoInitialize`` to re-invoke this function.
       */
    }
}



Ptr<NrUeMac>
NrUeNetDevice::GetMac (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}


Ptr<NrUeRrc>
NrUeNetDevice::GetRrc (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rrc;
}


Ptr<NrUePhy>
NrUeNetDevice::GetPhy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

Ptr<NgcUeNas>
NrUeNetDevice::GetNas (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nas;
}

uint64_t
NrUeNetDevice::GetImsi () const
{
  NS_LOG_FUNCTION (this);
  return m_imsi;
}

uint16_t
NrUeNetDevice::GetDlEarfcn () const
{
  NS_LOG_FUNCTION (this);
  return m_dlEarfcn;
}

void
NrUeNetDevice::SetDlEarfcn (uint16_t earfcn)
{
  NS_LOG_FUNCTION (this << earfcn);
  m_dlEarfcn = earfcn;
}

uint32_t
NrUeNetDevice::GetCsgId () const
{
  NS_LOG_FUNCTION (this);
  return m_csgId;
}

void
NrUeNetDevice::SetCsgId (uint32_t csgId)
{
  NS_LOG_FUNCTION (this << csgId);
  m_csgId = csgId;
  UpdateConfig (); // propagate the change down to NAS and RRC
}

void
NrUeNetDevice::SetTargetEnb (Ptr<NrEnbNetDevice> enb)
{
  NS_LOG_FUNCTION (this << enb);
  m_targetEnb = enb;
}


Ptr<NrEnbNetDevice>
NrUeNetDevice::GetTargetEnb (void)
{
  NS_LOG_FUNCTION (this);
  return m_targetEnb;
}

void 
NrUeNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
  UpdateConfig ();
  m_phy->Initialize ();
  m_mac->Initialize ();
  m_rrc->Initialize ();
}

bool
NrUeNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << dest << protocolNumber);
  if (protocolNumber != Ipv4L3Protocol::PROT_NUMBER)
    {
      NS_LOG_INFO("unsupported protocol " << protocolNumber << ", only IPv4 is supported");
      return true;
    }  
  return m_nas->Send (packet);
}


} // namespace ns3
