/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Authors: Nicola Baldo <nbaldo@cttc.es>
 *          Lluis Parcerisa <lparcerisa@cttc.cat>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */


#ifndef NR_RRC_PROTOCOL_REAL_H
#define NR_RRC_PROTOCOL_REAL_H

#include <stdint.h>
#include <map>

#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/nr-rrc-sap.h>
#include <ns3/nr-pdcp-sap.h>
#include <ns3/nr-rlc-sap.h>
#include <ns3/nr-rrc-header.h>
#include <ns3/ngc-x2-sap.h>
namespace ns3 {

class NrUeRrcSapProvider;
class NrUeRrcSapUser;
class NrEnbRrcSapProvider;
class NrUeRrc;


/**
 * Models the transmission of RRC messages from the UE to the eNB in
 * a real fashion, by creating real RRC PDUs and transmitting them
 * over Signaling Radio Bearers using radio resources allocated by the
 * NR MAC scheduler.
 * 
 */
class NrUeRrcProtocolReal : public Object
{
  friend class MemberNrUeRrcSapUser<NrUeRrcProtocolReal>;
  friend class NrRlcSpecificNrRlcSapUser<NrUeRrcProtocolReal>;
  friend class NrPdcpSpecificNrPdcpSapUser<NrUeRrcProtocolReal>;

public:
  NrUeRrcProtocolReal ();
  virtual ~NrUeRrcProtocolReal ();

  // inherited from Object
  virtual void DoDispose (void);
  static TypeId GetTypeId (void);

  void SetNrUeRrcSapProvider (NrUeRrcSapProvider* p);
  NrUeRrcSapUser* GetNrUeRrcSapUser ();

  void SetUeRrc (Ptr<NrUeRrc> rrc);
  void DoReceiveNrAssistantInfo(NgcX2Sap::AssistantInformationForSplitting info); //sjkang

  // jhlim
  void DoSendRrcIdentityResponse (NrRrcSap::RrcIdentityResponse msg);
  void DoSendRrcRegistrationComplete (NrRrcSap::RrcRegistrationComplete msg);
private:
  // methods forwarded from NrUeRrcSapUser
  void DoSetup (NrUeRrcSapUser::SetupParameters params);
  void DoSendRrcConnectionRequest (NrRrcSap::RrcConnectionRequest msg);
  void DoSendRrcConnectionSetupCompleted (NrRrcSap::RrcConnectionSetupCompleted msg);
  void DoSendRrcConnectionReconfigurationCompleted (NrRrcSap::RrcConnectionReconfigurationCompleted msg);
  void DoSendRrcConnectionReestablishmentRequest (NrRrcSap::RrcConnectionReestablishmentRequest msg);
  void DoSendRrcConnectionReestablishmentComplete (NrRrcSap::RrcConnectionReestablishmentComplete msg);
  void DoSendMeasurementReport (NrRrcSap::MeasurementReport msg);
  void DoSendNotifySecondaryCellConnected (uint16_t mmWaveRnti, uint16_t mmWaveCellId);

  void SetEnbRrcSapProvider ();
  void DoReceivePdcpPdu (Ptr<Packet> p);
  void DoReceivePdcpSdu (NrPdcpSapUser::ReceivePdcpSduParameters params);
  virtual void  SendNrAssi(NgcX2Sap::AssistantInformationForSplitting info);
  Ptr<NrUeRrc> m_rrc;
  uint16_t m_rnti;
  NrUeRrcSapProvider* m_ueRrcSapProvider;
  NrUeRrcSapUser* m_ueRrcSapUser;
  NrEnbRrcSapProvider* m_enbRrcSapProvider;

  NrUeRrcSapUser::SetupParameters m_setupParameters;
  NrUeRrcSapProvider::CompleteSetupParameters m_completeSetupParameters;

};


/**
 * Models the transmission of RRC messages from the UE to the eNB in
 * a real fashion, by creating real RRC PDUs and transmitting them
 * over Signaling Radio Bearers using radio resources allocated by the
 * NR MAC scheduler.
 *
 */
class NrEnbRrcProtocolReal : public Object
{
  friend class MemberNrEnbRrcSapUser<NrEnbRrcProtocolReal>;
  friend class NrPdcpSpecificNrPdcpSapUser<NrEnbRrcProtocolReal>;
  friend class NrRlcSpecificNrRlcSapUser<NrEnbRrcProtocolReal>;
  friend class RealProtocolRlcSapUser;

public:
  NrEnbRrcProtocolReal ();
  virtual ~NrEnbRrcProtocolReal ();

  // inherited from Object
  virtual void DoDispose (void);
  static TypeId GetTypeId (void);

  void SetNrEnbRrcSapProvider (NrEnbRrcSapProvider* p);
  NrEnbRrcSapUser* GetNrEnbRrcSapUser ();

  void SetCellId (uint16_t cellId);

  NrUeRrcSapProvider* GetUeRrcSapProvider (uint16_t rnti);
  void SetUeRrcSapProvider (uint16_t rnti, NrUeRrcSapProvider* p);
  virtual void SendNrAssi(NgcX2Sap::AssistantInformationForSplitting info); //sjkang
  void DoReceiveNrAssistantInfo(NgcX2Sap::AssistantInformationForSplitting info); //sjkang

  // jhlim
  void DoSendRrcIdentityRequest (uint16_t rnti, NrRrcSap::RrcIdentityRequest msg); // to UE
  void DoSendRrcRegistrationAccept (uint16_t rnti, NrRrcSap::RrcRegistrationAccept msg); // to UE
private:
  // methods forwarded from NrEnbRrcSapUser
  void DoSetupUe (uint16_t rnti, NrEnbRrcSapUser::SetupUeParameters params);
  void DoRemoveUe (uint16_t rnti);
  void DoSendSystemInformation (NrRrcSap::SystemInformation msg);
  void SendSystemInformation (NrRrcSap::SystemInformation msg);
  void DoSendRrcConnectionSetup (uint16_t rnti, NrRrcSap::RrcConnectionSetup msg);
  void DoSendRrcConnectionReconfiguration (uint16_t rnti, NrRrcSap::RrcConnectionReconfiguration msg);
  void DoSendRrcConnectionReestablishment (uint16_t rnti, NrRrcSap::RrcConnectionReestablishment msg);
  void DoSendRrcConnectionReestablishmentReject (uint16_t rnti, NrRrcSap::RrcConnectionReestablishmentReject msg);
  void DoSendRrcConnectionRelease (uint16_t rnti, NrRrcSap::RrcConnectionRelease msg);
  void DoSendRrcConnectionReject (uint16_t rnti, NrRrcSap::RrcConnectionReject msg);
  void DoSendRrcConnectionSwitch (uint16_t rnti, NrRrcSap::RrcConnectionSwitch msg);
  void DoSendRrcConnectToMmWave (uint16_t rnti, uint16_t mmWaveCellId, uint16_t mmWaveCellId_2);//sjkang0205
  Ptr<Packet> DoEncodeHandoverPreparationInformation (NrRrcSap::HandoverPreparationInfo msg);
  NrRrcSap::HandoverPreparationInfo DoDecodeHandoverPreparationInformation (Ptr<Packet> p);
  Ptr<Packet> DoEncodeHandoverCommand (NrRrcSap::RrcConnectionReconfiguration msg);
  NrRrcSap::RrcConnectionReconfiguration DoDecodeHandoverCommand (Ptr<Packet> p);

  void DoReceivePdcpSdu (NrPdcpSapUser::ReceivePdcpSduParameters params);
  void DoReceivePdcpPdu (uint16_t rnti, Ptr<Packet> p);

  uint16_t m_rnti;
  uint16_t m_cellId;
  NrEnbRrcSapProvider* m_enbRrcSapProvider;
  NrEnbRrcSapUser* m_enbRrcSapUser;
  std::map<uint16_t, NrUeRrcSapProvider*> m_enbRrcSapProviderMap;
  std::map<uint16_t, NrEnbRrcSapUser::SetupUeParameters> m_setupUeParametersMap;
  std::map<uint16_t, NrEnbRrcSapProvider::CompleteSetupUeParameters> m_completeSetupUeParametersMap;


};

///////////////////////////////////////

class RealProtocolRlcSapUser : public NrRlcSapUser
{
public:
  RealProtocolRlcSapUser (NrEnbRrcProtocolReal* pdcp, uint16_t rnti);

  // Interface implemented from NrRlcSapUser
  virtual void ReceivePdcpPdu (Ptr<Packet> p);
  virtual void SendNrAssi(NgcX2Sap::AssistantInformationForSplitting);

private:
  RealProtocolRlcSapUser ();
  NrEnbRrcProtocolReal* m_pdcp;
  uint16_t m_rnti;
};


}


#endif // NR_RRC_PROTOCOL_REAL_H
