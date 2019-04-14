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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#ifndef NGC_X2_H
#define NGC_X2_H

#include "ns3/socket.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/object.h"
 #include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"

#include "ns3/ngc-x2-sap.h"

#include <map>

namespace ns3 {


class NrX2IfaceInfo : public SimpleRefCount<NrX2IfaceInfo>
{
public:
  NrX2IfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket, Ptr<Socket> localUserPlaneSocket);
  virtual ~NrX2IfaceInfo (void);

  NrX2IfaceInfo& operator= (const NrX2IfaceInfo &);

public:
  Ipv4Address   m_remoteIpAddr;
  Ptr<Socket>   m_localCtrlPlaneSocket;
  Ptr<Socket>   m_localUserPlaneSocket;
};


class NrX2CellInfo : public SimpleRefCount<NrX2CellInfo>
{
public:
  NrX2CellInfo (uint16_t localCellId, uint16_t remoteCellId);
  virtual ~NrX2CellInfo (void);

  NrX2CellInfo& operator= (const NrX2CellInfo &);

public:
  uint16_t m_localCellId;
  uint16_t m_remoteCellId;
};


/**
 * \ingroup nr
 *
 * This entity is installed inside an eNB and provides the functionality for the X2 interface
 */
class NgcX2 : public Object
{
  friend class NgcX2SpecificNgcX2SapProvider<NgcX2>;
  friend class NgcX2PdcpSpecificProvider<NgcX2>;
  friend class NgcX2RlcSpecificProvider<NgcX2>;

public:
  /** 
   * Constructor
   */
  NgcX2 ();

  /**
   * Destructor
   */
  virtual ~NgcX2 (void);

  static TypeId GetTypeId (void);
  virtual void DoDispose (void);
  bool isAdditionalMmWave=false; //sjkang1016

  /**
   * \param s the X2 SAP User to be used by this NGC X2 entity
   */
  void SetNgcX2SapUser (NgcX2SapUser * s);

  /**
   * \return the X2 SAP Provider interface offered by this NGC X2 entity
   */
  NgcX2SapProvider* GetNgcX2SapProvider ();

  /**
   * \return the X2 Pdcp Provider interface offered by this NGC X2 entity
   */
  NgcX2PdcpProvider* GetNgcX2PdcpProvider ();
  void SetNgcX2PdcpUser(NgcX2PdcpUser *s); //sjkang1114

  /**
   * \return the X2 Rlc Provider interface offered by this NGC X2 entity
   */
  NgcX2RlcProvider* GetNgcX2RlcProvider ();

  /**
   * \param the teid of the MC device
   * \param the X2 Rlc User interface associated to the teid
   */
  void SetMcNgcX2RlcUser (uint32_t teid, NgcX2RlcUser* rlcUser);
  void SetMcNgcX2RlcUser_2 (uint32_t teid, NgcX2RlcUser* rlcUser); //sjkang1016

  /**
   * \param the teid of the MC device
   * \param the X2 Pdcp User interface associated to the teid
   */
  void SetMcNgcX2PdcpUser (uint32_t teid, NgcX2PdcpUser* pdcpUser);
  void SetMcNgcX2PdcpUser_2 (uint32_t teid, NgcX2PdcpUser* pdcpUser); //sjkang1016


  /**
   * Add an X2 interface to this NGC X2 entity
   * \param enb1CellId the cell ID of the current eNodeB
   * \param enb1X2Address the address of the current eNodeB
   * \param enb2CellId the cell ID of the neighbouring eNodeB
   * \param enb2X2Address the address of the neighbouring eNodeB
   */
  void AddX2Interface (uint16_t enb1CellId, Ipv4Address enb1X2Address,
                       uint16_t enb2CellId, Ipv4Address enb2X2Address);


  /** 
   * Method to be assigned to the recv callback of the X2-C (X2 Control Plane) socket.
   * It is called when the eNB receives a packet from the peer eNB of the X2-C interface
   * 
   * \param socket socket of the X2-C interface
   */
  void RecvFromX2cSocket (Ptr<Socket> socket);

  /** 
   * Method to be assigned to the recv callback of the X2-U (X2 User Plane) socket.
   * It is called when the eNB receives a packet from the peer eNB of the X2-U interface
   * 
   * \param socket socket of the X2-U interface
   */
  void RecvFromX2uSocket (Ptr<Socket> socket);

  /**
   * TracedCallback signature for
   *
   * \param [in] source 
   * \param [in] target 
   * \param [in] bytes The packet size.
   * \param [in] delay Delay since sender timestamp, in ns.
   */
  typedef void (* ReceiveTracedCallback)
    (uint16_t sourceCellId, uint16_t targetCellId, uint32_t bytes, uint64_t delay, bool data);

protected:
  // Interface provided by NgcX2SapProvider
virtual  void DoReceiveAssistantInformation(NgcX2Sap::AssistantInformationForSplitting info);//sjkang1114

  virtual void DoSendHandoverRequest (NgcX2SapProvider::HandoverRequestParams params);
  virtual void DoSendRlcSetupRequest (NgcX2SapProvider::RlcSetupRequest params);
  virtual void DoSendRlcSetupCompleted (NgcX2SapProvider::UeDataParams);
  virtual void DoSendHandoverRequestAck (NgcX2SapProvider::HandoverRequestAckParams params);
  virtual void DoSendHandoverRequestAckToNr (NgcX2SapProvider::HandoverRequestAckParams params);
  virtual void DoSendHandoverPreparationFailure (NgcX2SapProvider::HandoverPreparationFailureParams params);
  virtual void DoSendSnStatusTransfer (NgcX2SapProvider::SnStatusTransferParams params);
  virtual void DoSendUeContextRelease (NgcX2SapProvider::UeContextReleaseParams params);
  virtual void DoSendLoadInformation (NgcX2SapProvider::LoadInformationParams params);
  virtual void DoSendResourceStatusUpdate (NgcX2SapProvider::ResourceStatusUpdateParams params);
  virtual void DoSendUeData (NgcX2SapProvider::UeDataParams params);
  virtual void DoSendMcPdcpPdu (NgcX2SapProvider::UeDataParams params);
  virtual void DoReceiveMcPdcpSdu (NgcX2SapProvider::UeDataParams params);
  //virtual void DoReceiveAssistantInformation(NgcX2SapProvider::AssistantInformationForSplitting info); //sjkang1114
  virtual void DoSendUeSinrUpdate(NgcX2Sap::UeImsiSinrParams params);
  virtual void DoSendMcHandoverRequest (NgcX2SapProvider::SecondaryHandoverParams params);
  virtual void DoDuplicateRlcBuffer(NgcX2SapProvider::SendBufferDuplicationMessage params); //sjkang
  virtual void DoNotifyNrMmWaveHandoverCompleted (NgcX2SapProvider::SecondaryHandoverParams params);
  virtual void DoNotifyCoordinatorHandoverFailed(NgcX2SapProvider::HandoverFailedParams params);
  virtual void DoSendSwitchConnectionToMmWave(NgcX2SapProvider::SwitchConnectionParams params);
  virtual void DoSendSecondaryCellHandoverCompleted(NgcX2SapProvider::SecondaryHandoverCompletedParams params);

  // these methods are not used to send messages but to change the internal state of the NgcX2
  virtual void DoAddTeidToBeForwarded(uint32_t teid, uint16_t targetCellId);
  virtual void DoRemoveTeidToBeForwarded(uint32_t teid);

  NgcX2SapUser* m_x2SapUser;
  NgcX2SapProvider* m_x2SapProvider;
  
  /**
   * Map the PdcpUser to a certain teid
   */
  std::map < uint32_t, NgcX2PdcpUser* > m_x2PdcpUserMap;
  std::map < uint32_t, NgcX2PdcpUser* > m_x2PdcpUserMap_2; //sjkang1016

  // The PdcpProvider offered by this X2 interface
  NgcX2PdcpProvider* m_x2PdcpProvider;
  NgcX2PdcpUser * m_x2PdcpUser; //sjkang1114
  /**
   * Map the RlcUser to a certain teid
   */
  std::map < uint32_t, NgcX2RlcUser* > m_x2RlcUserMap;
  std::map < uint32_t, NgcX2RlcUser* > m_x2RlcUserMap_2;//sjkang1016

  // The RlcProvider offered by this X2 interface
  NgcX2RlcProvider* m_x2RlcProvider;

private:

  /**
   * Map the targetCellId to the corresponding (sourceSocket, remoteIpAddr) to be used
   * to send the X2 message
   */
  std::map < uint16_t, Ptr<NrX2IfaceInfo> > m_x2InterfaceSockets;

  /**
   * Map the localSocket (the one receiving the X2 message) 
   * to the corresponding (sourceCellId, targetCellId) associated with the X2 interface
   */
  std::map < Ptr<Socket>, Ptr<NrX2CellInfo> > m_x2InterfaceCellIds;

  /**
   * UDP ports to be used for the X2 interfaces: X2-C and X2-U
   */
  uint16_t m_x2cUdpPort;
  uint16_t m_x2uUdpPort;

  TracedCallback<uint16_t, uint16_t, uint32_t, uint64_t, bool> m_rxPdu;

  /**
   * Map the gtpTeid to the targetCellId to which the packet should be forwarded
   * during a secondary cell handover
   */
  std::map <uint32_t, uint16_t> m_teidToBeForwardedMap;

};

} //namespace ns3

#endif // NGC_X2_H
