/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Piotr Gawlowicz
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
 * Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>
 *
 */

#include "nr-ue-power-control.h"
#include <ns3/log.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/math.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrUePowerControl");

NS_OBJECT_ENSURE_REGISTERED (NrUePowerControl);

NrUePowerControl::NrUePowerControl ()
{
  NS_LOG_FUNCTION (this);
  m_deltaTF = 0;
  m_fc = 0;
  m_pathLoss = 100; //initial value
  m_curPuschTxPower = 10;
  m_curPucchTxPower = 10;
  m_curSrsTxPower = 10;
  m_txPower = 10;

  m_cellId = 0;
  m_rnti = 0;

  m_M_Pusch = 0;
  m_rsrpSet = false;
}

NrUePowerControl::~NrUePowerControl ()
{
  NS_LOG_FUNCTION (this);
}

void
NrUePowerControl::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
}

void
NrUePowerControl::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrUePowerControl::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrUePowerControl")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    .AddConstructor<NrUePowerControl> ()
    .AddAttribute ("ClosedLoop",
                   "If true Closed Loop mode will be active, otherwise Open Loop",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrUePowerControl::m_closedLoop),
                   MakeBooleanChecker ())
    .AddAttribute ("AccumulationEnabled",
                   "If true TCP accumulation mode will be active, otherwise absolute mode will be active",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrUePowerControl::m_accumulationEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("Alpha",
                   "Value of Alpha paramter",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&NrUePowerControl::SetAlpha),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Pcmax",
                   "Max Transmission power in dBm, Default value 23 dBm"
                   "TS36.101 section 6.2.3",
                   DoubleValue (23.0),
                   MakeDoubleAccessor (&NrUePowerControl::m_Pcmax),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Pcmin",
                   "Min Transmission power in dBm, Default value -40 dBm"
                   "TS36.101 section 6.2.3",
                   DoubleValue (-40),
                   MakeDoubleAccessor (&NrUePowerControl::m_Pcmin),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("PoNominalPusch",
                   "P_O_NOMINAL_PUSCH   INT (-126 ... 24), Default value -80",
                   IntegerValue (-80),
                   MakeIntegerAccessor (&NrUePowerControl::SetPoNominalPusch),
                   MakeIntegerChecker<int16_t> ())
    .AddAttribute ("PoUePusch",
                   "P_O_UE_PUSCH   INT(-8...7), Default value 0",
                   IntegerValue (0),
                   MakeIntegerAccessor (&NrUePowerControl::SetPoUePusch),
                   MakeIntegerChecker<int16_t> ())
    .AddAttribute ("PsrsOffset",
                   "P_SRS_OFFSET   INT(0...15), Default value 7",
                   IntegerValue (7),
                   MakeIntegerAccessor (&NrUePowerControl::m_PsrsOffset),
                   MakeIntegerChecker<int16_t> ())
    .AddTraceSource ("ReportPuschTxPower",
                     "Report PUSCH TxPower in dBm",
                     MakeTraceSourceAccessor (&NrUePowerControl::m_reportPuschTxPower),
                     "ns3::NrUePowerControl::TxPowerTracedCallback")
    .AddTraceSource ("ReportPucchTxPower",
                     "Report PUCCH TxPower in dBm",
                     MakeTraceSourceAccessor (&NrUePowerControl::m_reportPucchTxPower),
                     "ns3::NrUePowerControl::TxPowerTracedCallback")
    .AddTraceSource ("ReportSrsTxPower",
                     "Report SRS TxPower in dBm",
                     MakeTraceSourceAccessor (&NrUePowerControl::m_reportSrsTxPower),
                     "ns3::NrUePowerControl::TxPowerTracedCallback")
  ;
  return tid;
}

void
NrUePowerControl::SetPcmax (double value)
{
  NS_LOG_FUNCTION (this);
  m_Pcmax = value;
}

double
NrUePowerControl::GetPcmax ()
{
  NS_LOG_FUNCTION (this);
  return m_Pcmax;
}

void
NrUePowerControl::SetTxPower (double value)
{
  NS_LOG_FUNCTION (this);
  m_txPower = value;
  m_curPuschTxPower = value;
  m_curPucchTxPower = value;
  m_curSrsTxPower = value;
}

void
NrUePowerControl::ConfigureReferenceSignalPower (int8_t referenceSignalPower)
{
  NS_LOG_FUNCTION (this);
  m_referenceSignalPower = referenceSignalPower;
}

void
NrUePowerControl::SetCellId (uint16_t cellId)
{
  NS_LOG_FUNCTION (this);
  m_cellId = cellId;
}
void
NrUePowerControl::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  m_rnti = rnti;
}

void
NrUePowerControl::SetPoNominalPusch (int16_t value)
{
  NS_LOG_FUNCTION (this);

  if (m_PoNominalPusch.empty ())
    {
      m_PoNominalPusch.push_back (value);
      m_PoNominalPusch.push_back (value);
      m_PoNominalPusch.push_back (value);
    }
  else
    {
      m_PoNominalPusch[0] = value;
      m_PoNominalPusch[1] = value;
      m_PoNominalPusch[2] = value;
    }

}
void
NrUePowerControl::SetPoUePusch (int16_t value)
{
  NS_LOG_FUNCTION (this);
  if (m_PoUePusch.empty ())
    {
      m_PoUePusch.push_back (value);
      m_PoUePusch.push_back (value);
      m_PoUePusch.push_back (0);
    }
  else
    {
      m_PoUePusch[0] = value;
      m_PoUePusch[1] = value;
      m_PoUePusch[2] = 0;
    }
}
void
NrUePowerControl::SetAlpha (double value)
{
  NS_LOG_FUNCTION (this);

  uint32_t temp = value * 10;
  switch (temp)
    {
    case 0:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      break;
    default:
      NS_FATAL_ERROR ("Unexpected Alpha value");
    }

  if (m_alpha.empty ())
    {
      m_alpha.push_back (value);
      m_alpha.push_back (value);
      m_alpha.push_back (0);
    }
  else
    {
      m_alpha[0] = value;
      m_alpha[1] = value;
      m_alpha[2] = 1;
    }

}

void
NrUePowerControl::SetRsrp (double value)
{
  NS_LOG_FUNCTION (this);

  if (!m_rsrpSet)
    {
      m_rsrp = value;
      m_rsrpSet = true;
      return;
    }

  double coeff = 0.7;
  m_rsrp = coeff * m_rsrp + (1 - coeff) * value;
  m_pathLoss = m_referenceSignalPower - m_rsrp;
}

void
NrUePowerControl::ReportTpc (uint8_t tpc)
{
  NS_LOG_FUNCTION (this);

  int delta = 0;
  if (m_accumulationEnabled)
    {
      switch (tpc)
        {
        case 0:
          delta = -1;
          break;
        case 1:
          delta = 0;
          break;
        case 2:
          delta = 1;
          break;
        case 3:
          delta = 3;
          break;
        default:
          NS_FATAL_ERROR ("Unexpected TPC value");
        }
    }
  else
    {
      switch (tpc)
        {
        case 0:
          delta = -4;
          break;
        case 1:
          delta = -1;
          break;
        case 2:
          delta = 1;
          break;
        case 3:
          delta = 4;
          break;
        default:
          NS_FATAL_ERROR ("Unexpected TPC value");
        }
    }

  m_deltaPusch.push_back (delta);

  if (m_closedLoop)
    {
      if (m_accumulationEnabled)
        {
          if (m_deltaPusch.size () == 4)
            {
              if ((m_curPuschTxPower <= m_Pcmin && m_deltaPusch.at (0) < 0)
                  || (m_curPuschTxPower >= m_Pcmax && m_deltaPusch.at (0) > 0))
                {
                  //TPC commands for serving cell shall not be accumulated
                  m_deltaPusch.erase (m_deltaPusch.begin ());
                }
              else
                {
                  m_fc = m_fc + m_deltaPusch.at (0);
                  m_deltaPusch.erase (m_deltaPusch.begin ());
                }
            }
          else
            {
              m_fc = 0;
            }
        }
      else
        {
          m_fc = m_deltaPusch.at (0);
          m_deltaPusch.erase (m_deltaPusch.begin ());
        }
    }
  else
    {
      m_fc = 0;
    }

  NS_LOG_INFO ("ReportTpc: " << (int)tpc << " delta: " << delta << " Fc: " << m_fc);
}

void
NrUePowerControl::SetSubChannelMask (std::vector <int> mask)
{
  NS_LOG_FUNCTION (this);
  m_M_Pusch = mask.size ();
}

void
NrUePowerControl::CalculatePuschTxPower ()
{
  NS_LOG_FUNCTION (this);
  int32_t j = 1;
  int32_t PoPusch = m_PoNominalPusch[j] + m_PoUePusch[j];

  NS_LOG_INFO ("RB: " << m_M_Pusch << " m_PoPusch: " << PoPusch
                      << " Alpha: " << m_alpha[j] << " PathLoss: " << m_pathLoss
                      << " deltaTF: " << m_deltaTF << " fc: " << m_fc);

  if ( m_M_Pusch > 0 )
    {
      m_curPuschTxPower = 10 * log10 (1.0 * m_M_Pusch) + PoPusch + m_alpha[j] * m_pathLoss + m_deltaTF + m_fc;
      m_M_Pusch = 0;
    }
  else
    {
      m_curPuschTxPower = PoPusch + m_alpha[j] * m_pathLoss + m_fc;
    }

  NS_LOG_INFO ("CalcPower: " << m_curPuschTxPower << " MinPower: " << m_Pcmin << " MaxPower:" << m_Pcmax);

  m_curPuschTxPower = m_curPuschTxPower > m_Pcmin ? m_curPuschTxPower : m_Pcmin;
  m_curPuschTxPower = m_Pcmax < m_curPuschTxPower ? m_Pcmax : m_curPuschTxPower;
  NS_LOG_INFO ("PuschTxPower: " << m_curPuschTxPower);
}

void
NrUePowerControl::CalculatePucchTxPower ()
{
  NS_LOG_FUNCTION (this);
  m_curPucchTxPower = m_curPuschTxPower;
  NS_LOG_INFO ("PucchTxPower: " << m_curPucchTxPower);
}

void
NrUePowerControl::CalculateSrsTxPower ()
{
  NS_LOG_FUNCTION (this);
  int32_t j = 1;
  int32_t PoPusch = m_PoNominalPusch[j] + m_PoUePusch[j];

  NS_LOG_INFO ("RB: " << m_M_Pusch << " m_PoPusch: " << PoPusch
                      << " Alpha: " << m_alpha[j] << " PathLoss: " << m_pathLoss
                      << " deltaTF: " << m_deltaTF << " fc: " << m_fc);


  double pSrsOffsetValue = -10.5 + m_PsrsOffset * 1.5;

  m_curSrsTxPower = pSrsOffsetValue + 10 * log10 (m_srsBandwidth) + PoPusch + m_alpha[j] * m_pathLoss + m_fc;

  NS_LOG_INFO ("CalcPower: " << m_curSrsTxPower << " MinPower: " << m_Pcmin << " MaxPower:" << m_Pcmax);

  m_curSrsTxPower = m_curSrsTxPower > m_Pcmin ? m_curSrsTxPower : m_Pcmin;
  m_curSrsTxPower = m_Pcmax < m_curSrsTxPower ? m_Pcmax : m_curSrsTxPower;
  NS_LOG_INFO ("SrsTxPower: " << m_curSrsTxPower);
}


double
NrUePowerControl::GetPuschTxPower (std::vector <int> dlRb)
{
  NS_LOG_FUNCTION (this);

  m_M_Pusch = dlRb.size ();
  CalculatePuschTxPower ();

  m_reportPuschTxPower (m_cellId, m_rnti, m_curPuschTxPower);

  return m_curPuschTxPower;
}

double
NrUePowerControl::GetPucchTxPower (std::vector <int> dlRb)
{
  NS_LOG_FUNCTION (this);

  CalculatePucchTxPower ();

  m_reportPucchTxPower (m_cellId, m_rnti, m_curPucchTxPower);

  return m_curPucchTxPower;
}

double
NrUePowerControl::GetSrsTxPower (std::vector <int> dlRb)
{
  NS_LOG_FUNCTION (this);

  m_srsBandwidth = dlRb.size ();
  CalculateSrsTxPower ();

  m_reportSrsTxPower (m_cellId, m_rnti, m_curSrsTxPower);

  return m_curSrsTxPower;
}

} // namespace ns3
