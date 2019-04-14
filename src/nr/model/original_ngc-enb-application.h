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
 * Author: Jaume Nin <jnin@cttc.cat>
 *         Nicola Baldo <nbaldo@cttc.cat>
 */

#ifndef ORIGINAL_NGC_ENB_APPLICATION_H
#define ORIGINAL_NGC_ENB_APPLICATION_H

#include <ns3/address.h>
#include <ns3/socket.h>
#include <ns3/original_virtual-net-device.h>
#include <ns3/traced-callback.h>
#include <ns3/callback.h>
#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/nr-common.h>
#include <ns3/application.h>
#include <ns3/eps-bearer.h>
#include <ns3/ngc-enb-n2-sap.h>
#include <ns3/ngc-n2ap-sap.h>
#include <map>

namespace ns3 {
class NgcEnbN2SapUser;
class NgcEnbN2SapProvider;


/**
 * \ingroup nr
 *
 * This application is installed inside eNBs and provides the bridge functionality for user data plane packets between the radio interface and the N2-U interface.
 */
class NgcEnbApplication1 : public Application
{

  friend class MemberNgcEnbN2SapProvider<NgcEnbApplication1>;
  friend class MemberNgcN2apSapEnb<NgcEnbApplication1>;


  // inherited from Object
public:
  static TypeId GetTypeId (void);
protected:
  void DoDispose (void);

public:
  
  

  /** 
   * Constructor
   * 
   * \param nrSocket the socket to be used to send/receive packets to/from the NR radio interface
   * \param n2uSocket the socket to be used to send/receive packets
   * to/from the N2-U interface connected with the SMF 
   * \param enbN2uAddress the IPv4 address of the N2-U interface of this eNB
   * \param smfN2uAddress the IPv4 address at which this eNB will be able to reach its SMF for N2-U communications
   * \param cellId the identifier of the enb
   */
  NgcEnbApplication1 (Ptr<Socket> nrSocket, Ptr<Socket> n2uSocket, Ipv4Address enbN2uAddress, Ipv4Address smfN2uAddress, uint16_t cellId);

  /**
   * Destructor
   * 
   */
  virtual ~NgcEnbApplication1 (void);


  /** 
   * Set the N2 SAP User
   * 
   * \param s the N2 SAP User
   */
  void SetN2SapUser (NgcEnbN2SapUser * s);

  /** 
   * 
   * \return the N2 SAP Provider
   */
  NgcEnbN2SapProvider* GetN2SapProvider ();

  /** 
   * Set the AMF side of the N2-AP SAP 
   * 
   * \param s the AMF side of the N2-AP SAP 
   */
  void SetN2apSapAmf (NgcN2apSapAmf * s);

  /** 
   * 
   * \return the ENB side of the N2-AP SAP 
   */
  NgcN2apSapEnb* GetN2apSapEnb ();
 
  /** 
   * Method to be assigned to the recv callback of the NR socket. It is called when the eNB receives a data packet from the radio interface that is to be forwarded to the SMF.
   * 
   * \param socket pointer to the NR socket
   */
  void RecvFromNrSocket (Ptr<Socket> socket);


  /** 
   * Method to be assigned to the recv callback of the N2-U socket. It is called when the eNB receives a data packet from the SMF that is to be forwarded to the UE.
   * 
   * \param socket pointer to the N2-U socket
   */
  void RecvFromN2uSocket (Ptr<Socket> socket);


  struct EpsFlowId_t
  {
    uint16_t  m_rnti;
    uint8_t   m_bid;

  public:
    EpsFlowId_t ();
    EpsFlowId_t (const uint16_t a, const uint8_t b);

    friend bool operator == (const EpsFlowId_t &a, const EpsFlowId_t &b);
    friend bool operator < (const EpsFlowId_t &a, const EpsFlowId_t &b);
  };


private:

  // ENB N2 SAP provider methods
  void DoInitialUeMessage (uint64_t imsi, uint16_t rnti);
  void DoN2Message (uint64_t imsi, uint16_t rnti);
  void DoPathSwitchRequest (NgcEnbN2SapProvider::PathSwitchRequestParameters params);
  void DoUeContextRelease (uint16_t rnti);
  
  // N2-AP SAP ENB methods
  void DoInitialContextSetupRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapEnb::ErabToBeSetupItem> erabToBeSetupList);
  //smsohn
  void DoN2Request (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapEnb::ErabToBeSetupItem> erabToBeSetupList);
  

  void DoPathSwitchRequestAcknowledge (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<NgcN2apSapEnb::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList);

  /** 
   * \brief This function accepts bearer id corresponding to a particular UE and schedules indication of bearer release towards AMF
   * \param imsi maps to amfUeN2Id
   * \param rnti maps to enbUeN2Id
   * \param bearerId Bearer Identity which is to be de-activated
   */
  void DoReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId);


  /**
   * Send a packet to the UE via the NR radio interface of the eNB
   * 
   * \param packet t
   * \param bid the EPS Bearer IDentifier
   */
  void SendToNrSocket (Ptr<Packet> packet, uint16_t rnti, uint8_t bid);


  /** 
   * Send a packet to the SMF via the N2-U interface
   * 
   * \param packet packet to be sent
   * \param teid the Tunnel Enpoint IDentifier
   */
  void SendToN2uSocket (Ptr<Packet> packet, uint32_t teid);


  
  /** 
   * internal method used for the actual setup of the N2 Bearer
   * 
   * \param teid 
   * \param rnti 
   * \param bid 
   */
  void SetupN2Bearer (uint32_t teid, uint16_t rnti, uint8_t bid);

  /**
   * raw packet socket to send and receive the packets to and from the NR radio interface
   */
  Ptr<Socket> m_nrSocket;

  /**
   * UDP socket to send and receive GTP-U the packets to and from the N2-U interface
   */
  Ptr<Socket> m_n2uSocket;

  /**
   * address of the eNB for N2-U communications
   */
  Ipv4Address m_enbN2uAddress;

  /**
   * address of the SMF which terminates all N2-U tunnels
   */
  Ipv4Address m_smfN2uAddress;

  /**
   * map of maps telling for each RNTI and BID the corresponding  N2-U TEID
   * 
   */
  std::map<uint16_t, std::map<uint8_t, uint32_t> > m_rbidTeidMap;  

  /**
   * map telling for each N2-U TEID the corresponding RNTI,BID
   * 
   */
  std::map<uint32_t, EpsFlowId_t> m_teidRbidMap;
 
  /**
   * UDP port to be used for GTP
   */
  uint16_t m_gtpuUdpPort;

  /**
   * Provider for the N2 SAP 
   */
  NgcEnbN2SapProvider* m_n2SapProvider;

  /**
   * User for the N2 SAP 
   */
  NgcEnbN2SapUser* m_n2SapUser;

  /**
   * AMF side of the N2-AP SAP
   * 
   */
  NgcN2apSapAmf* m_n2apSapAmf;

  /**
   * ENB side of the N2-AP SAP
   * 
   */
  NgcN2apSapEnb* m_n2apSapEnb;

  /**
   * UE context info
   * 
   */
  std::map<uint64_t, uint16_t> m_imsiRntiMap;

  uint16_t m_cellId;

};

} //namespace ns3

#endif /* NGC_ENB_APPLICATION_H */

