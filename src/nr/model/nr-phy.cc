/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab
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
 *         Marco Miozzo <mmiozzo@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#include <ns3/waveform-generator.h>
#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <cmath>
#include <ns3/simulator.h>
#include "ns3/spectrum-error-model.h"
#include "nr-phy.h"
#include "nr-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrPhy");

NS_OBJECT_ENSURE_REGISTERED (NrPhy);


NrPhy::NrPhy ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("This constructor should not be called");
}

NrPhy::NrPhy (Ptr<NrSpectrumPhy> dlPhy, Ptr<NrSpectrumPhy> ulPhy)
  : m_downlinkSpectrumPhy (dlPhy),
    m_uplinkSpectrumPhy (ulPhy),
    m_tti (0.001),
    m_ulBandwidth (0),
    m_dlBandwidth (0),
    m_rbgSize (0),
    m_macChTtiDelay (0),
    m_cellId (0)
{
  NS_LOG_FUNCTION (this);
}


TypeId
NrPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrPhy")
    .SetParent<Object> ()
    .SetGroupName("Nr")
  ;
  return tid;
}


NrPhy::~NrPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
NrPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_packetBurstQueue.clear ();
  m_controlMessagesQueue.clear ();
  m_downlinkSpectrumPhy->Dispose ();
  m_downlinkSpectrumPhy = 0;
  m_uplinkSpectrumPhy->Dispose ();
  m_uplinkSpectrumPhy = 0;
  m_netDevice = 0;
  Object::DoDispose ();
}

void
NrPhy::SetDevice (Ptr<NetDevice> d)
{
  NS_LOG_FUNCTION (this << d);
  m_netDevice = d;
}


Ptr<NetDevice>
NrPhy::GetDevice () const
{
  NS_LOG_FUNCTION (this);
  return m_netDevice;
}

Ptr<NrSpectrumPhy> 
NrPhy::GetDownlinkSpectrumPhy ()
{
  return m_downlinkSpectrumPhy;
}

Ptr<NrSpectrumPhy> 
NrPhy::GetUplinkSpectrumPhy ()
{
  return m_uplinkSpectrumPhy;
}


void
NrPhy::SetDownlinkChannel (Ptr<SpectrumChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  m_downlinkSpectrumPhy->SetChannel (c);
}

void
NrPhy::SetUplinkChannel (Ptr<SpectrumChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  m_uplinkSpectrumPhy->SetChannel (c);
}

void
NrPhy::SetTti (double tti)
{
  NS_LOG_FUNCTION (this << tti);
  m_tti = tti;
}


double
NrPhy::GetTti (void) const
{
  NS_LOG_FUNCTION (this << m_tti);
  return m_tti;
}


uint16_t
NrPhy::GetSrsPeriodicity (uint16_t srcCi) const
{
  // from 3GPP TS 36.213 table 8.2-1 UE Specific SRS Periodicity
  uint16_t SrsPeriodicity[9] = {0, 2, 5, 10, 20, 40, 80, 160, 320};
  uint16_t SrsCiLow[9] = {0, 0, 2, 7, 17, 37, 77, 157, 317};
  uint16_t SrsCiHigh[9] = {0, 1, 6, 16, 36, 76, 156, 316, 636};
  uint8_t i;
  for (i = 8; i > 0; i --)
    {
      if ((srcCi>=SrsCiLow[i])&&(srcCi<=SrsCiHigh[i]))
        {
          break;
        }
    }
  return SrsPeriodicity[i];
}

uint16_t
NrPhy::GetSrsSubframeOffset (uint16_t srcCi) const
{
  // from 3GPP TS 36.213 table 8.2-1 UE Specific SRS Periodicity
  uint16_t SrsSubframeOffset[9] = {0, 0, 2, 7, 17, 37, 77, 157, 317};
  uint16_t SrsCiLow[9] = {0, 0, 2, 7, 17, 37, 77, 157, 317};
  uint16_t SrsCiHigh[9] = {0, 1, 6, 16, 36, 76, 156, 316, 636};
  uint8_t i;
  for (i = 8; i > 0; i --)
    {
      if ((srcCi>=SrsCiLow[i])&&(srcCi<=SrsCiHigh[i]))
        {
          break;
        }
    }
  return (srcCi - SrsSubframeOffset[i]);
}

uint8_t
NrPhy::GetRbgSize (void) const
{
  return m_rbgSize;
}

void
NrPhy::SetMacPdu (Ptr<Packet> p)
{
  m_packetBurstQueue.at (m_packetBurstQueue.size () - 1)->AddPacket (p);
}

Ptr<PacketBurst>
NrPhy::GetPacketBurst (void)
{
  if (m_packetBurstQueue.at (0)->GetSize () > 0)
    {
      Ptr<PacketBurst> ret = m_packetBurstQueue.at (0)->Copy ();
      m_packetBurstQueue.erase (m_packetBurstQueue.begin ());
      m_packetBurstQueue.push_back (CreateObject <PacketBurst> ());
      return (ret);
    }
  else
    {
      m_packetBurstQueue.erase (m_packetBurstQueue.begin ());
      m_packetBurstQueue.push_back (CreateObject <PacketBurst> ());
      return (0);
    }
}


void
NrPhy::SetControlMessages (Ptr<NrControlMessage> m)
{
  // In uplink the queue of control messages and packet are of different sizes
  // for avoiding TTI cancellation due to synchronization of subframe triggers
  m_controlMessagesQueue.at (m_controlMessagesQueue.size () - 1).push_back (m);
}

std::list<Ptr<NrControlMessage> >
NrPhy::GetControlMessages (void)
{
  NS_LOG_FUNCTION (this);
  if (m_controlMessagesQueue.at (0).size () > 0)
    {
      std::list<Ptr<NrControlMessage> > ret = m_controlMessagesQueue.at (0);
      m_controlMessagesQueue.erase (m_controlMessagesQueue.begin ());
      std::list<Ptr<NrControlMessage> > newlist;
      m_controlMessagesQueue.push_back (newlist);
      return (ret);
    }
  else
    {
      m_controlMessagesQueue.erase (m_controlMessagesQueue.begin ());
      std::list<Ptr<NrControlMessage> > newlist;
      m_controlMessagesQueue.push_back (newlist);
      std::list<Ptr<NrControlMessage> > emptylist;
      return (emptylist);
    }
}


void
NrPhy::DoSetCellId (uint16_t cellId)
{
  m_cellId = cellId;
  m_downlinkSpectrumPhy->SetCellId (cellId);
  m_uplinkSpectrumPhy->SetCellId (cellId);
}


} // namespace ns3
