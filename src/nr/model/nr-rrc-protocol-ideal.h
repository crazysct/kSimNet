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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */


#ifndef NR_RRC_PROTOCOL_IDEAL_H
#define NR_RRC_PROTOCOL_IDEAL_H

#include <stdint.h>
#include <map>

#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/nr-rrc-sap.h>

namespace ns3 {

class NrUeRrcSapProvider;
class NrUeRrcSapUser;
class NrEnbRrcSapProvider;
class NrUeRrc;


/**
 * Models the transmission of RRC messages from the UE to the eNB in
 * an ideal fashion, without errors and without consuming any radio
 * resources. 
 * 
 */
class NrUeRrcProtocolIdeal : public Object
{
  friend class MemberNrUeRrcSapUser<NrUeRrcProtocolIdeal>;

public:

  NrUeRrcProtocolIdeal ();
  virtual ~NrUeRrcProtocolIdeal ();

  // inherited from Object
  virtual void DoDispose (void);
  static TypeId GetTypeId (void);

  void SetNrUeRrcSapProvider (NrUeRrcSapProvider* p);
  NrUeRrcSapUser* GetNrUeRrcSapUser ();
  
  void SetUeRrc (Ptr<NrUeRrc> rrc);

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

  Ptr<NrUeRrc> m_rrc;
  uint16_t m_rnti;
  NrUeRrcSapProvider* m_ueRrcSapProvider;
  NrUeRrcSapUser* m_ueRrcSapUser;
  NrEnbRrcSapProvider* m_enbRrcSapProvider;
  
};


/**
 * Models the transmission of RRC messages from the UE to the eNB in
 * an ideal fashion, without errors and without consuming any radio
 * resources. 
 * 
 */
class NrEnbRrcProtocolIdeal : public Object
{
  friend class MemberNrEnbRrcSapUser<NrEnbRrcProtocolIdeal>;

public:

  NrEnbRrcProtocolIdeal ();
  virtual ~NrEnbRrcProtocolIdeal ();

  // inherited from Object
  virtual void DoDispose (void);
  static TypeId GetTypeId (void);

  void SetNrEnbRrcSapProvider (NrEnbRrcSapProvider* p);
  NrEnbRrcSapUser* GetNrEnbRrcSapUser ();

  void SetCellId (uint16_t cellId);

  NrUeRrcSapProvider* GetUeRrcSapProvider (uint16_t rnti);
  void SetUeRrcSapProvider (uint16_t rnti, NrUeRrcSapProvider* p);

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
  void DoSendRrcConnectToMmWave (uint16_t rnti, uint16_t mmWaveCellId,uint16_t mmWaveCellId_2);
  Ptr<Packet> DoEncodeHandoverPreparationInformation (NrRrcSap::HandoverPreparationInfo msg);
  NrRrcSap::HandoverPreparationInfo DoDecodeHandoverPreparationInformation (Ptr<Packet> p);
  Ptr<Packet> DoEncodeHandoverCommand (NrRrcSap::RrcConnectionReconfiguration msg);
  NrRrcSap::RrcConnectionReconfiguration DoDecodeHandoverCommand (Ptr<Packet> p);

  // jhlim
  void DoSendRrcIdentityRequest (uint16_t rnti, NrRrcSap::RrcIdentityRequest msg); // to UE
  void DoSendRrcRegistrationAccept (uint16_t rnti, NrRrcSap::RrcRegistrationAccept msg); // to UE


  uint16_t m_rnti;
  uint16_t m_cellId;
  NrEnbRrcSapProvider* m_enbRrcSapProvider;
  NrEnbRrcSapUser* m_enbRrcSapUser;
  std::map<uint16_t, NrUeRrcSapProvider*> m_enbRrcSapProviderMap;
  
};



}


#endif // NR_RRC_PROTOCOL_IDEAL_H
