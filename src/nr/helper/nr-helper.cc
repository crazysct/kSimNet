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
 * Author: Nicola Baldo <nbaldo@cttc.es> (re-wrote from scratch this helper)
 *         Giuseppe Piro <g.piro@poliba.it> (parts of the PHY & channel  creation & configuration copied from the GSoC 2011 code)
 */


#include "nr-helper.h"
#include <ns3/string.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/pointer.h>
#include <ns3/nr-enb-rrc.h>
#include <ns3/ngc-ue-nas.h>
#include <ns3/ngc-enb-application.h>
#include <ns3/nr-ue-rrc.h>
#include <ns3/nr-ue-mac.h>
#include <ns3/nr-enb-mac.h>
#include <ns3/nr-enb-net-device.h>
#include <ns3/nr-enb-phy.h>
#include <ns3/nr-ue-phy.h>
#include <ns3/nr-spectrum-phy.h>
#include <ns3/nr-chunk-processor.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/friis-spectrum-propagation-loss.h>
#include <ns3/nr-trace-fading-loss-model.h>
#include <ns3/isotropic-antenna-model.h>
#include <ns3/nr-enb-net-device.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/nr-ff-mac-scheduler.h>
#include <ns3/nr-ffr-algorithm.h>
#include <ns3/nr-handover-algorithm.h>
#include <ns3/nr-anr.h>
#include <ns3/nr-rlc.h>
#include <ns3/nr-rlc-um.h>
#include <ns3/nr-rlc-am.h>
#include <ns3/ngc-enb-n2-sap.h>
#include <ns3/nr-rrc-protocol-ideal.h>
#include <ns3/nr-rrc-protocol-real.h>
#include <ns3/nr-mac-stats-calculator.h>
#include <ns3/nr-phy-stats-calculator.h>
#include <ns3/nr-phy-tx-stats-calculator.h>
#include <ns3/nr-phy-rx-stats-calculator.h>
#include <ns3/ngc-helper.h>
#include <iostream>
#include <ns3/buildings-propagation-loss-model.h>
#include <ns3/nr-spectrum-value-helper.h>
#include <ns3/ngc-x2.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrHelper");

NS_OBJECT_ENSURE_REGISTERED (NrHelper);

NrHelper::NrHelper (void)
  : m_fadingStreamsAssigned (false),
    m_imsiCounter (0),
    m_cellIdCounter (0)
{
  NS_LOG_FUNCTION (this);
  m_enbNetDeviceFactory.SetTypeId (NrEnbNetDevice::GetTypeId ());
  m_enbAntennaModelFactory.SetTypeId (IsotropicAntennaModel::GetTypeId ());
  m_ueNetDeviceFactory.SetTypeId (NrUeNetDevice::GetTypeId ());
  m_ueAntennaModelFactory.SetTypeId (IsotropicAntennaModel::GetTypeId ());
  m_channelFactory.SetTypeId (MultiModelSpectrumChannel::GetTypeId ());
}

void 
NrHelper::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_downlinkChannel = m_channelFactory.Create<SpectrumChannel> ();
  m_uplinkChannel = m_channelFactory.Create<SpectrumChannel> ();

  m_downlinkPathlossModel = m_dlPathlossModelFactory.Create ();
  Ptr<SpectrumPropagationLossModel> dlSplm = m_downlinkPathlossModel->GetObject<SpectrumPropagationLossModel> ();
  if (dlSplm != 0)
    {
      NS_LOG_LOGIC (this << " using a SpectrumPropagationLossModel in DL");
      m_downlinkChannel->AddSpectrumPropagationLossModel (dlSplm);
    }
  else
    {
      NS_LOG_LOGIC (this << " using a PropagationLossModel in DL");
      Ptr<PropagationLossModel> dlPlm = m_downlinkPathlossModel->GetObject<PropagationLossModel> ();
      NS_ASSERT_MSG (dlPlm != 0, " " << m_downlinkPathlossModel << " is neither PropagationLossModel nor SpectrumPropagationLossModel");
      m_downlinkChannel->AddPropagationLossModel (dlPlm);
    }

  m_uplinkPathlossModel = m_ulPathlossModelFactory.Create ();
  Ptr<SpectrumPropagationLossModel> ulSplm = m_uplinkPathlossModel->GetObject<SpectrumPropagationLossModel> ();
  if (ulSplm != 0)
    {
      NS_LOG_LOGIC (this << " using a SpectrumPropagationLossModel in UL");
      m_uplinkChannel->AddSpectrumPropagationLossModel (ulSplm);
    }
  else
    {
      NS_LOG_LOGIC (this << " using a PropagationLossModel in UL");
      Ptr<PropagationLossModel> ulPlm = m_uplinkPathlossModel->GetObject<PropagationLossModel> ();
      NS_ASSERT_MSG (ulPlm != 0, " " << m_uplinkPathlossModel << " is neither PropagationLossModel nor SpectrumPropagationLossModel");
      m_uplinkChannel->AddPropagationLossModel (ulPlm);
    }
  if (!m_fadingModelType.empty ())
    {
      m_fadingModule = m_fadingModelFactory.Create<SpectrumPropagationLossModel> ();
      m_fadingModule->Initialize ();
      m_downlinkChannel->AddSpectrumPropagationLossModel (m_fadingModule);
      m_uplinkChannel->AddSpectrumPropagationLossModel (m_fadingModule);
    }
  m_phyStats = CreateObject<NrPhyStatsCalculator> ();
  m_phyTxStats = CreateObject<NrPhyTxStatsCalculator> ();
  m_phyRxStats = CreateObject<NrPhyRxStatsCalculator> ();
  m_macStats = CreateObject<NrMacStatsCalculator> ();
  Object::DoInitialize ();

}

NrHelper::~NrHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId NrHelper::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::NrHelper")
    .SetParent<Object> ()
    .AddConstructor<NrHelper> ()
    .AddAttribute ("Scheduler",
                   "The type of scheduler to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::NrFfMacScheduler.",
                   StringValue ("ns3::NrPfFfMacScheduler"),
                   MakeStringAccessor (&NrHelper::SetSchedulerType,
                                       &NrHelper::GetSchedulerType),
                   MakeStringChecker ())
    .AddAttribute ("FfrAlgorithm",
                   "The type of FFR algorithm to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::NrFfrAlgorithm.",
                   StringValue ("ns3::NrFrNoOpAlgorithm"),
                   MakeStringAccessor (&NrHelper::SetFfrAlgorithmType,
                                       &NrHelper::GetFfrAlgorithmType),
                   MakeStringChecker ())
    .AddAttribute ("HandoverAlgorithm",
                   "The type of handover algorithm to be used for eNBs. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::NrHandoverAlgorithm.",
                   StringValue ("ns3::NrNoOpHandoverAlgorithm"),
                   MakeStringAccessor (&NrHelper::SetHandoverAlgorithmType,
                                       &NrHelper::GetHandoverAlgorithmType),
                   MakeStringChecker ())
    .AddAttribute ("PathlossModel",
                   "The type of pathloss model to be used. "
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::PropagationLossModel.",
                   StringValue ("ns3::FriisPropagationLossModel"),
                   MakeStringAccessor (&NrHelper::SetPathlossModelType),
                   MakeStringChecker ())
    .AddAttribute ("FadingModel",
                   "The type of fading model to be used."
                   "The allowed values for this attributes are the type names "
                   "of any class inheriting from ns3::SpectrumPropagationLossModel."
                   "If the type is set to an empty string, no fading model is used.",
                   StringValue (""),
                   MakeStringAccessor (&NrHelper::SetFadingModel),
                   MakeStringChecker ())
    .AddAttribute ("UseIdealRrc",
                   "If true, NrRrcProtocolIdeal will be used for RRC signaling. "
                   "If false, NrRrcProtocolReal will be used.",
                   BooleanValue (true), 
                   MakeBooleanAccessor (&NrHelper::m_useIdealRrc),
                   MakeBooleanChecker ())
    .AddAttribute ("AnrEnabled",
                   "Activate or deactivate Automatic Neighbour Relation function",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrHelper::m_isAnrEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("UsePdschForCqiGeneration",
                   "If true, DL-CQI will be calculated from PDCCH as signal and PDSCH as interference "
                   "If false, DL-CQI will be calculated from PDCCH as signal and PDCCH as interference  ",
                   BooleanValue (true),
                   MakeBooleanAccessor (&NrHelper::m_usePdschForCqiGeneration),
                   MakeBooleanChecker ())
  ;
  return tid;
}

void
NrHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_downlinkChannel = 0;
  m_uplinkChannel = 0;
  Object::DoDispose ();
}


void 
NrHelper::SetNgcHelper (Ptr<NgcHelper> h)
{
  NS_LOG_FUNCTION (this << h);
  m_ngcHelper = h;
}

void 
NrHelper::SetSchedulerType (std::string type) 
{
  NS_LOG_FUNCTION (this << type);
  m_schedulerFactory = ObjectFactory ();
  m_schedulerFactory.SetTypeId (type);
}

std::string
NrHelper::GetSchedulerType () const
{
  return m_schedulerFactory.GetTypeId ().GetName ();
} 

void 
NrHelper::SetSchedulerAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_schedulerFactory.Set (n, v);
}

std::string
NrHelper::GetFfrAlgorithmType () const
{
  return m_ffrAlgorithmFactory.GetTypeId ().GetName ();
}

void
NrHelper::SetFfrAlgorithmType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_ffrAlgorithmFactory = ObjectFactory ();
  m_ffrAlgorithmFactory.SetTypeId (type);
}

void
NrHelper::SetFfrAlgorithmAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_ffrAlgorithmFactory.Set (n, v);
}

std::string
NrHelper::GetHandoverAlgorithmType () const
{
  return m_handoverAlgorithmFactory.GetTypeId ().GetName ();
}

void
NrHelper::SetHandoverAlgorithmType (std::string type)
{
  NS_LOG_FUNCTION (this << type);
  m_handoverAlgorithmFactory = ObjectFactory ();
  m_handoverAlgorithmFactory.SetTypeId (type);
}

void
NrHelper::SetHandoverAlgorithmAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_handoverAlgorithmFactory.Set (n, v);
}


void 
NrHelper::SetPathlossModelType (std::string type) 
{
  NS_LOG_FUNCTION (this << type);
  m_dlPathlossModelFactory = ObjectFactory ();
  m_dlPathlossModelFactory.SetTypeId (type);
  m_ulPathlossModelFactory = ObjectFactory ();
  m_ulPathlossModelFactory.SetTypeId (type);
}

void 
NrHelper::SetPathlossModelAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this << n);
  m_dlPathlossModelFactory.Set (n, v);
  m_ulPathlossModelFactory.Set (n, v);
}

void
NrHelper::SetEnbDeviceAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_enbNetDeviceFactory.Set (n, v);
}


void 
NrHelper::SetEnbAntennaModelType (std::string type)
{
  NS_LOG_FUNCTION (this);
  m_enbAntennaModelFactory.SetTypeId (type);
}

void 
NrHelper::SetEnbAntennaModelAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_enbAntennaModelFactory.Set (n, v);
}

void
NrHelper::SetUeDeviceAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueNetDeviceFactory.Set (n, v);
}

void 
NrHelper::SetUeAntennaModelType (std::string type)
{
  NS_LOG_FUNCTION (this);
  m_ueAntennaModelFactory.SetTypeId (type);
}

void 
NrHelper::SetUeAntennaModelAttribute (std::string n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_ueAntennaModelFactory.Set (n, v);
}

void 
NrHelper::SetFadingModel (std::string type) 
{
  NS_LOG_FUNCTION (this << type);
  m_fadingModelType = type;
  if (!type.empty ())
    {
      m_fadingModelFactory = ObjectFactory ();
      m_fadingModelFactory.SetTypeId (type);
    }
}

void 
NrHelper::SetFadingModelAttribute (std::string n, const AttributeValue &v)
{
  m_fadingModelFactory.Set (n, v);
}

void 
NrHelper::SetSpectrumChannelType (std::string type) 
{
  NS_LOG_FUNCTION (this << type);
  m_channelFactory.SetTypeId (type);
}

void 
NrHelper::SetSpectrumChannelAttribute (std::string n, const AttributeValue &v)
{
  m_channelFactory.Set (n, v);
}


NetDeviceContainer
NrHelper::InstallEnbDevice (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  Initialize ();  // will run DoInitialize () if necessary
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleEnbDevice (node);
      devices.Add (device);
    }
  return devices;
}

NetDeviceContainer
NrHelper::InstallUeDevice (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleUeDevice (node);
      devices.Add (device);
    }
  return devices;
}


Ptr<NetDevice>
NrHelper::InstallSingleEnbDevice (Ptr<Node> n)
{

  NS_ABORT_MSG_IF (m_cellIdCounter == 65535, "max num eNBs exceeded");
  uint16_t cellId = ++m_cellIdCounter;

  Ptr<NrSpectrumPhy> dlPhy = CreateObject<NrSpectrumPhy> ();
  Ptr<NrSpectrumPhy> ulPhy = CreateObject<NrSpectrumPhy> ();

  Ptr<NrEnbPhy> phy = CreateObject<NrEnbPhy> (dlPhy, ulPhy);

  Ptr<NrHarqPhy> harq = Create<NrHarqPhy> ();
  dlPhy->SetHarqPhyModule (harq);
  ulPhy->SetHarqPhyModule (harq);
  phy->SetHarqPhyModule (harq);

  Ptr<NrChunkProcessor> pCtrl = Create<NrChunkProcessor> ();
  pCtrl->AddCallback (MakeCallback (&NrEnbPhy::GenerateCtrlCqiReport, phy));
  ulPhy->AddCtrlSinrChunkProcessor (pCtrl); // for evaluating SRS UL-CQI

  Ptr<NrChunkProcessor> pData = Create<NrChunkProcessor> ();
  pData->AddCallback (MakeCallback (&NrEnbPhy::GenerateDataCqiReport, phy));
  pData->AddCallback (MakeCallback (&NrSpectrumPhy::UpdateSinrPerceived, ulPhy));
  ulPhy->AddDataSinrChunkProcessor (pData); // for evaluating PUSCH UL-CQI

  Ptr<NrChunkProcessor> pInterf = Create<NrChunkProcessor> ();
  pInterf->AddCallback (MakeCallback (&NrEnbPhy::ReportInterference, phy));
  ulPhy->AddInterferenceDataChunkProcessor (pInterf); // for interference power tracing

  dlPhy->SetChannel (m_downlinkChannel);
  ulPhy->SetChannel (m_uplinkChannel);

  Ptr<MobilityModel> mm = n->GetObject<MobilityModel> ();
  NS_ASSERT_MSG (mm, "MobilityModel needs to be set on node before calling NrHelper::InstallUeDevice ()");
  dlPhy->SetMobility (mm);
  ulPhy->SetMobility (mm);

  Ptr<AntennaModel> antenna = (m_enbAntennaModelFactory.Create ())->GetObject<AntennaModel> ();
  NS_ASSERT_MSG (antenna, "error in creating the AntennaModel object");
  dlPhy->SetAntenna (antenna);
  ulPhy->SetAntenna (antenna);

  Ptr<NrEnbMac> mac = CreateObject<NrEnbMac> ();
  Ptr<NrFfMacScheduler> sched = m_schedulerFactory.Create<NrFfMacScheduler> ();
  Ptr<NrFfrAlgorithm> ffrAlgorithm = m_ffrAlgorithmFactory.Create<NrFfrAlgorithm> ();
  Ptr<NrHandoverAlgorithm> handoverAlgorithm = m_handoverAlgorithmFactory.Create<NrHandoverAlgorithm> ();
  Ptr<NrEnbRrc> rrc = CreateObject<NrEnbRrc> ();

  if (m_useIdealRrc)
    {
      Ptr<NrEnbRrcProtocolIdeal> rrcProtocol = CreateObject<NrEnbRrcProtocolIdeal> ();
      rrcProtocol->SetNrEnbRrcSapProvider (rrc->GetNrEnbRrcSapProvider ());
      rrc->SetNrEnbRrcSapUser (rrcProtocol->GetNrEnbRrcSapUser ());
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetCellId (cellId);
    }
  else
    {
      Ptr<NrEnbRrcProtocolReal> rrcProtocol = CreateObject<NrEnbRrcProtocolReal> ();
      rrcProtocol->SetNrEnbRrcSapProvider (rrc->GetNrEnbRrcSapProvider ());
      rrc->SetNrEnbRrcSapUser (rrcProtocol->GetNrEnbRrcSapUser ());
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetCellId (cellId);
    }

  if (m_ngcHelper != 0)
    {
      EnumValue epsBearerToRlcMapping;
      rrc->GetAttribute ("EpsBearerToRlcMapping", epsBearerToRlcMapping);
      // it does not make sense to use RLC/SM when also using the NGC
      if (epsBearerToRlcMapping.Get () == NrEnbRrc::RLC_SM_ALWAYS)
        {
          rrc->SetAttribute ("EpsBearerToRlcMapping", EnumValue (NrEnbRrc::RLC_UM_ALWAYS));
        }
    }

  rrc->SetNrEnbCmacSapProvider (mac->GetNrEnbCmacSapProvider ());
  mac->SetNrEnbCmacSapUser (rrc->GetNrEnbCmacSapUser ());
  rrc->SetNrMacSapProvider (mac->GetNrMacSapProvider ());

  rrc->SetNrHandoverManagementSapProvider (handoverAlgorithm->GetNrHandoverManagementSapProvider ());
  handoverAlgorithm->SetNrHandoverManagementSapUser (rrc->GetNrHandoverManagementSapUser ());

  mac->SetNrFfMacSchedSapProvider (sched->GetNrFfMacSchedSapProvider ());
  mac->SetNrFfMacCschedSapProvider (sched->GetNrFfMacCschedSapProvider ());

  sched->SetNrFfMacSchedSapUser (mac->GetNrFfMacSchedSapUser ());
  sched->SetNrFfMacCschedSapUser (mac->GetNrFfMacCschedSapUser ());

  phy->SetNrEnbPhySapUser (mac->GetNrEnbPhySapUser ());
  mac->SetNrEnbPhySapProvider (phy->GetNrEnbPhySapProvider ());

  phy->SetNrEnbCphySapUser (rrc->GetNrEnbCphySapUser ());
  rrc->SetNrEnbCphySapProvider (phy->GetNrEnbCphySapProvider ());

  //FFR SAP
  sched->SetNrFfrSapProvider (ffrAlgorithm->GetNrFfrSapProvider ());
  ffrAlgorithm->SetNrFfrSapUser (sched->GetNrFfrSapUser ());

  rrc->SetNrFfrRrcSapProvider (ffrAlgorithm->GetNrFfrRrcSapProvider ());
  ffrAlgorithm->SetNrFfrRrcSapUser (rrc->GetNrFfrRrcSapUser ());
  //FFR SAP END

  Ptr<NrEnbNetDevice> dev = m_enbNetDeviceFactory.Create<NrEnbNetDevice> ();
  dev->SetNode (n);
  dev->SetAttribute ("CellId", UintegerValue (cellId)); 
  dev->SetAttribute ("NrEnbPhy", PointerValue (phy));
  dev->SetAttribute ("NrEnbMac", PointerValue (mac));
  dev->SetAttribute ("NrFfMacScheduler", PointerValue (sched));
  dev->SetAttribute ("NrEnbRrc", PointerValue (rrc)); 
  dev->SetAttribute ("NrHandoverAlgorithm", PointerValue (handoverAlgorithm));
  dev->SetAttribute ("NrFfrAlgorithm", PointerValue (ffrAlgorithm));

  if (m_isAnrEnabled)
    {
      Ptr<NrAnr> anr = CreateObject<NrAnr> (cellId);
      rrc->SetNrAnrSapProvider (anr->GetNrAnrSapProvider ());
      anr->SetNrAnrSapUser (rrc->GetNrAnrSapUser ());
      dev->SetAttribute ("NrAnr", PointerValue (anr));
    }

  phy->SetDevice (dev);
  dlPhy->SetDevice (dev);
  ulPhy->SetDevice (dev);

  n->AddDevice (dev);
  ulPhy->SetNrPhyRxDataEndOkCallback (MakeCallback (&NrEnbPhy::PhyPduReceived, phy));
  ulPhy->SetNrPhyRxCtrlEndOkCallback (MakeCallback (&NrEnbPhy::ReceiveNrControlMessageList, phy));
  ulPhy->SetNrPhyUlHarqFeedbackCallback (MakeCallback (&NrEnbPhy::ReceiveNrUlHarqFeedback, phy));
  rrc->SetForwardUpCallback (MakeCallback (&NrEnbNetDevice::Receive, dev));

  NS_LOG_LOGIC ("set the propagation model frequencies");
  double dlFreq = NrSpectrumValueHelper::GetCarrierFrequency (dev->GetDlEarfcn ());
  NS_LOG_LOGIC ("DL freq: " << dlFreq);
  bool dlFreqOk = m_downlinkPathlossModel->SetAttributeFailSafe ("Frequency", DoubleValue (dlFreq));
  if (!dlFreqOk)
    {
      NS_LOG_WARN ("DL propagation model does not have a Frequency attribute");
    }
  double ulFreq = NrSpectrumValueHelper::GetCarrierFrequency (dev->GetUlEarfcn ());
  NS_LOG_LOGIC ("UL freq: " << ulFreq);
  bool ulFreqOk = m_uplinkPathlossModel->SetAttributeFailSafe ("Frequency", DoubleValue (ulFreq));
  if (!ulFreqOk)
    {
      NS_LOG_WARN ("UL propagation model does not have a Frequency attribute");
    }

  dev->Initialize ();

  m_uplinkChannel->AddRx (ulPhy);

  if (m_ngcHelper != 0)
    {
      NS_LOG_INFO ("adding this eNB to the NGC");
      m_ngcHelper->AddEnb (n, dev, dev->GetCellId ());
      Ptr<NgcEnbApplication> enbApp = n->GetApplication (0)->GetObject<NgcEnbApplication> ();
      NS_ASSERT_MSG (enbApp != 0, "cannot retrieve NgcEnbApplication");

      // N2 SAPs
      rrc->SetN2SapProvider (enbApp->GetN2SapProvider ());
      enbApp->SetN2SapUser (rrc->GetN2SapUser ());

      // X2 SAPs
      Ptr<NgcX2> x2 = n->GetObject<NgcX2> ();
      x2->SetNgcX2SapUser (rrc->GetNgcX2SapUser ());
      rrc->SetNgcX2SapProvider (x2->GetNgcX2SapProvider ());
    }

  return dev;
}

Ptr<NetDevice>
NrHelper::InstallSingleUeDevice (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this);
  Ptr<NrSpectrumPhy> dlPhy = CreateObject<NrSpectrumPhy> ();
  Ptr<NrSpectrumPhy> ulPhy = CreateObject<NrSpectrumPhy> ();

  Ptr<NrUePhy> phy = CreateObject<NrUePhy> (dlPhy, ulPhy);

  Ptr<NrHarqPhy> harq = Create<NrHarqPhy> ();
  dlPhy->SetHarqPhyModule (harq);
  ulPhy->SetHarqPhyModule (harq);
  phy->SetHarqPhyModule (harq);

  Ptr<NrChunkProcessor> pRs = Create<NrChunkProcessor> ();
  pRs->AddCallback (MakeCallback (&NrUePhy::ReportRsReceivedPower, phy));
  dlPhy->AddRsPowerChunkProcessor (pRs);

  Ptr<NrChunkProcessor> pInterf = Create<NrChunkProcessor> ();
  pInterf->AddCallback (MakeCallback (&NrUePhy::ReportInterference, phy));
  dlPhy->AddInterferenceCtrlChunkProcessor (pInterf); // for RSRQ evaluation of UE Measurements

  Ptr<NrChunkProcessor> pCtrl = Create<NrChunkProcessor> ();
  pCtrl->AddCallback (MakeCallback (&NrSpectrumPhy::UpdateSinrPerceived, dlPhy));
  dlPhy->AddCtrlSinrChunkProcessor (pCtrl);

  Ptr<NrChunkProcessor> pData = Create<NrChunkProcessor> ();
  pData->AddCallback (MakeCallback (&NrSpectrumPhy::UpdateSinrPerceived, dlPhy));
  dlPhy->AddDataSinrChunkProcessor (pData);

  if (m_usePdschForCqiGeneration)
    {
      // CQI calculation based on PDCCH for signal and PDSCH for interference
      pCtrl->AddCallback (MakeCallback (&NrUePhy::GenerateMixedCqiReport, phy));
      Ptr<NrChunkProcessor> pDataInterf = Create<NrChunkProcessor> ();      
      pDataInterf->AddCallback (MakeCallback (&NrUePhy::ReportDataInterference, phy));
      dlPhy->AddInterferenceDataChunkProcessor (pDataInterf);
    }
  else
    {
      // CQI calculation based on PDCCH for both signal and interference
      pCtrl->AddCallback (MakeCallback (&NrUePhy::GenerateCtrlCqiReport, phy));
    }



  dlPhy->SetChannel (m_downlinkChannel);
  ulPhy->SetChannel (m_uplinkChannel);

  Ptr<MobilityModel> mm = n->GetObject<MobilityModel> ();
  NS_ASSERT_MSG (mm, "MobilityModel needs to be set on node before calling NrHelper::InstallUeDevice ()");
  dlPhy->SetMobility (mm);
  ulPhy->SetMobility (mm);

  Ptr<AntennaModel> antenna = (m_ueAntennaModelFactory.Create ())->GetObject<AntennaModel> ();
  NS_ASSERT_MSG (antenna, "error in creating the AntennaModel object");
  dlPhy->SetAntenna (antenna);
  ulPhy->SetAntenna (antenna);

  Ptr<NrUeMac> mac = CreateObject<NrUeMac> ();
  Ptr<NrUeRrc> rrc = CreateObject<NrUeRrc> ();

  if (m_useIdealRrc)
    {
      Ptr<NrUeRrcProtocolIdeal> rrcProtocol = CreateObject<NrUeRrcProtocolIdeal> ();
      rrcProtocol->SetUeRrc (rrc);
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetNrUeRrcSapProvider (rrc->GetNrUeRrcSapProvider ());
      rrc->SetNrUeRrcSapUser (rrcProtocol->GetNrUeRrcSapUser ());
    }
  else
    {
      Ptr<NrUeRrcProtocolReal> rrcProtocol = CreateObject<NrUeRrcProtocolReal> ();
      rrcProtocol->SetUeRrc (rrc);
      rrc->AggregateObject (rrcProtocol);
      rrcProtocol->SetNrUeRrcSapProvider (rrc->GetNrUeRrcSapProvider ());
      rrc->SetNrUeRrcSapUser (rrcProtocol->GetNrUeRrcSapUser ());
    }

  if (m_ngcHelper != 0)
    {
      rrc->SetUseRlcSm (false);
    }
  Ptr<NgcUeNas> nas = CreateObject<NgcUeNas> ();
 
  nas->SetAsSapProvider (rrc->GetAsSapProvider ());
  rrc->SetAsSapUser (nas->GetAsSapUser ());

  rrc->SetNrUeCmacSapProvider (mac->GetNrUeCmacSapProvider ());
  mac->SetNrUeCmacSapUser (rrc->GetNrUeCmacSapUser ());
  rrc->SetNrMacSapProvider (mac->GetNrMacSapProvider ());

  phy->SetNrUePhySapUser (mac->GetNrUePhySapUser ());
  mac->SetNrUePhySapProvider (phy->GetNrUePhySapProvider ());

  phy->SetNrUeCphySapUser (rrc->GetNrUeCphySapUser ());
  rrc->SetNrUeCphySapProvider (phy->GetNrUeCphySapProvider ());

  NS_ABORT_MSG_IF (m_imsiCounter >= 0xFFFFFFFF, "max num UEs exceeded");
  uint64_t imsi = ++m_imsiCounter;

  Ptr<NrUeNetDevice> dev = m_ueNetDeviceFactory.Create<NrUeNetDevice> ();
  dev->SetNode (n);
  dev->SetAttribute ("Imsi", UintegerValue (imsi));
  dev->SetAttribute ("NrUePhy", PointerValue (phy));
  dev->SetAttribute ("NrUeMac", PointerValue (mac));
  dev->SetAttribute ("NrUeRrc", PointerValue (rrc));
  dev->SetAttribute ("NgcUeNas", PointerValue (nas));

  phy->SetDevice (dev);
  dlPhy->SetDevice (dev);
  ulPhy->SetDevice (dev);
  nas->SetDevice (dev);

  n->AddDevice (dev);
  dlPhy->SetNrPhyRxDataEndOkCallback (MakeCallback (&NrUePhy::PhyPduReceived, phy));
  dlPhy->SetNrPhyRxCtrlEndOkCallback (MakeCallback (&NrUePhy::ReceiveNrControlMessageList, phy));
  dlPhy->SetNrPhyRxPssCallback (MakeCallback (&NrUePhy::ReceivePss, phy));
  dlPhy->SetNrPhyDlHarqFeedbackCallback (MakeCallback (&NrUePhy::ReceiveNrDlHarqFeedback, phy));
  nas->SetForwardUpCallback (MakeCallback (&NrUeNetDevice::Receive, dev));

  if (m_ngcHelper != 0)
    {
      m_ngcHelper->AddUe (dev, dev->GetImsi ());
    }

  dev->Initialize ();

  return dev;
}


void
NrHelper::Attach (NetDeviceContainer ueDevices)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      Attach (*i);
    }
}

void
NrHelper::Attach (Ptr<NetDevice> ueDevice)
{
  NS_LOG_FUNCTION (this);

  if (m_ngcHelper == 0)
    {
      NS_FATAL_ERROR ("This function is not valid without properly configured NGC");
    }

  Ptr<NrUeNetDevice> ueNrDevice = ueDevice->GetObject<NrUeNetDevice> ();
  if (ueNrDevice == 0)
    {
      NS_FATAL_ERROR ("The passed NetDevice must be an NrUeNetDevice");
    }

  // initiate cell selection
  Ptr<NgcUeNas> ueNas = ueNrDevice->GetNas ();
  NS_ASSERT (ueNas != 0);
  uint16_t dlEarfcn = ueNrDevice->GetDlEarfcn ();
  ueNas->StartCellSelection (dlEarfcn);

  // instruct UE to immediately enter CONNECTED mode after camping
  ueNas->Connect ();

  // activate default EPS bearer
  m_ngcHelper->ActivateEpsBearer (ueDevice, ueNrDevice->GetImsi (),
                                  NgcTft::Default (),
                                  EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT));
}

void
NrHelper::Attach (NetDeviceContainer ueDevices, Ptr<NetDevice> enbDevice)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      Attach (*i, enbDevice);
    }
}

void
NrHelper::Attach (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice)
{
  NS_LOG_FUNCTION (this);
  //enbRrc->SetCellId (enbDevice->GetObject<NrEnbNetDevice> ()->GetCellId ());

  Ptr<NrUeNetDevice> ueNrDevice = ueDevice->GetObject<NrUeNetDevice> ();
  Ptr<NrEnbNetDevice> enbNrDevice = enbDevice->GetObject<NrEnbNetDevice> ();

  Ptr<NgcUeNas> ueNas = ueNrDevice->GetNas ();
  ueNas->Connect (enbNrDevice->GetCellId (), enbNrDevice->GetDlEarfcn ());

  if (m_ngcHelper != 0)
    {
      // activate default EPS bearer
      m_ngcHelper->ActivateEpsBearer (ueDevice, ueNrDevice->GetImsi (), NgcTft::Default (), EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT));
    }

  // tricks needed for the simplified NR-only simulations 
  if (m_ngcHelper == 0)
    {
      ueDevice->GetObject<NrUeNetDevice> ()->SetTargetEnb (enbDevice->GetObject<NrEnbNetDevice> ());
    }
}

void
NrHelper::AttachToClosestEnb (NetDeviceContainer ueDevices, NetDeviceContainer enbDevices)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      AttachToClosestEnb (*i, enbDevices);
    }
}

void
NrHelper::AttachToClosestEnb (Ptr<NetDevice> ueDevice, NetDeviceContainer enbDevices)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (enbDevices.GetN () > 0, "empty enb device container");
  Vector uepos = ueDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
  double minDistance = std::numeric_limits<double>::infinity ();
  Ptr<NetDevice> closestEnbDevice;
  for (NetDeviceContainer::Iterator i = enbDevices.Begin (); i != enbDevices.End (); ++i)
    {
      Vector enbpos = (*i)->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
      double distance = CalculateDistance (uepos, enbpos);
      if (distance < minDistance)
        {
          minDistance = distance;
          closestEnbDevice = *i;
        }
    }
  NS_ASSERT (closestEnbDevice != 0);
  Attach (ueDevice, closestEnbDevice);
}

uint8_t
NrHelper::ActivateDedicatedEpsBearer (NetDeviceContainer ueDevices, EpsBearer bearer, Ptr<NgcTft> tft)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      uint8_t bearerId = ActivateDedicatedEpsBearer (*i, bearer, tft);
      return bearerId;
    }
  return 0;
}


uint8_t
NrHelper::ActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer, Ptr<NgcTft> tft)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_ngcHelper != 0, "dedicated EPS bearers cannot be set up when the NGC is not used");

  uint64_t imsi = ueDevice->GetObject<NrUeNetDevice> ()->GetImsi ();
  uint8_t bearerId = m_ngcHelper->ActivateEpsBearer (ueDevice, imsi, tft, bearer);
  return bearerId;
}

/**
 * \ingroup nr
 *
 * DrbActivatior allows user to activate bearers for UEs
 * when NGC is not used. Activation function is hooked to
 * the Enb RRC Connection Estabilished trace source. When
 * UE change its RRC state to CONNECTED_NORMALLY, activation
 * function is called and bearer is activated.
*/
class DrbActivator : public SimpleRefCount<DrbActivator>
{
public:
  /**
  * DrbActivator Constructor
  *
  * \param ueDevice the UeNetDevice for which bearer will be activated
  * \param bearer the bearer configuration
  */
  DrbActivator (Ptr<NetDevice> ueDevice, EpsBearer bearer);

  /**
   * Function hooked to the Enb RRC Connection Established trace source
   * Fired upon successful RRC connection establishment.
   *
   * \param a DrbActivator object
   * \param context
   * \param imsi
   * \param cellId
   * \param rnti
   */
  static void ActivateCallback (Ptr<DrbActivator> a, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti);

  /**
   * Procedure firstly checks if bearer was not activated, if IMSI
   * from trace source equals configured one and if UE is really
   * in RRC connected state. If all requirements are met, it performs
   * bearer activation.
   *
   * \param imsi
   * \param cellId
   * \param rnti
   */
  void ActivateDrb (uint64_t imsi, uint16_t cellId, uint16_t rnti);
private:
  /**
   * Bearer can be activated only once. This value stores state of
   * bearer. Initially is set to false and changed to true during
   * bearer activation.
   */
  bool m_active;
  /**
   * UeNetDevice for which bearer will be activated
   */
  Ptr<NetDevice> m_ueDevice;
  /**
   * Configuration of bearer which will be activated
   */
  EpsBearer m_bearer;
  /**
   * imsi the unique UE identifier
   */
  uint64_t m_imsi;
};

DrbActivator::DrbActivator (Ptr<NetDevice> ueDevice, EpsBearer bearer)
  : m_active (false),
    m_ueDevice (ueDevice),
    m_bearer (bearer),
    m_imsi (m_ueDevice->GetObject<NrUeNetDevice> ()->GetImsi ())
{
}

void
DrbActivator::ActivateCallback (Ptr<DrbActivator> a, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (a << context << imsi << cellId << rnti);
  a->ActivateDrb (imsi, cellId, rnti);
}

void
DrbActivator::ActivateDrb (uint64_t imsi, uint16_t cellId, uint16_t rnti)
{ 
  NS_LOG_FUNCTION (this << imsi << cellId << rnti << m_active);
  if ((!m_active) && (imsi == m_imsi))
    {
      Ptr<NrUeRrc> ueRrc = m_ueDevice->GetObject<NrUeNetDevice> ()->GetRrc ();
      NS_ASSERT (ueRrc->GetState () == NrUeRrc::CONNECTED_NORMALLY);
      uint16_t rnti = ueRrc->GetRnti ();
      Ptr<NrEnbNetDevice> enbNrDevice = m_ueDevice->GetObject<NrUeNetDevice> ()->GetTargetEnb ();
      Ptr<NrEnbRrc> enbRrc = enbNrDevice->GetObject<NrEnbNetDevice> ()->GetRrc ();
      NS_ASSERT (ueRrc->GetCellId () == enbNrDevice->GetCellId ());
      Ptr<UeManager> ueManager = enbRrc->GetUeManager (rnti);
      NS_ASSERT (ueManager->GetState () == UeManager::CONNECTED_NORMALLY
                 || ueManager->GetState () == UeManager::CONNECTION_RECONFIGURATION);
      NgcEnbN2SapUser::DataRadioBearerSetupRequestParameters params;
      params.rnti = rnti;
      params.bearer = m_bearer;
      params.bearerId = 0;
      params.gtpTeid = 0; // don't care
      enbRrc->GetN2SapUser ()->DataRadioBearerSetupRequest (params);
      m_active = true;
    }
}


void 
NrHelper::ActivateDataRadioBearer (Ptr<NetDevice> ueDevice, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this << ueDevice);
  NS_ASSERT_MSG (m_ngcHelper == 0, "this method must not be used when the NGC is being used");

  // Normally it is the NGC that takes care of activating DRBs
  // when the UE gets connected. When the NGC is not used, we achieve
  // the same behavior by hooking a dedicated DRB activation function
  // to the Enb RRC Connection Established trace source


  Ptr<NrEnbNetDevice> enbNrDevice = ueDevice->GetObject<NrUeNetDevice> ()->GetTargetEnb ();

  std::ostringstream path;
  path << "/NodeList/" << enbNrDevice->GetNode ()->GetId () 
       << "/DeviceList/" << enbNrDevice->GetIfIndex ()
       << "/NrEnbRrc/ConnectionEstablished";
  Ptr<DrbActivator> arg = Create<DrbActivator> (ueDevice, bearer);
  Config::Connect (path.str (), MakeBoundCallback (&DrbActivator::ActivateCallback, arg));
}

void
NrHelper::AddX2Interface (NodeContainer enbNodes)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (m_ngcHelper != 0, "X2 interfaces cannot be set up when the NGC is not used");

  for (NodeContainer::Iterator i = enbNodes.Begin (); i != enbNodes.End (); ++i)
    {
      for (NodeContainer::Iterator j = i + 1; j != enbNodes.End (); ++j)
        {
          AddX2Interface (*i, *j);
        }
    }
}

void
NrHelper::AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("setting up the X2 interface");

  m_ngcHelper->AddX2Interface (enbNode1, enbNode2);
}

void
NrHelper::HandoverRequest (Time hoTime, Ptr<NetDevice> ueDev, Ptr<NetDevice> sourceEnbDev, Ptr<NetDevice> targetEnbDev)
{
  NS_LOG_FUNCTION (this << ueDev << sourceEnbDev << targetEnbDev);
  NS_ASSERT_MSG (m_ngcHelper, "Handover requires the use of the NGC - did you forget to call NrHelper::SetNgcHelper () ?");
  Simulator::Schedule (hoTime, &NrHelper::DoHandoverRequest, this, ueDev, sourceEnbDev, targetEnbDev);
}

void
NrHelper::DoHandoverRequest (Ptr<NetDevice> ueDev, Ptr<NetDevice> sourceEnbDev, Ptr<NetDevice> targetEnbDev)
{
  NS_LOG_FUNCTION (this << ueDev << sourceEnbDev << targetEnbDev);

  uint16_t targetCellId = targetEnbDev->GetObject<NrEnbNetDevice> ()->GetCellId ();
  Ptr<NrEnbRrc> sourceRrc = sourceEnbDev->GetObject<NrEnbNetDevice> ()->GetRrc ();
  uint16_t rnti = ueDev->GetObject<NrUeNetDevice> ()->GetRrc ()->GetRnti ();
  sourceRrc->SendHandoverRequest (rnti, targetCellId);
}

void
NrHelper::DeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice,Ptr<NetDevice> enbDevice, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << ueDevice << bearerId);
  NS_ASSERT_MSG (m_ngcHelper != 0, "Dedicated EPS bearers cannot be de-activated when the NGC is not used");
  NS_ASSERT_MSG (bearerId != 1, "Default bearer cannot be de-activated until and unless and UE is released");

  DoDeActivateDedicatedEpsBearer (ueDevice, enbDevice, bearerId);
}

void
NrHelper::DoDeActivateDedicatedEpsBearer (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << ueDevice << bearerId);

  //Extract IMSI and rnti
  uint64_t imsi = ueDevice->GetObject<NrUeNetDevice> ()->GetImsi ();
  uint16_t rnti = ueDevice->GetObject<NrUeNetDevice> ()->GetRrc ()->GetRnti ();


  Ptr<NrEnbRrc> enbRrc = enbDevice->GetObject<NrEnbNetDevice> ()->GetRrc ();

  enbRrc->DoSendReleaseDataRadioBearer (imsi,rnti,bearerId);
}


void 
NrHelper::ActivateDataRadioBearer (NetDeviceContainer ueDevices, EpsBearer bearer)
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator i = ueDevices.Begin (); i != ueDevices.End (); ++i)
    {
      ActivateDataRadioBearer (*i, bearer);
    }
}

void
NrHelper::EnableLogComponents (void)
{
  LogComponentEnable ("NrHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("NrEnbRrc", LOG_LEVEL_ALL);
  LogComponentEnable ("NrUeRrc", LOG_LEVEL_ALL);
  LogComponentEnable ("NrEnbMac", LOG_LEVEL_ALL);
  LogComponentEnable ("NrUeMac", LOG_LEVEL_ALL);
  LogComponentEnable ("NrRlc", LOG_LEVEL_ALL);
  LogComponentEnable ("NrRlcUm", LOG_LEVEL_ALL);
  LogComponentEnable ("NrRlcAm", LOG_LEVEL_ALL);
  LogComponentEnable ("NrRrFfMacScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("NrPfFfMacScheduler", LOG_LEVEL_ALL);

  LogComponentEnable ("NrPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("NrEnbPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("NrUePhy", LOG_LEVEL_ALL);
  LogComponentEnable ("NrSpectrumValueHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("NrSpectrumPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("NrInterference", LOG_LEVEL_ALL);
  LogComponentEnable ("NrChunkProcessor", LOG_LEVEL_ALL);

  std::string propModelStr = m_dlPathlossModelFactory.GetTypeId ().GetName ().erase (0,5).c_str ();
  LogComponentEnable ("NrNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("NrUeNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("NrEnbNetDevice", LOG_LEVEL_ALL);

  LogComponentEnable ("NrRadioBearerStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("NrStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("NrMacStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("NrPhyTxStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("NrPhyRxStatsCalculator", LOG_LEVEL_ALL);
  LogComponentEnable ("NrPhyStatsCalculator", LOG_LEVEL_ALL);


}

void
NrHelper::EnableTraces (void)
{
  EnablePhyTraces ();
  EnableMacTraces ();
  EnableRlcTraces ();
  EnablePdcpTraces ();
}

void
NrHelper::EnableRlcTraces (void)
{
  NS_ASSERT_MSG (m_rlcStats == 0, "please make sure that NrHelper::EnableRlcTraces is called at most once");
  m_rlcStats = CreateObject<NrRadioBearerStatsCalculator> ("RLC");
  m_radioBearerStatsConnector.EnableRlcStats (m_rlcStats);
}

int64_t
NrHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  if ((m_fadingModule != 0) && (m_fadingStreamsAssigned == false))
    {
      Ptr<NrTraceFadingLossModel> tflm = m_fadingModule->GetObject<NrTraceFadingLossModel> ();
      if (tflm != 0)
        {
          currentStream += tflm->AssignStreams (currentStream);
          m_fadingStreamsAssigned = true;
        }
    }
  Ptr<NetDevice> netDevice;
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<NrEnbNetDevice> nrEnb = DynamicCast<NrEnbNetDevice> (netDevice);
      if (nrEnb)
        {
          Ptr<NrSpectrumPhy> dlPhy = nrEnb->GetPhy ()->GetDownlinkSpectrumPhy ();
          Ptr<NrSpectrumPhy> ulPhy = nrEnb->GetPhy ()->GetUplinkSpectrumPhy ();
          currentStream += dlPhy->AssignStreams (currentStream);
          currentStream += ulPhy->AssignStreams (currentStream);
        }
      Ptr<NrUeNetDevice> nrUe = DynamicCast<NrUeNetDevice> (netDevice);
      if (nrUe)
        {
          Ptr<NrSpectrumPhy> dlPhy = nrUe->GetPhy ()->GetDownlinkSpectrumPhy ();
          Ptr<NrSpectrumPhy> ulPhy = nrUe->GetPhy ()->GetUplinkSpectrumPhy ();
          Ptr<NrUeMac> ueMac = nrUe->GetMac ();
          currentStream += dlPhy->AssignStreams (currentStream);
          currentStream += ulPhy->AssignStreams (currentStream);
          currentStream += ueMac->AssignStreams (currentStream);
        }
    }
  return (currentStream - stream);
}


void
NrHelper::EnablePhyTraces (void)
{
  EnableDlPhyTraces ();
  EnableUlPhyTraces ();
  EnableDlTxPhyTraces ();
  EnableUlTxPhyTraces ();
  EnableDlRxPhyTraces ();
  EnableUlRxPhyTraces ();
}

void
NrHelper::EnableDlTxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/NrEnbPhy/DlPhyTransmission",
                   MakeBoundCallback (&NrPhyTxStatsCalculator::DlPhyTransmissionCallback, m_phyTxStats));
}

void
NrHelper::EnableUlTxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/NrUePhy/UlPhyTransmission",
                   MakeBoundCallback (&NrPhyTxStatsCalculator::UlPhyTransmissionCallback, m_phyTxStats));
}

void
NrHelper::EnableDlRxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/NrUePhy/DlSpectrumPhy/DlPhyReception",
                   MakeBoundCallback (&NrPhyRxStatsCalculator::DlPhyReceptionCallback, m_phyRxStats));
}

void
NrHelper::EnableUlRxPhyTraces (void)
{
  Config::Connect ("/NodeList/*/DeviceList/*/NrEnbPhy/UlSpectrumPhy/UlPhyReception",
                   MakeBoundCallback (&NrPhyRxStatsCalculator::UlPhyReceptionCallback, m_phyRxStats));
}


void
NrHelper::EnableMacTraces (void)
{
  EnableDlMacTraces ();
  EnableUlMacTraces ();
}


void
NrHelper::EnableDlMacTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/NrEnbMac/DlScheduling",
                   MakeBoundCallback (&NrMacStatsCalculator::DlSchedulingCallback, m_macStats));
}

void
NrHelper::EnableUlMacTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/NrEnbMac/UlScheduling",
                   MakeBoundCallback (&NrMacStatsCalculator::UlSchedulingCallback, m_macStats));
}

void
NrHelper::EnableDlPhyTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/NrUePhy/ReportCurrentCellRsrpSinr",
                   MakeBoundCallback (&NrPhyStatsCalculator::ReportCurrentCellRsrpSinrCallback, m_phyStats));
}

void
NrHelper::EnableUlPhyTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::Connect ("/NodeList/*/DeviceList/*/NrEnbPhy/ReportUeSinr",
                   MakeBoundCallback (&NrPhyStatsCalculator::ReportUeSinr, m_phyStats));
  Config::Connect ("/NodeList/*/DeviceList/*/NrEnbPhy/ReportInterference",
                   MakeBoundCallback (&NrPhyStatsCalculator::ReportInterference, m_phyStats));

}

Ptr<NrRadioBearerStatsCalculator>
NrHelper::GetRlcStats (void)
{
  return m_rlcStats;
}

void
NrHelper::EnablePdcpTraces (void)
{
  NS_ASSERT_MSG (m_pdcpStats == 0, "please make sure that NrHelper::EnablePdcpTraces is called at most once");
  m_pdcpStats = CreateObject<NrRadioBearerStatsCalculator> ("PDCP");
  m_radioBearerStatsConnector.EnablePdcpStats (m_pdcpStats);
}

Ptr<NrRadioBearerStatsCalculator>
NrHelper::GetPdcpStats (void)
{
  return m_pdcpStats;
}

} // namespace ns3
