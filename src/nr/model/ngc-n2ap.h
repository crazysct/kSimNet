/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2016, University of Padova, Dep. of Information Engineering, SIGNET lab

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
 * Author: Michele Polese <michele.polese@gmail.com> 
 */

#ifndef NGC_N2AP_H
#define NGC_N2AP_H

#include "ns3/socket.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/object.h"

#include "ns3/ngc-n2ap-sap.h"

#include <map>

namespace ns3 {


class N2apIfaceInfo : public SimpleRefCount<N2apIfaceInfo>
{
public:
  N2apIfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket);
  virtual ~N2apIfaceInfo (void);

  N2apIfaceInfo& operator= (const N2apIfaceInfo &);

public:
  Ipv4Address   m_remoteIpAddr;
  Ptr<Socket>   m_localCtrlPlaneSocket;
};


class N2apConnectionInfo : public SimpleRefCount<N2apConnectionInfo>
{
public:
  N2apConnectionInfo (uint16_t enbId, uint16_t amfId);
  virtual ~N2apConnectionInfo (void);

  N2apConnectionInfo& operator= (const N2apConnectionInfo &);

public:
  uint16_t m_enbId;
  uint16_t m_amfId;
};


/**
 * \ingroup nr
 *
 * This entity is installed inside an eNB and provides the functionality for the N2AP interface
 */
class NgcN2apEnb : public Object
{
  friend class MemberNgcN2apSapEnbProvider<NgcN2apEnb>;

public:
  /** 
   * Constructor
   */
  NgcN2apEnb (Ptr<Socket> localSocket, Ipv4Address enbAddress, Ipv4Address amfAddress, uint16_t cellId, uint16_t amfId);

  /**
   * Destructor
   */
  virtual ~NgcN2apEnb (void);

  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


  /**
   * \param s the N2ap SAP User to be used by this NGC N2ap eNB entity in order to call methods of ngcEnbApplication
   */
  void SetNgcN2apSapEnbUser (NgcN2apSapEnb * s);

  /**
   * \return the N2ap SAP Provider interface offered by this NGC N2ap entity
   */
  NgcN2apSapEnbProvider* GetNgcN2apSapEnbProvider ();


  /**
   * Add an N2ap interface to this NGC N2ap entity
   * \param enbId the cell ID of the eNodeB on which this is installed
   * \param enbAddress the address of the eNodeB on which this is installed
   * \param amfId the ID of the AMF to which the eNB is connected
   * \param amfAddress the address of the AMF to which the eNB is connected
   * \param the socket created in the Ngc Helper
   */
  void AddN2apInterface (uint16_t enbId, Ipv4Address enbAddress,
                       uint16_t amfId, Ipv4Address amfAddress, Ptr<Socket> localN2apSocket);


  /** 
   * Method to be assigned to the recv callback of the N2ap-C (N2ap Control Plane) socket.
   * It is called when the eNB receives a packet from the AMF on the N2ap interface
   * 
   * \param socket socket of the N2ap interface
   */
  void RecvFromN2apSocket (Ptr<Socket> socket);


protected:
  // Interface provided by NgcN2apSapEnbProvider

  // jhlim
  virtual void DoSendRegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t stmsi, uint16_t ecgi);
  virtual void DoSendN2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t stmsi, uint16_t ecgi);
  virtual void DoSendErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication );
  virtual void DoSendInitialContextSetupResponse (uint64_t amfUeN2Id,
                                                  uint16_t enbUeN2Id,
                                                  std::list<NgcN2apSap::ErabSetupItem> erabSetupList);
  virtual void DoSendPathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t gci, std::list<NgcN2apSap::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
  ;

  // jhlim
  virtual void DoSendIdentityResponse (uint64_t amfUeN2Id,
  									   uint16_t enbUeN2Id);
  virtual void DoSendRegistrationComplete (uint64_t amfUeN2Id, uint16_t enbUeN2Id);

  NgcN2apSapEnb* m_n2apSapUser;
  NgcN2apSapEnbProvider* m_n2apSapProvider;


private:

  /**
   * Map the amfId to the corresponding (sourceSocket, remoteIpAddr) to be used
   * to send the N2ap message
   */
  std::map < uint16_t, Ptr<N2apIfaceInfo> > m_n2apInterfaceSockets;

  /**
   * Map the localSocket (the one receiving the N2ap message) 
   * to the corresponding (sourceCellId, targetCellId) associated with the N2ap interface
   */
  std::map < Ptr<Socket>, Ptr<N2apConnectionInfo> > m_n2apInterfaceCellIds;

  /**
   * UDP port to be used for the N2ap interfaces: N2ap
   */
  uint16_t m_n2apUdpPort;

  /**
   * Amf ID, stored as a private variable until more than one AMF will be implemented
   */
  uint16_t m_amfId; 

};


/**
 * \ingroup nr
 *
 * This entity is installed inside an eNB and provides the functionality for the N2AP interface
 */
class NgcN2apAmf : public Object
{
  friend class MemberNgcN2apSapAmfProvider<NgcN2apAmf>;

public:
  /** 
   * Constructor
   * \param the socket opened on the node in which this object is installed
   */
  NgcN2apAmf (const Ptr<Socket> n2apSocket, uint16_t amfId);

  /**
   * Destructor
   */
  virtual ~NgcN2apAmf (void);

  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


  /**
   * \param s the N2ap SAP User to be used by this NGC N2ap Amf entity in order to call methods of ngcAmfApplication
   */
  void SetNgcN2apSapAmfUser (NgcN2apSapAmf * s);

  /**
   * \return the N2ap SAP Provider interface offered by this NGC N2ap entity
   */
  NgcN2apSapAmfProvider* GetNgcN2apSapAmfProvider ();


  /**
   * Add an N2ap interface to this NGC N2ap entity
   * \param enbId the cell ID of the eNodeB which the AMF is connected to
   * \param enbAddress the address of the eNodeB which the AMF is connected to
   */
  void AddN2apInterface (uint16_t enbId, Ipv4Address enbAddress);


  /** 
   * Method to be assigned to the recv callback of the N2ap-C (N2ap Control Plane) socket.
   * It is called when the AMF receives a packet from the eNB on the N2ap interface
   * 
   * \param socket socket of the N2ap interface
   */
  void RecvFromN2apSocket (Ptr<Socket> socket);


protected:
  // Interface provided by NgcN2apSapAmfProvider
  virtual void DoSendInitialContextSetupRequest (uint64_t amfUeN2Id,
                                           uint16_t enbUeN2Id,
                                           std::list<NgcN2apSap::ErabToBeSetupItem> erabToBeSetupList,
                                           uint16_t cellId);
  //smsohn
  virtual void DoSendN2Request (uint64_t amfUeN2Id,
                                           uint16_t enbUeN2Id,
                                           std::list<NgcN2apSap::ErabToBeSetupItem> erabToBeSetupList,
                                           uint16_t cellId,
					   uint16_t cause);


  virtual void DoSendPathSwitchRequestAcknowledge (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, 
                                        std::list<NgcN2apSap::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList);

  // jhlim
  virtual void DoSendIdentityRequest (uint64_t amfUeN2Id,
  								uint16_t enbUeN2Id,
								uint16_t cellId);
  virtual void DoSendRegistrationAccept (uint64_t amfUeN2Id,
  								uint16_t enbUeN2Id, 
								uint16_t cellId,
								uint64_t guti);

  NgcN2apSapAmf* m_n2apSapUser;
  NgcN2apSapAmfProvider* m_n2apSapProvider;


private:

  /**
   * Map the enbId to the corresponding (sourceSocket, remoteIpAddr) to be used
   * to send the N2ap message
   */
  std::map < uint16_t, Ptr<N2apIfaceInfo> > m_n2apInterfaceSockets;

  /**
   * Map the localSocket (the one receiving the N2ap message) 
   * to the corresponding (sourceCellId, targetCellId) associated with the N2ap interface
   */
  std::map < Ptr<Socket>, Ptr<N2apConnectionInfo> > m_n2apInterfaceCellIds;

  /**
   * UDP port to be used for the N2ap interfaces: N2ap
   */
  uint16_t m_n2apUdpPort;

  uint16_t m_amfId; // ID of the AMF to which this N2AP endpoint is installed

  Ptr<Socket> m_localN2APSocket; // local socket to receive from the eNBs N2AP endpoints

};

} //namespace ns3

#endif // NGC_N2AP_H
