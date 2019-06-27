/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2017 SIGNET Lab, Dept. of Information Engineering, UNIPD
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
 *         Nicola Baldo <nbaldo@cttc.es>
 * Modified by: Michele Polese <michele.polese@gmail.com>
 */

#include "nr-mac-tx-stats-calculator.h"
#include "ns3/string.h"
#include "ns3/nstime.h"
#include <ns3/log.h>
#include <vector>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacTxStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED ( NrMacTxStatsCalculator);

NrMacTxStatsCalculator::NrMacTxStatsCalculator ()
{
  NS_LOG_FUNCTION (this);
}


NrMacTxStatsCalculator::~NrMacTxStatsCalculator ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrMacTxStatsCalculator::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::NrMacTxStatsCalculator")
    .SetParent<Object> ().AddConstructor<NrMacTxStatsCalculator> ()
    .SetGroupName("Nr")
    .AddAttribute ("DlMacTxFilename",
                   "Name of the file where the MAC downlink tx results will be saved.",
                   StringValue ("DlMacTx.txt"),
                   MakeStringAccessor (&NrMacTxStatsCalculator::m_retxDlFilename),
                   MakeStringChecker ())
    .AddAttribute ("UlMacTxFilename",
                   "Name of the file where the MAC uplink tx results will be saved.",
                   StringValue ("UlMacTx.txt"),
                   MakeStringAccessor (&NrMacTxStatsCalculator::m_retxUlFilename),
                   MakeStringChecker ())
    ;
  return tid;
}

void
NrMacTxStatsCalculator::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

void
NrMacTxStatsCalculator::RegisterMacTxDl(uint16_t rnti, uint16_t cellId, uint32_t packetSize, uint8_t numRetx)
{
	if(!m_retxDlFile.is_open())
	{
	    m_retxDlFile.open(m_retxDlFilename.c_str());
	    NS_LOG_LOGIC("File opened");
	}
  NS_LOG_LOGIC(rnti << cellId << packetSize << numRetx);
	m_retxDlFile << Simulator::Now().GetSeconds() << " " << cellId << " " << rnti << " "  << packetSize << " " << (uint32_t)numRetx << std::endl;
}

void
NrMacTxStatsCalculator::RegisterMacTxUl(uint16_t rnti, uint16_t cellId, uint32_t packetSize, uint8_t numRetx)
{
	if(!m_retxUlFile.is_open())
	{
	    m_retxUlFile.open(m_retxUlFilename.c_str());
	    NS_LOG_LOGIC("File opened");
	}
  m_retxUlFile << Simulator::Now().GetSeconds() << " " << cellId << " " << rnti << " "  << packetSize << " " << (uint32_t)numRetx << std::endl;
}

}