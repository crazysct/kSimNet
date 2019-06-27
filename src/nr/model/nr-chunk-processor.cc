/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Modified by : Marco Miozzo <mmiozzo@cttc.es>
 *        (move from CQI to Ctrl and Data SINR Chunk processors
 */


#include <ns3/log.h>
#include <ns3/spectrum-value.h>
#include "nr-chunk-processor.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrChunkProcessor");

NrChunkProcessor::NrChunkProcessor ()
{
  NS_LOG_FUNCTION (this);
}

NrChunkProcessor::~NrChunkProcessor ()
{
  NS_LOG_FUNCTION (this);
}

void
NrChunkProcessor::AddCallback (NrChunkProcessorCallback c)
{
  NS_LOG_FUNCTION (this);
  m_nrChunkProcessorCallbacks.push_back (c);
}

void
NrChunkProcessor::Start ()
{
  NS_LOG_FUNCTION (this);
  m_sumValues = 0;
  m_totDuration = MicroSeconds (0);
}


void
NrChunkProcessor::EvaluateChunk (const SpectrumValue& sinr, Time duration)
{
  NS_LOG_FUNCTION (this << sinr << duration);
  if (m_sumValues == 0)
    {
      m_sumValues = Create<SpectrumValue> (sinr.GetSpectrumModel ());
    }
  (*m_sumValues) += sinr * duration.GetSeconds ();
  m_totDuration += duration;
}

void
NrChunkProcessor::End ()
{
  NS_LOG_FUNCTION (this);
  if (m_totDuration.GetSeconds () > 0)
    {
      std::vector<NrChunkProcessorCallback>::iterator it;
      for (it = m_nrChunkProcessorCallbacks.begin (); it != m_nrChunkProcessorCallbacks.end (); it++)
        {
          (*it)((*m_sumValues) / m_totDuration.GetSeconds ());
        }
    }
  else
    {
      NS_LOG_WARN ("m_numSinr == 0");
    }
}


  
void
NrSpectrumValueCatcher::ReportValue (const SpectrumValue& value)
{
  m_value = value.Copy ();
}

Ptr<SpectrumValue> 
NrSpectrumValueCatcher::GetValue ()
{
  return m_value;
}


} // namespace ns3
