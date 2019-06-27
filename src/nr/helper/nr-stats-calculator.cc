/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jnin@cttc.es>
 */

#include "nr-stats-calculator.h"

#include <ns3/log.h>
#include <ns3/config.h>
#include <ns3/nr-enb-rrc.h>
#include <ns3/nr-ue-rrc.h>
#include <ns3/nr-enb-net-device.h>
#include <ns3/nr-ue-net-device.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED (NrStatsCalculator);

NrStatsCalculator::NrStatsCalculator ()
  : m_dlOutputFilename (""),
    m_ulOutputFilename ("")
{
  // Nothing to do here

}

NrStatsCalculator::~NrStatsCalculator ()
{
  // Nothing to do here
}


TypeId
NrStatsCalculator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrStatsCalculator")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddConstructor<NrStatsCalculator> ()
  ;
  return tid;
}


void
NrStatsCalculator::SetUlOutputFilename (std::string outputFilename)
{
  m_ulOutputFilename = outputFilename;
}

std::string
NrStatsCalculator::GetUlOutputFilename (void)
{
  return m_ulOutputFilename;
}

void
NrStatsCalculator::SetDlOutputFilename (std::string outputFilename)
{
  m_dlOutputFilename = outputFilename;
}

std::string
NrStatsCalculator::GetDlOutputFilename (void)
{
  return m_dlOutputFilename;
}


bool
NrStatsCalculator::ExistsImsiPath (std::string path)
{
  if (m_pathImsiMap.find (path) == m_pathImsiMap.end () )
    {
      return false;
    }
  else
    {
      return true;
    }
}

void
NrStatsCalculator::SetImsiPath (std::string path, uint64_t imsi)
{
  NS_LOG_FUNCTION (this << path << imsi);
  m_pathImsiMap[path] = imsi;
}

uint64_t
NrStatsCalculator::GetImsiPath (std::string path)
{
  return m_pathImsiMap.find (path)->second;
}

bool
NrStatsCalculator::ExistsCellIdPath (std::string path)
{
  if (m_pathCellIdMap.find (path) == m_pathCellIdMap.end () )
    {
      return false;
    }
  else
    {
      return true;
    }
}

void
NrStatsCalculator::SetCellIdPath (std::string path, uint16_t cellId)
{
  NS_LOG_FUNCTION (this << path << cellId);
  m_pathCellIdMap[path] = cellId;
}

uint16_t
NrStatsCalculator::GetCellIdPath (std::string path)
{
  return m_pathCellIdMap.find (path)->second;
}


uint64_t
NrStatsCalculator::FindImsiFromEnbRlcPath (std::string path)
{
  NS_LOG_FUNCTION (path);
  // Sample path input:
  // /NodeList/#NodeId/DeviceList/#DeviceId/NrEnbRrc/UeMap/#C-RNTI/DataRadioBearerMap/#LCID/NrRlc/RxPDU

  // We retrieve the UeManager associated to the C-RNTI and perform the IMSI lookup
  std::string ueMapPath = path.substr (0, path.find ("/DataRadioBearerMap"));
  Config::MatchContainer match = Config::LookupMatches (ueMapPath);

  if (match.GetN () != 0)
    {
      Ptr<Object> ueInfo = match.Get (0);
      NS_LOG_LOGIC ("FindImsiFromEnbRlcPath: " << path << ", " << ueInfo->GetObject<UeManager> ()->GetImsi ());
      return ueInfo->GetObject<UeManager> ()->GetImsi ();
    }
  else
    {
      NS_FATAL_ERROR ("Lookup " << ueMapPath << " got no matches");
    }
}

uint64_t
NrStatsCalculator::FindImsiFromUePhy (std::string path)
{
  NS_LOG_FUNCTION (path);
  // Sample path input:
  // /NodeList/#NodeId/DeviceList/#DeviceId/NrUePhy

  // We retrieve the UeInfo associated to the C-RNTI and perform the IMSI lookup
  std::string ueRlcPath = path.substr (0, path.find ("/NrUePhy"));
  ueRlcPath += "/NrUeRrc";
  Config::MatchContainer match = Config::LookupMatches (ueRlcPath);

  if (match.GetN () != 0)
    {
      Ptr<Object> ueRrc = match.Get (0);
      return ueRrc->GetObject<NrUeRrc> ()->GetImsi ();
    }
  else
    {
      NS_FATAL_ERROR ("Lookup " << ueRlcPath << " got no matches");
    }
  return 0;
}


uint64_t
NrStatsCalculator::FindImsiFromNrNetDevice (std::string path)
{
  NS_LOG_FUNCTION (path);
  // Sample path input:
  // /NodeList/#NodeId/DeviceList/#DeviceId/

  // We retrieve the Imsi associated to the NrUeNetDevice
  Config::MatchContainer match = Config::LookupMatches (path);

  if (match.GetN () != 0)
    {
      Ptr<Object> ueNetDevice = match.Get (0);
      NS_LOG_LOGIC ("FindImsiFromNrNetDevice: " << path << ", " << ueNetDevice->GetObject<NrUeNetDevice> ()->GetImsi ());
      return ueNetDevice->GetObject<NrUeNetDevice> ()->GetImsi ();
    }
  else
    {
      NS_FATAL_ERROR ("Lookup " << path << " got no matches");
    }
}

uint16_t
NrStatsCalculator::FindCellIdFromEnbRlcPath (std::string path)
{
  NS_LOG_FUNCTION (path);
  // Sample path input:
  // /NodeList/#NodeId/DeviceList/#DeviceId/NrEnbRrc/UeMap/#C-RNTI/DataRadioBearerMap/#LCID/NrRlc/RxPDU

  // We retrieve the CellId associated to the Enb
  std::string enbNetDevicePath = path.substr (0, path.find ("/NrEnbRrc"));
  Config::MatchContainer match = Config::LookupMatches (enbNetDevicePath);
  if (match.GetN () != 0)
    {
      Ptr<Object> enbNetDevice = match.Get (0);
      NS_LOG_LOGIC ("FindCellIdFromEnbRlcPath: " << path << ", " << enbNetDevice->GetObject<NrEnbNetDevice> ()->GetCellId ());
      return enbNetDevice->GetObject<NrEnbNetDevice> ()->GetCellId ();
    }
  else
    {
      NS_FATAL_ERROR ("Lookup " << enbNetDevicePath << " got no matches");
    }
}

uint64_t
NrStatsCalculator::FindImsiFromEnbMac (std::string path, uint16_t rnti)
{
  NS_LOG_FUNCTION (path << rnti);

  // /NodeList/#NodeId/DeviceList/#DeviceId/NrEnbMac/DlScheduling
  std::ostringstream oss;
  std::string p = path.substr (0, path.find ("/NrEnbMac"));
  oss << rnti;
  p += "/NrEnbRrc/UeMap/" + oss.str ();
  uint64_t imsi = FindImsiFromEnbRlcPath (p);
  NS_LOG_LOGIC ("FindImsiFromEnbMac: " << path << ", " << rnti << ", " << imsi);
  return imsi;
}

uint16_t
NrStatsCalculator::FindCellIdFromEnbMac (std::string path, uint16_t rnti)
{
  NS_LOG_FUNCTION (path << rnti);
  // /NodeList/#NodeId/DeviceList/#DeviceId/NrEnbMac/DlScheduling
  std::ostringstream oss;
  std::string p = path.substr (0, path.find ("/NrEnbMac"));
  oss << rnti;
  p += "/NrEnbRrc/UeMap/" + oss.str ();
  uint16_t cellId = FindCellIdFromEnbRlcPath (p);
  NS_LOG_LOGIC ("FindCellIdFromEnbMac: " << path << ", "<< rnti << ", " << cellId);
  return cellId;
}


uint64_t
NrStatsCalculator::FindImsiForEnb (std::string path, uint16_t rnti)
{
  NS_LOG_FUNCTION (path << rnti);
  uint64_t imsi = 0;
  if (path.find ("/DlPhyTransmission"))
    {
      // /NodeList/0/DeviceList/0/NrEnbPhy/DlPhyTransmission/NrEnbRrc/UeMap/1
      std::ostringstream oss;
      std::string p = path.substr (0, path.find ("/NrEnbPhy"));
      oss << rnti;
      p += "/NrEnbRrc/UeMap/" + oss.str ();
      imsi = FindImsiFromEnbRlcPath (p);
      NS_LOG_LOGIC ("FindImsiForEnb[Tx]: " << path << ", " << rnti << ", " << imsi);
    }
  else if (path.find ("/UlPhyReception"))
    {
      std::string p = path.substr (0, path.find ("/NrUePhy"));
      imsi = FindImsiFromNrNetDevice (p);
      NS_LOG_LOGIC ("FindImsiForEnb[Rx]: " << path << ", " << rnti << ", " << imsi);
    }
  return imsi;
}


uint64_t
NrStatsCalculator::FindImsiForUe (std::string path, uint16_t rnti)
{
  NS_LOG_FUNCTION (path << rnti);
  uint64_t imsi = 0;
  if (path.find ("/UlPhyTransmission"))
    {
      std::string p = path.substr (0, path.find ("/NrUePhy"));
      imsi = FindImsiFromNrNetDevice (p);
      NS_LOG_LOGIC ("FindImsiForUe[Tx]: " << path << ", " << rnti << ", " << imsi);
    }
  else if (path.find ("/DlPhyReception"))
    {
      // /NodeList/0/DeviceList/0/NrEnbPhy/NrSpectrumPhy
      std::ostringstream oss;
      std::string p = path.substr (0, path.find ("/NrEnbPhy"));
      oss << rnti;
      p += "/NrEnbRrc/UeMap/" + oss.str ();
      imsi = FindImsiFromEnbRlcPath (p);
      NS_LOG_LOGIC ("FindImsiForUe[Rx]: " << path << ", " << rnti << ", " << imsi);
    }
  return imsi;
}


} // namespace ns3
