/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009, 2011 CTTC
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *         Giuseppe Piro  <g.piro@poliba.it>
 *         Marco Miozzo <marco.miozzo@cttc.es> (add physical error model)
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */


#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <cmath>
#include <ns3/simulator.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/antenna-model.h>
#include "nr-spectrum-phy.h"
#include "nr-spectrum-signal-parameters.h"
#include "nr-net-device.h"
#include "nr-radio-bearer-tag.h"
#include "nr-chunk-processor.h"
#include "nr-phy-tag.h"
#include <ns3/nr-mi-error-model.h>
#include <ns3/nr-radio-bearer-tag.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/config.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSpectrumPhy");


// duration of SRS portion of UL subframe  
// = 1 symbol for SRS -1ns as margin to avoid overlapping simulator events
static const Time UL_SRS_DURATION = NanoSeconds (71429 -1);  

// duration of the control portion of a subframe
// = 0.001 / 14 * 3 (ctrl fixed to 3 symbols) -1ns as margin to avoid overlapping simulator events
static const Time DL_CTRL_DURATION = NanoSeconds (214286 -1);

static const double EffectiveCodingRate[29] = {
  0.08,
  0.1,
  0.11,
  0.15,
  0.19,
  0.24,
  0.3,
  0.37,
  0.44,
  0.51,
  0.3,
  0.33,
  0.37,
  0.42,
  0.48,
  0.54,
  0.6,
  0.43,
  0.45,
  0.5,
  0.55,
  0.6,
  0.65,
  0.7,
  0.75,
  0.8,
  0.85,
  0.89,
  0.92
};



  
TbId_t::TbId_t ()
{
}

TbId_t::TbId_t (const uint16_t a, const uint8_t b)
: m_rnti (a),
  m_layer (b)
{
}

bool
operator == (const TbId_t &a, const TbId_t &b)
{
  return ( (a.m_rnti == b.m_rnti) && (a.m_layer == b.m_layer) );
}

bool
operator < (const TbId_t& a, const TbId_t& b)
{
  return ( (a.m_rnti < b.m_rnti) || ( (a.m_rnti == b.m_rnti) && (a.m_layer < b.m_layer) ) );
}

NS_OBJECT_ENSURE_REGISTERED (NrSpectrumPhy);

NrSpectrumPhy::NrSpectrumPhy ()
  : m_state (IDLE),
    m_cellId (0),
  m_transmissionMode (0),
  m_layersNum (1)
{
  NS_LOG_FUNCTION (this);
  m_random = CreateObject<UniformRandomVariable> ();
  m_random->SetAttribute ("Min", DoubleValue (0.0));
  m_random->SetAttribute ("Max", DoubleValue (1.0));
  m_interferenceData = CreateObject<NrInterference> ();
  m_interferenceCtrl = CreateObject<NrInterference> ();

  for (uint8_t i = 0; i < 7; i++)
    {
      m_txModeGain.push_back (1.0);
    }
}


NrSpectrumPhy::~NrSpectrumPhy ()
{
  NS_LOG_FUNCTION (this);
  m_expectedTbs.clear ();
  m_txModeGain.clear ();
}

void NrSpectrumPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_channel = 0;
  m_mobility = 0;
  m_device = 0;
  m_interferenceData->Dispose ();
  m_interferenceData = 0;
  m_interferenceCtrl->Dispose ();
  m_interferenceCtrl = 0;
  m_nrPhyRxDataEndErrorCallback = MakeNullCallback< void > ();
  m_nrPhyRxDataEndOkCallback    = MakeNullCallback< void, Ptr<Packet> >  ();
  m_nrPhyRxCtrlEndOkCallback = MakeNullCallback< void, std::list<Ptr<NrControlMessage> > > ();
  m_nrPhyRxCtrlEndErrorCallback = MakeNullCallback< void > ();
  m_nrPhyDlHarqFeedbackCallback = MakeNullCallback< void, NrDlInfoListElement_s > ();
  m_nrPhyUlHarqFeedbackCallback = MakeNullCallback< void, NrUlInfoListElement_s > ();
  m_nrPhyRxPssCallback = MakeNullCallback< void, uint16_t, Ptr<SpectrumValue> > ();
  SpectrumPhy::DoDispose ();
} 

std::ostream& operator<< (std::ostream& os, NrSpectrumPhy::State s)
{
  switch (s)
    {
    case NrSpectrumPhy::IDLE:
      os << "IDLE";
      break;
    case NrSpectrumPhy::RX_DATA:
      os << "RX_DATA";
      break;
    case NrSpectrumPhy::RX_DL_CTRL:
      os << "RX_DL_CTRL";
      break;
    case NrSpectrumPhy::TX_DATA:
      os << "TX_DATA";
      break;
    case NrSpectrumPhy::TX_DL_CTRL:
      os << "TX_DL_CTRL";
      break;
    case NrSpectrumPhy::TX_UL_SRS:
      os << "TX_UL_SRS";
      break;
    default:
      os << "UNKNOWN";
      break;
    }
  return os;
}

TypeId
NrSpectrumPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSpectrumPhy")
    .SetParent<SpectrumPhy> ()
    .SetGroupName("Nr")
    .AddTraceSource ("TxStart",
                     "Trace fired when a new transmission is started",
                     MakeTraceSourceAccessor (&NrSpectrumPhy::m_phyTxStartTrace),
                     "ns3::PacketBurst::TracedCallback")
    .AddTraceSource ("TxEnd",
                     "Trace fired when a previosuly started transmission is finished",
                     MakeTraceSourceAccessor (&NrSpectrumPhy::m_phyTxEndTrace),
                     "ns3::PacketBurst::TracedCallback")
    .AddTraceSource ("RxStart",
                     "Trace fired when the start of a signal is detected",
                     MakeTraceSourceAccessor (&NrSpectrumPhy::m_phyRxStartTrace),
                     "ns3::PacketBurst::TracedCallback")
    .AddTraceSource ("RxEndOk",
                     "Trace fired when a previosuly started RX terminates successfully",
                     MakeTraceSourceAccessor (&NrSpectrumPhy::m_phyRxEndOkTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("RxEndError",
                     "Trace fired when a previosuly started RX terminates with an error",
                     MakeTraceSourceAccessor (&NrSpectrumPhy::m_phyRxEndErrorTrace),
                     "ns3::Packet::TracedCallback")
    .AddAttribute ("DataErrorModelEnabled",
                    "Activate/Deactivate the error model of data (TBs of PDSCH and PUSCH) [by default is active].",
                    BooleanValue (true),
                   MakeBooleanAccessor (&NrSpectrumPhy::m_dataErrorModelEnabled),
                    MakeBooleanChecker ())
    .AddAttribute ("CtrlErrorModelEnabled",
                    "Activate/Deactivate the error model of control (PCFICH-PDCCH decodification) [by default is active].",
                    BooleanValue (true),
                    MakeBooleanAccessor (&NrSpectrumPhy::m_ctrlErrorModelEnabled),
                    MakeBooleanChecker ())
    .AddTraceSource ("DlPhyReception",
                     "DL reception PHY layer statistics.",
                     MakeTraceSourceAccessor (&NrSpectrumPhy::m_dlPhyReception),
                     "ns3::NrPhyReceptionStatParameters::TracedCallback")
    .AddTraceSource ("UlPhyReception",
                     "DL reception PHY layer statistics.",
                     MakeTraceSourceAccessor (&NrSpectrumPhy::m_ulPhyReception),
                     "ns3::NrPhyReceptionStatParameters::TracedCallback")
  ;
  return tid;
}



Ptr<NetDevice>
NrSpectrumPhy::GetDevice () const
{
  NS_LOG_FUNCTION (this);
  return m_device;
}


Ptr<MobilityModel>
NrSpectrumPhy::GetMobility ()
{
  NS_LOG_FUNCTION (this);
  return m_mobility;
}


void
NrSpectrumPhy::SetDevice (Ptr<NetDevice> d)
{
  NS_LOG_FUNCTION (this << d);
  m_device = d;
}


void
NrSpectrumPhy::SetMobility (Ptr<MobilityModel> m)
{
  NS_LOG_FUNCTION (this << m);
  m_mobility = m;
}


void
NrSpectrumPhy::SetChannel (Ptr<SpectrumChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  m_channel = c;
}

Ptr<SpectrumChannel> 
NrSpectrumPhy::GetChannel ()
{
  return m_channel;
}

Ptr<const SpectrumModel>
NrSpectrumPhy::GetRxSpectrumModel () const
{
  return m_rxSpectrumModel;
}


void
NrSpectrumPhy::SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd)
{
  NS_LOG_FUNCTION (this << txPsd);
  NS_ASSERT (txPsd);
  m_txPsd = txPsd;
}


void
NrSpectrumPhy::SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd)
{
  NS_LOG_FUNCTION (this << noisePsd);
  NS_ASSERT (noisePsd);
  m_rxSpectrumModel = noisePsd->GetSpectrumModel ();
  m_interferenceData->SetNoisePowerSpectralDensity (noisePsd);
  m_interferenceCtrl->SetNoisePowerSpectralDensity (noisePsd);
}

  
void 
NrSpectrumPhy::Reset ()
{
  NS_LOG_FUNCTION (this);
  m_cellId = 0;
  m_state = IDLE;
  m_transmissionMode = 0;
  m_layersNum = 1;
  m_endTxEvent.Cancel ();
  m_endRxDataEvent.Cancel ();
  m_endRxDlCtrlEvent.Cancel ();
  m_endRxUlSrsEvent.Cancel ();
  m_rxControlMessageList.clear ();
  m_expectedTbs.clear ();
  m_txControlMessageList.clear ();
  m_rxPacketBurstList.clear ();
  m_txPacketBurst = 0;
  //m_rxSpectrumModel = 0;
}


void
NrSpectrumPhy::SetNrPhyRxDataEndErrorCallback (NrPhyRxDataEndErrorCallback c)
{
  NS_LOG_FUNCTION (this);
  m_nrPhyRxDataEndErrorCallback = c;
}


void
NrSpectrumPhy::SetNrPhyRxDataEndOkCallback (NrPhyRxDataEndOkCallback c)
{
  NS_LOG_FUNCTION (this);
  m_nrPhyRxDataEndOkCallback = c;
}

void
NrSpectrumPhy::SetNrPhyRxCtrlEndOkCallback (NrPhyRxCtrlEndOkCallback c)
{
  NS_LOG_FUNCTION (this);
  m_nrPhyRxCtrlEndOkCallback = c;
}

void
NrSpectrumPhy::SetNrPhyRxCtrlEndErrorCallback (NrPhyRxCtrlEndErrorCallback c)
{
  NS_LOG_FUNCTION (this);
  m_nrPhyRxCtrlEndErrorCallback = c;
}


void
NrSpectrumPhy::SetNrPhyRxPssCallback (NrPhyRxPssCallback c)
{
  NS_LOG_FUNCTION (this);
  m_nrPhyRxPssCallback = c;
}

void
NrSpectrumPhy::SetNrPhyDlHarqFeedbackCallback (NrPhyDlHarqFeedbackCallback c)
{
  NS_LOG_FUNCTION (this);
  m_nrPhyDlHarqFeedbackCallback = c;
}

void
NrSpectrumPhy::SetNrPhyUlHarqFeedbackCallback (NrPhyUlHarqFeedbackCallback c)
{
  NS_LOG_FUNCTION (this);
  m_nrPhyUlHarqFeedbackCallback = c;
}


Ptr<AntennaModel>
NrSpectrumPhy::GetRxAntenna ()
{
  return m_antenna;
}

void
NrSpectrumPhy::SetAntenna (Ptr<AntennaModel> a)
{
  NS_LOG_FUNCTION (this << a);
  m_antenna = a;
}

void
NrSpectrumPhy::SetState (State newState)
{
  ChangeState (newState);
}


void
NrSpectrumPhy::ChangeState (State newState)
{
  NS_LOG_LOGIC (this << " state: " << m_state << " -> " << newState);
  m_state = newState;
}


void
NrSpectrumPhy::SetHarqPhyModule (Ptr<NrHarqPhy> harq)
{
  m_harqPhyModule = harq;
}




bool
NrSpectrumPhy::StartTxDataFrame (Ptr<PacketBurst> pb, std::list<Ptr<NrControlMessage> > ctrlMsgList, Time duration)
{
  NS_LOG_FUNCTION (this << pb);
  NS_LOG_LOGIC (this << " state: " << m_state);
  
  m_phyTxStartTrace (pb);
  
  switch (m_state)
    {
    case RX_DATA:
    case RX_DL_CTRL:
    case RX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while RX: according to FDD channel acces, the physical layer for transmission cannot be used for reception");
      break;

    case TX_DATA:
    case TX_DL_CTRL:      
    case TX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while already TX: the MAC should avoid this");
      break;
      
    case IDLE:
    {
      /*
      m_txPsd must be setted by the device, according to
      (i) the available subchannel for transmission
      (ii) the power transmission
      */
      NS_ASSERT (m_txPsd);
      m_txPacketBurst = pb;
      
      // we need to convey some PHY meta information to the receiver
      // to be used for simulation purposes (e.g., the CellId). This
      // is done by setting the ctrlMsgList parameter of
      // NrSpectrumSignalParametersDataFrame
      ChangeState (TX_DATA);
      NS_ASSERT (m_channel);
      Ptr<NrSpectrumSignalParametersDataFrame> txParams = Create<NrSpectrumSignalParametersDataFrame> ();
      txParams->duration = duration;
      txParams->txPhy = GetObject<SpectrumPhy> ();
      txParams->txAntenna = m_antenna;
      txParams->psd = m_txPsd;
      txParams->packetBurst = pb;
      txParams->ctrlMsgList = ctrlMsgList;
      txParams->cellId = m_cellId;
      m_channel->StartTx (txParams);
      m_endTxEvent = Simulator::Schedule (duration, &NrSpectrumPhy::EndTxData, this);
    }
    return false;
    break;
    
    default:
      NS_FATAL_ERROR ("unknown state");
      return true;
      break;
  }
}

bool
NrSpectrumPhy::StartTxDlCtrlFrame (std::list<Ptr<NrControlMessage> > ctrlMsgList, bool pss)
{
  NS_LOG_FUNCTION (this << " PSS " << (uint16_t)pss);
  NS_LOG_LOGIC (this << " state: " << m_state);
  
  switch (m_state)
  {
    case RX_DATA:
    case RX_DL_CTRL:
    case RX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while RX: according to FDD channel acces, the physical layer for transmission cannot be used for reception");
      break;
      
    case TX_DATA:
    case TX_DL_CTRL:
    case TX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while already TX: the MAC should avoid this");
      break;
      
    case IDLE:
    {
      /*
      m_txPsd must be setted by the device, according to
      (i) the available subchannel for transmission
      (ii) the power transmission
      */
      NS_ASSERT (m_txPsd);
      
      // we need to convey some PHY meta information to the receiver
      // to be used for simulation purposes (e.g., the CellId). This
      // is done by setting the cellId parameter of
      // NrSpectrumSignalParametersDlCtrlFrame
      ChangeState (TX_DL_CTRL);
      NS_ASSERT (m_channel);

      Ptr<NrSpectrumSignalParametersDlCtrlFrame> txParams = Create<NrSpectrumSignalParametersDlCtrlFrame> ();
      txParams->duration = DL_CTRL_DURATION;
      txParams->txPhy = GetObject<SpectrumPhy> ();
      txParams->txAntenna = m_antenna;
      txParams->psd = m_txPsd;
      txParams->cellId = m_cellId;
      txParams->pss = pss;
      txParams->ctrlMsgList = ctrlMsgList;
      m_channel->StartTx (txParams);
      m_endTxEvent = Simulator::Schedule (DL_CTRL_DURATION, &NrSpectrumPhy::EndTxDlCtrl, this);
    }
    return false;
    break;
    
    default:
      NS_FATAL_ERROR ("unknown state");
      return true;
      break;
  }
}


bool
NrSpectrumPhy::StartTxUlSrsFrame ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);
  
  switch (m_state)
    {
    case RX_DATA:
    case RX_DL_CTRL:
    case RX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while RX: according to FDD channel acces, the physical layer for transmission cannot be used for reception");
      break;
      
    case TX_DL_CTRL:
    case TX_DATA:
    case TX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while already TX: the MAC should avoid this");
      break;
      
    case IDLE:
    {
      /*
      m_txPsd must be setted by the device, according to
      (i) the available subchannel for transmission
      (ii) the power transmission
      */
      NS_ASSERT (m_txPsd);
      NS_LOG_LOGIC (this << " m_txPsd: " << *m_txPsd);
      
      // we need to convey some PHY meta information to the receiver
      // to be used for simulation purposes (e.g., the CellId). This
      // is done by setting the cellId parameter of 
      // NrSpectrumSignalParametersDlCtrlFrame
      ChangeState (TX_UL_SRS);
      NS_ASSERT (m_channel);
      Ptr<NrSpectrumSignalParametersUlSrsFrame> txParams = Create<NrSpectrumSignalParametersUlSrsFrame> ();
      txParams->duration = UL_SRS_DURATION;
      txParams->txPhy = GetObject<SpectrumPhy> ();
      txParams->txAntenna = m_antenna;
      txParams->psd = m_txPsd;
      txParams->cellId = m_cellId;
      m_channel->StartTx (txParams);
      m_endTxEvent = Simulator::Schedule (UL_SRS_DURATION, &NrSpectrumPhy::EndTxUlSrs, this);
    }
    return false;
    break;
    
    default:
      NS_FATAL_ERROR ("unknown state");
      return true;
      break;
  }
}



void
NrSpectrumPhy::EndTxData ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  NS_ASSERT (m_state == TX_DATA);
  m_phyTxEndTrace (m_txPacketBurst);
  m_txPacketBurst = 0;
  ChangeState (IDLE);
}

void
NrSpectrumPhy::EndTxDlCtrl ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  NS_ASSERT (m_state == TX_DL_CTRL);
  NS_ASSERT (m_txPacketBurst == 0);
  ChangeState (IDLE);
}

void
NrSpectrumPhy::EndTxUlSrs ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  NS_ASSERT (m_state == TX_UL_SRS);
  NS_ASSERT (m_txPacketBurst == 0);
  ChangeState (IDLE);
}




void
NrSpectrumPhy::StartRx (Ptr<SpectrumSignalParameters> spectrumRxParams)
{
  NS_LOG_FUNCTION (this << spectrumRxParams);
  NS_LOG_LOGIC (this << " state: " << m_state);
  
  Ptr <const SpectrumValue> rxPsd = spectrumRxParams->psd;
  Time duration = spectrumRxParams->duration;
  
  // the device might start RX only if the signal is of a type
  // understood by this device - in this case, an NR signal.
  Ptr<NrSpectrumSignalParametersDataFrame> nrDataRxParams = DynamicCast<NrSpectrumSignalParametersDataFrame> (spectrumRxParams);
  Ptr<NrSpectrumSignalParametersDlCtrlFrame> nrDlCtrlRxParams = DynamicCast<NrSpectrumSignalParametersDlCtrlFrame> (spectrumRxParams);
  Ptr<NrSpectrumSignalParametersUlSrsFrame> nrUlSrsRxParams = DynamicCast<NrSpectrumSignalParametersUlSrsFrame> (spectrumRxParams);
  if (nrDataRxParams != 0)
    {
      m_interferenceData->AddSignal (rxPsd, duration);
      StartRxData (nrDataRxParams);
    }
  else if (nrDlCtrlRxParams!=0)
    {
      m_interferenceCtrl->AddSignal (rxPsd, duration);
      StartRxDlCtrl (nrDlCtrlRxParams);
    }
  else if (nrUlSrsRxParams!=0)
    {
      m_interferenceCtrl->AddSignal (rxPsd, duration);
      StartRxUlSrs (nrUlSrsRxParams);
    }
  else
    {
      // other type of signal (could be 3G, GSM, whatever) -> interference
      m_interferenceData->AddSignal (rxPsd, duration);
      m_interferenceCtrl->AddSignal (rxPsd, duration);
    }    
}

void
NrSpectrumPhy::StartRxData (Ptr<NrSpectrumSignalParametersDataFrame> params)
{
  NS_LOG_FUNCTION (this);
  switch (m_state)
    {
      case TX_DATA:
      case TX_DL_CTRL:
      case TX_UL_SRS:
        NS_FATAL_ERROR ("cannot RX while TX: according to FDD channel access, the physical layer for transmission cannot be used for reception");
        break;
      case RX_DL_CTRL:
        NS_FATAL_ERROR ("cannot RX Data while receiving control");
        break;
      case IDLE:
      case RX_DATA:
        // the behavior is similar when
        // we're IDLE or RX because we can receive more signals
        // simultaneously (e.g., at the eNB).
        {
          // To check if we're synchronized to this signal, we check
          // for the CellId which is reported in the
          //  NrSpectrumSignalParametersDataFrame
          if (params->cellId  == m_cellId)
            {
              NS_LOG_LOGIC (this << " synchronized with this signal (cellId=" << params->cellId << ")");
              if ((m_rxPacketBurstList.empty ())&&(m_rxControlMessageList.empty ()))
                {
                  NS_ASSERT (m_state == IDLE);
                  // first transmission, i.e., we're IDLE and we
                  // start RX
                  m_firstRxStart = Simulator::Now ();
                  m_firstRxDuration = params->duration;
                  NS_LOG_LOGIC (this << " scheduling EndRx with delay " << params->duration.GetSeconds () << "s");
                  m_endRxDataEvent = Simulator::Schedule (params->duration, &NrSpectrumPhy::EndRxData, this);
                }
              else
                {
                  NS_ASSERT (m_state == RX_DATA);
                  // sanity check: if there are multiple RX events, they
                  // should occur at the same time and have the same
                  // duration, otherwise the interference calculation
                  // won't be correct
                  NS_ASSERT ((m_firstRxStart == Simulator::Now ()) 
                  && (m_firstRxDuration == params->duration));
                }
              
              ChangeState (RX_DATA);
              if (params->packetBurst)
                {
                  m_rxPacketBurstList.push_back (params->packetBurst);
                  m_interferenceData->StartRx (params->psd);
                  
                  m_phyRxStartTrace (params->packetBurst);
                }
                NS_LOG_DEBUG (this << " insert msgs " << params->ctrlMsgList.size ());
              m_rxControlMessageList.insert (m_rxControlMessageList.end (), params->ctrlMsgList.begin (), params->ctrlMsgList.end ());
              
              NS_LOG_LOGIC (this << " numSimultaneousRxEvents = " << m_rxPacketBurstList.size ());
            }
          else
            {
              NS_LOG_LOGIC (this << " not in sync with this signal (cellId=" 
              << params->cellId  << ", m_cellId=" << m_cellId << ")");
            }
        }
        break;
        
        default:
          NS_FATAL_ERROR ("unknown state");
          break;
      }
      
   NS_LOG_LOGIC (this << " state: " << m_state);
}



void
NrSpectrumPhy::StartRxDlCtrl (Ptr<NrSpectrumSignalParametersDlCtrlFrame> nrDlCtrlRxParams)
{
  NS_LOG_FUNCTION (this);

  // To check if we're synchronized to this signal, we check
  // for the CellId which is reported in the
  // NrSpectrumSignalParametersDlCtrlFrame
  uint16_t cellId;        
  NS_ASSERT (nrDlCtrlRxParams != 0);
  cellId = nrDlCtrlRxParams->cellId;

  switch (m_state)
    {
    case TX_DATA:
    case TX_DL_CTRL:
    case TX_UL_SRS:
    case RX_DATA:
    case RX_UL_SRS:
      NS_FATAL_ERROR ("unexpected event in state " << m_state);
      break;

    case RX_DL_CTRL:
    case IDLE:

      // common code for the two states
      // check presence of PSS for UE measuerements
      if (nrDlCtrlRxParams->pss == true)
        {
          if (!m_nrPhyRxPssCallback.IsNull ())
              {
                m_nrPhyRxPssCallback (cellId, nrDlCtrlRxParams->psd);
              }
        }   

      // differentiated code for the two states
      switch (m_state)
        {
        case RX_DL_CTRL:
          NS_ASSERT_MSG (m_cellId != cellId, "any other DlCtrl should be from a different cell");
          NS_LOG_LOGIC (this << " ignoring other DlCtrl (cellId=" 
                        << cellId  << ", m_cellId=" << m_cellId << ")");      
          break;
          
        case IDLE:
          if (cellId  == m_cellId)
            {
              NS_LOG_LOGIC (this << " synchronized with this signal (cellId=" << cellId << ")");
              
              NS_ASSERT (m_rxControlMessageList.empty ());
              m_firstRxStart = Simulator::Now ();
              m_firstRxDuration = nrDlCtrlRxParams->duration;
              NS_LOG_LOGIC (this << " scheduling EndRx with delay " << nrDlCtrlRxParams->duration);
              
              // store the DCIs
              m_rxControlMessageList = nrDlCtrlRxParams->ctrlMsgList;
              m_endRxDlCtrlEvent = Simulator::Schedule (nrDlCtrlRxParams->duration, &NrSpectrumPhy::EndRxDlCtrl, this);
              ChangeState (RX_DL_CTRL);
              m_interferenceCtrl->StartRx (nrDlCtrlRxParams->psd);            
            }
          else
            {
              NS_LOG_LOGIC (this << " not synchronizing with this signal (cellId=" 
                            << cellId  << ", m_cellId=" << m_cellId << ")");          
            }
          break;
          
        default:
          NS_FATAL_ERROR ("unexpected event in state " << m_state);
          break;
        }
      break; // case RX_DL_CTRL or IDLE
      
    default:
      NS_FATAL_ERROR ("unknown state");
      break;
    }
  
  NS_LOG_LOGIC (this << " state: " << m_state);
}




void
NrSpectrumPhy::StartRxUlSrs (Ptr<NrSpectrumSignalParametersUlSrsFrame> nrUlSrsRxParams)
{
  NS_LOG_FUNCTION (this);
  switch (m_state)
    {
    case TX_DATA:
    case TX_DL_CTRL:
    case TX_UL_SRS:
      NS_FATAL_ERROR ("cannot RX while TX: according to FDD channel access, the physical layer for transmission cannot be used for reception");
      break;

    case RX_DATA:
    case RX_DL_CTRL:
      NS_FATAL_ERROR ("cannot RX SRS while receiving something else");
      break;

    case IDLE:
    case RX_UL_SRS:
      // the behavior is similar when
      // we're IDLE or RX_UL_SRS because we can receive more signals
      // simultaneously at the eNB
      {
        // To check if we're synchronized to this signal, we check
        // for the CellId which is reported in the
        // NrSpectrumSignalParametersDlCtrlFrame
        uint16_t cellId;
        cellId = nrUlSrsRxParams->cellId;
        if (cellId  == m_cellId)
          {
            NS_LOG_LOGIC (this << " synchronized with this signal (cellId=" << cellId << ")");
            if (m_state == IDLE)
              {
                // first transmission, i.e., we're IDLE and we
                // start RX
                NS_ASSERT (m_rxControlMessageList.empty ());
                m_firstRxStart = Simulator::Now ();
                m_firstRxDuration = nrUlSrsRxParams->duration;
                NS_LOG_LOGIC (this << " scheduling EndRx with delay " << nrUlSrsRxParams->duration);

                m_endRxUlSrsEvent = Simulator::Schedule (nrUlSrsRxParams->duration, &NrSpectrumPhy::EndRxUlSrs, this);
              }
            else if (m_state == RX_UL_SRS)
              {
                // sanity check: if there are multiple RX events, they
                // should occur at the same time and have the same
                // duration, otherwise the interference calculation
                // won't be correct
                NS_ASSERT ((m_firstRxStart == Simulator::Now ()) 
                           && (m_firstRxDuration == nrUlSrsRxParams->duration));
              }            
            ChangeState (RX_UL_SRS);
            m_interferenceCtrl->StartRx (nrUlSrsRxParams->psd);          
          }
        else
          {
            NS_LOG_LOGIC (this << " not in sync with this signal (cellId=" 
                          << cellId  << ", m_cellId=" << m_cellId << ")");          
          }
      }
      break;
      
    default:
      NS_FATAL_ERROR ("unknown state");
      break;
    }
  
  NS_LOG_LOGIC (this << " state: " << m_state);
}


void
NrSpectrumPhy::UpdateSinrPerceived (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this << sinr);
  m_sinrPerceived = sinr;
}


void
NrSpectrumPhy::AddExpectedTb (uint16_t  rnti, uint8_t ndi, uint16_t size, uint8_t mcs, std::vector<int> map, uint8_t layer, uint8_t harqId,uint8_t rv,  bool downlink)
{
  NS_LOG_FUNCTION (this << " rnti: " << rnti << " NDI " << (uint16_t)ndi << " size " << size << " mcs " << (uint16_t)mcs << " layer " << (uint16_t)layer << " rv " << (uint16_t)rv);
  TbId_t tbId;
  tbId.m_rnti = rnti;
  tbId.m_layer = layer;
  expectedTbs_t::iterator it;
  it = m_expectedTbs.find (tbId);
  if (it != m_expectedTbs.end ())
    {
      // migth be a TB of an unreceived packet (due to high progpalosses)
      m_expectedTbs.erase (it);
    }
  // insert new entry
  tbInfo_t tbInfo = {ndi, size, mcs, map, harqId, rv, 0.0, downlink, false, false};
  m_expectedTbs.insert (std::pair<TbId_t, tbInfo_t> (tbId,tbInfo));
}


void
NrSpectrumPhy::EndRxData ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);

  NS_ASSERT (m_state == RX_DATA);

  // this will trigger CQI calculation and Error Model evaluation
  // as a side effect, the error model should update the error status of all TBs
  m_interferenceData->EndRx ();
  NS_LOG_DEBUG (this << " No. of burts " << m_rxPacketBurstList.size ());
  NS_LOG_DEBUG (this << " Expected TBs " << m_expectedTbs.size ());
  expectedTbs_t::iterator itTb = m_expectedTbs.begin ();
  
  // apply transmission mode gain
  NS_LOG_DEBUG (this << " txMode " << (uint16_t)m_transmissionMode << " gain " << m_txModeGain.at (m_transmissionMode));
  NS_ASSERT (m_transmissionMode < m_txModeGain.size ());
  m_sinrPerceived *= m_txModeGain.at (m_transmissionMode);
  
  while (itTb!=m_expectedTbs.end ())
    {
      if ((m_dataErrorModelEnabled)&&(m_rxPacketBurstList.size ()>0)) // avoid to check for errors when there is no actual data transmitted
        {
          // retrieve HARQ info
          HarqProcessInfoList_t harqInfoList;
          if ((*itTb).second.ndi == 0)
            {
              // TB retxed: retrieve HARQ history
              uint16_t ulHarqId = 0;
              if ((*itTb).second.downlink)
                {
                  harqInfoList = m_harqPhyModule->GetHarqProcessInfoDl ((*itTb).second.harqProcessId, (*itTb).first.m_layer);
                }
              else
                {
                  harqInfoList = m_harqPhyModule->GetHarqProcessInfoUl ((*itTb).first.m_rnti, ulHarqId);
                }
            }
          TbStats_t tbStats = NrMiErrorModel::GetTbDecodificationStats (m_sinrPerceived, (*itTb).second.rbBitmap, (*itTb).second.size, (*itTb).second.mcs, harqInfoList);
          (*itTb).second.mi = tbStats.mi;
          (*itTb).second.corrupt = m_random->GetValue () > tbStats.tbler ? false : true;
          NS_LOG_DEBUG (this << "RNTI " << (*itTb).first.m_rnti << " size " << (*itTb).second.size << " mcs " << (uint32_t)(*itTb).second.mcs << " bitmap " << (*itTb).second.rbBitmap.size () << " layer " << (uint16_t)(*itTb).first.m_layer << " TBLER " << tbStats.tbler << " corrupted " << (*itTb).second.corrupt);
          // fire traces on DL/UL reception PHY stats
          NrPhyReceptionStatParameters params;
          params.m_timestamp = Simulator::Now ().GetMilliSeconds ();
          params.m_cellId = m_cellId;
          params.m_imsi = 0; // it will be set by DlPhyTransmissionCallback in NrHelper
          params.m_rnti = (*itTb).first.m_rnti;
          params.m_txMode = m_transmissionMode;
          params.m_layer =  (*itTb).first.m_layer;
          params.m_mcs = (*itTb).second.mcs;
          params.m_size = (*itTb).second.size;
          params.m_rv = (*itTb).second.rv;
          params.m_ndi = (*itTb).second.ndi;
          params.m_correctness = (uint8_t)!(*itTb).second.corrupt;
          if ((*itTb).second.downlink)
            {
              // DL
              m_dlPhyReception (params);
            }
          else
            {
              // UL
              params.m_rv = harqInfoList.size ();
              m_ulPhyReception (params);
            }
       }
      
      itTb++;
    }
    std::map <uint16_t, NrDlInfoListElement_s> harqDlInfoMap;
    for (std::list<Ptr<PacketBurst> >::const_iterator i = m_rxPacketBurstList.begin (); 
    i != m_rxPacketBurstList.end (); ++i)
      {
        for (std::list<Ptr<Packet> >::const_iterator j = (*i)->Begin (); j != (*i)->End (); ++j)
          {
            // retrieve TB info of this packet 
            NrRadioBearerTag tag;
            (*j)->PeekPacketTag (tag);
            TbId_t tbId;
            tbId.m_rnti = tag.GetRnti ();
            tbId.m_layer = tag.GetLayer ();
            itTb = m_expectedTbs.find (tbId);
            NS_LOG_INFO (this << " Packet of " << tbId.m_rnti << " layer " <<  (uint16_t) tag.GetLayer ());
            if (itTb!=m_expectedTbs.end ())
              {
                if (!(*itTb).second.corrupt)
                  {
                    m_phyRxEndOkTrace (*j);
                
                    if (!m_nrPhyRxDataEndOkCallback.IsNull ())
                      {
                        m_nrPhyRxDataEndOkCallback (*j);
                      }
                  }
                else
                  {
                    // TB received with errors
                    m_phyRxEndErrorTrace (*j);
                  }

                // send HARQ feedback (if not already done for this TB)
                if (!(*itTb).second.harqFeedbackSent)
                  {
                    (*itTb).second.harqFeedbackSent = true;
                    if (!(*itTb).second.downlink)
                      {
                        NrUlInfoListElement_s harqUlInfo;
                        harqUlInfo.m_rnti = tbId.m_rnti;
                        harqUlInfo.m_tpc = 0;
                        if ((*itTb).second.corrupt)
                          {
                            harqUlInfo.m_receptionStatus = NrUlInfoListElement_s::NotOk;
                            NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " send UL-HARQ-NACK");
                            m_harqPhyModule->UpdateUlHarqProcessStatus (tbId.m_rnti, (*itTb).second.mi, (*itTb).second.size, (*itTb).second.size / EffectiveCodingRate [(*itTb).second.mcs]);
                          }
                        else
                          {
                            harqUlInfo.m_receptionStatus = NrUlInfoListElement_s::Ok;
                            NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " send UL-HARQ-ACK");
                            m_harqPhyModule->ResetUlHarqProcessStatus (tbId.m_rnti, (*itTb).second.harqProcessId);
                          }
                          if (!m_nrPhyUlHarqFeedbackCallback.IsNull ())
                            {
                              m_nrPhyUlHarqFeedbackCallback (harqUlInfo);
                            }
                      }
                    else
                      {
                        std::map <uint16_t, NrDlInfoListElement_s>::iterator itHarq = harqDlInfoMap.find (tbId.m_rnti);
                        if (itHarq==harqDlInfoMap.end ())
                          {
                            NrDlInfoListElement_s harqDlInfo;
                            harqDlInfo.m_harqStatus.resize (m_layersNum, NrDlInfoListElement_s::NACK);
                            harqDlInfo.m_rnti = tbId.m_rnti;
                            harqDlInfo.m_harqProcessId = (*itTb).second.harqProcessId;
                            if ((*itTb).second.corrupt)
                              {
                                harqDlInfo.m_harqStatus.at (tbId.m_layer) = NrDlInfoListElement_s::NACK;
                                NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " harqId " << (uint16_t)(*itTb).second.harqProcessId << " layer " <<(uint16_t)tbId.m_layer << " send DL-HARQ-NACK");
                                m_harqPhyModule->UpdateDlHarqProcessStatus ((*itTb).second.harqProcessId, tbId.m_layer, (*itTb).second.mi, (*itTb).second.size, (*itTb).second.size / EffectiveCodingRate [(*itTb).second.mcs]);
                              }
                            else
                              {

                                harqDlInfo.m_harqStatus.at (tbId.m_layer) = NrDlInfoListElement_s::ACK;
                                NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " harqId " << (uint16_t)(*itTb).second.harqProcessId << " layer " <<(uint16_t)tbId.m_layer << " size " << (*itTb).second.size << " send DL-HARQ-ACK");
                                m_harqPhyModule->ResetDlHarqProcessStatus ((*itTb).second.harqProcessId);
                              }
                            harqDlInfoMap.insert (std::pair <uint16_t, NrDlInfoListElement_s> (tbId.m_rnti, harqDlInfo));
                          }
                        else
                        {
                          if ((*itTb).second.corrupt)
                            {
                              (*itHarq).second.m_harqStatus.at (tbId.m_layer) = NrDlInfoListElement_s::NACK;
                              NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " harqId " << (uint16_t)(*itTb).second.harqProcessId << " layer " <<(uint16_t)tbId.m_layer << " size " << (*itHarq).second.m_harqStatus.size () << " send DL-HARQ-NACK");
                              m_harqPhyModule->UpdateDlHarqProcessStatus ((*itTb).second.harqProcessId, tbId.m_layer, (*itTb).second.mi, (*itTb).second.size, (*itTb).second.size / EffectiveCodingRate [(*itTb).second.mcs]);
                            }
                          else
                            {
                              NS_ASSERT_MSG (tbId.m_layer < (*itHarq).second.m_harqStatus.size (), " layer " << (uint16_t)tbId.m_layer);
                              (*itHarq).second.m_harqStatus.at (tbId.m_layer) = NrDlInfoListElement_s::ACK;
                              NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " harqId " << (uint16_t)(*itTb).second.harqProcessId << " layer " << (uint16_t)tbId.m_layer << " size " << (*itHarq).second.m_harqStatus.size () << " send DL-HARQ-ACK");
                              m_harqPhyModule->ResetDlHarqProcessStatus ((*itTb).second.harqProcessId);
                            }
                        }
                      } // end if ((*itTb).second.downlink) HARQ
                  } // end if (!(*itTb).second.harqFeedbackSent)
              }
          }
      }

  // send DL HARQ feedback to NrPhy
  std::map <uint16_t, NrDlInfoListElement_s>::iterator itHarq;
  for (itHarq = harqDlInfoMap.begin (); itHarq != harqDlInfoMap.end (); itHarq++)
    {
      if (!m_nrPhyDlHarqFeedbackCallback.IsNull ())
        {
          m_nrPhyDlHarqFeedbackCallback ((*itHarq).second);
        }
    }
  // forward control messages of this frame to NrPhy
  if (!m_rxControlMessageList.empty ())
    {
      if (!m_nrPhyRxCtrlEndOkCallback.IsNull ())
        {
          m_nrPhyRxCtrlEndOkCallback (m_rxControlMessageList);
        }
    }
  ChangeState (IDLE);
  m_rxPacketBurstList.clear ();
  m_rxControlMessageList.clear ();
  m_expectedTbs.clear ();
}


void
NrSpectrumPhy::EndRxDlCtrl ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);
  
  NS_ASSERT (m_state == RX_DL_CTRL);
  
  // this will trigger CQI calculation and Error Model evaluation
  // as a side effect, the error model should update the error status of all TBs
  m_interferenceCtrl->EndRx ();
  // apply transmission mode gain
  NS_LOG_DEBUG (this << " txMode " << (uint16_t)m_transmissionMode << " gain " << m_txModeGain.at (m_transmissionMode));
  NS_ASSERT (m_transmissionMode < m_txModeGain.size ());
  if (m_transmissionMode>0)
    {
      // in case of MIMO, ctrl is always txed as TX diversity
      m_sinrPerceived *= m_txModeGain.at (1);
    }
//   m_sinrPerceived *= m_txModeGain.at (m_transmissionMode);
  bool error = false;
  if (m_ctrlErrorModelEnabled)
    {
      double  errorRate = NrMiErrorModel::GetPcfichPdcchError (m_sinrPerceived);
      error = m_random->GetValue () > errorRate ? false : true;
      NS_LOG_DEBUG (this << " PCFICH-PDCCH Decodification, errorRate " << errorRate << " error " << error);
    }

  if (!error)
    {
      if (!m_nrPhyRxCtrlEndOkCallback.IsNull ())
        {
          NS_LOG_DEBUG (this << " PCFICH-PDCCH Rxed OK");
          m_nrPhyRxCtrlEndOkCallback (m_rxControlMessageList);
        }
    }
  else
    {
      if (!m_nrPhyRxCtrlEndErrorCallback.IsNull ())
        {
          NS_LOG_DEBUG (this << " PCFICH-PDCCH Error");
          m_nrPhyRxCtrlEndErrorCallback ();
        }
    }
  ChangeState (IDLE);
  m_rxControlMessageList.clear ();
}

void
NrSpectrumPhy::EndRxUlSrs ()
{
  NS_ASSERT (m_state == RX_UL_SRS);
  ChangeState (IDLE);
  m_interferenceCtrl->EndRx ();
  // nothing to do (used only for SRS at this stage)
}

void 
NrSpectrumPhy::SetCellId (uint16_t cellId)
{
  m_cellId = cellId;
}


void
NrSpectrumPhy::AddRsPowerChunkProcessor (Ptr<NrChunkProcessor> p)
{
  m_interferenceCtrl->AddRsPowerChunkProcessor (p);
}

void
NrSpectrumPhy::AddDataPowerChunkProcessor (Ptr<NrChunkProcessor> p)
{
  m_interferenceData->AddRsPowerChunkProcessor (p);
}

void
NrSpectrumPhy::AddDataSinrChunkProcessor (Ptr<NrChunkProcessor> p)
{
  m_interferenceData->AddSinrChunkProcessor (p);
}

void
NrSpectrumPhy::AddInterferenceCtrlChunkProcessor (Ptr<NrChunkProcessor> p)
{
  m_interferenceCtrl->AddInterferenceChunkProcessor (p);
}

void
NrSpectrumPhy::AddInterferenceDataChunkProcessor (Ptr<NrChunkProcessor> p)
{
  m_interferenceData->AddInterferenceChunkProcessor (p);
}

void
NrSpectrumPhy::AddCtrlSinrChunkProcessor (Ptr<NrChunkProcessor> p)
{
  m_interferenceCtrl->AddSinrChunkProcessor (p);
}

void 
NrSpectrumPhy::SetTransmissionMode (uint8_t txMode)
{
  NS_LOG_FUNCTION (this << (uint16_t) txMode);
  NS_ASSERT_MSG (txMode < m_txModeGain.size (), "TransmissionMode not available: 1.." << m_txModeGain.size ());
  m_transmissionMode = txMode;
  m_layersNum = NrTransmissionModesLayers::TxMode2LayerNum (txMode);
}


void 
NrSpectrumPhy::SetTxModeGain (uint8_t txMode, double gain)
{
  NS_LOG_FUNCTION (this << " txmode " << (uint16_t)txMode << " gain " << gain);
  // convert to linear
  gain = std::pow (10.0, (gain / 10.0));
  if (m_txModeGain.size () < txMode)
  {
    m_txModeGain.resize (txMode);
  }
  std::vector <double> temp;
  temp = m_txModeGain;
  m_txModeGain.clear ();
  for (uint8_t i = 0; i < temp.size (); i++)
  {
    if (i==txMode-1)
    {
      m_txModeGain.push_back (gain);
    }
    else
    {
      m_txModeGain.push_back (temp.at (i));
    }
  }
}

int64_t
NrSpectrumPhy::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_random->SetStream (stream);
  return 1;
}



} // namespace ns3
