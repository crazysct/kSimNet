/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jnin@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 *         Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *        Support for real N2AP link
 */

#ifndef POINT_TO_POINT_NGC_HELPER_H
#define POINT_TO_POINT_NGC_HELPER_H

#include <ns3/object.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/data-rate.h>
#include <ns3/ngc-tft.h>
#include <ns3/eps-bearer.h>
#include <ns3/ngc-helper.h>

namespace ns3 {

class Node;
class NetDevice;
class VirtualNetDevice;
class NgcSmfUpfApplication;
class NgcX2;
class NgcAmf;
class NgcUeNas;
class NgcAmfApplication;
class NgcN2apEnb;
class NgcN2apAmf;

/**
 * \ingroup nr
 * \brief Create an NGC network with PointToPoint links
 *
 * This Helper will create an NGC network topology comprising of a
 * single node that implements both the SMF and UPF functionality, and
 * an AMF node. The N2-U, N2-AP, X2-U and X2-C interfaces are realized over
 * PointToPoint links. 
 */
class PointToPointNgcHelper : public NgcHelper
{
public:
  
  /** 
   * Constructor
   */
  PointToPointNgcHelper ();

  /** 
   * Destructor
   */  
  virtual ~PointToPointNgcHelper ();
  
  // inherited from Object
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  // inherited from NgcHelper
  virtual void AddEnb (Ptr<Node> enbNode, Ptr<NetDevice> nrEnbNetDevice, uint16_t cellId);
  virtual void AddUe (Ptr<NetDevice> ueNrDevice, uint64_t imsi);
  virtual void AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2);
  virtual uint8_t ActivateEpsBearer (Ptr<NetDevice> ueNrDevice, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer);
  virtual uint8_t ActivateEpsBearer (Ptr<NetDevice> ueNrDevice, Ptr<NgcUeNas> ueNas, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer);
  virtual Ptr<Node> GetUpfNode ();
  virtual Ptr<Node> GetAmfNode ();
  virtual Ipv4InterfaceContainer AssignUeIpv4Address (NetDeviceContainer ueDevices);
  virtual Ipv4Address GetUeDefaultGatewayAddress ();



private:

  /** 
   * helper to assign addresses to UE devices as well as to the TUN device of the SMF/UPF
   */
  Ipv4AddressHelper m_ueAddressHelper; 
  
  /**
   * SMF-UPF network element
   */
  Ptr<Node> m_smfUpf; 

  /**
   * SMF-UPF application
   */
  Ptr<NgcSmfUpfApplication> m_smfUpfApp;

  /**
   * TUN device implementing tunneling of user data over GTP-U/UDP/IP
   */
  Ptr<VirtualNetDevice> m_tunDevice;

  /**
   * AMF network element
   */
  Ptr<Node> m_amfNode;

  /**
   * AMF application
   */
  Ptr<NgcAmfApplication> m_amfApp;

  /**
   * N2-U interfaces
   */

  /** 
   * helper to assign addresses to N2-U NetDevices 
   */
  Ipv4AddressHelper m_n2uIpv4AddressHelper; 

  /**
   * The data rate to be used for the next N2-U link to be created
   */
  DataRate m_n2uLinkDataRate;

  /**
   * The delay to be used for the next N2-U link to be created
   */
  Time     m_n2uLinkDelay;

  /**
   * The MTU of the next N2-U link to be created. Note that,
   * because of the additional GTP/UDP/IP tunneling overhead,
   * you need a MTU larger than the end-to-end MTU that you
   * want to support.
   */
  uint16_t m_n2uLinkMtu;

  /**
   * UDP port where the GTP-U Socket is bound, fixed by the standard as 2152
   */
  uint16_t m_gtpuUdpPort;

  /**
   * Map storing for each IMSI the corresponding eNB NetDevice
   */
  std::map<uint64_t, Ptr<NetDevice> > m_imsiEnbDeviceMap;

  /**
   * N2-AP interfaces
   */

  /** 
   * helper to assign addresses to N2-AP NetDevices 
   */
  Ipv4AddressHelper m_n2apIpv4AddressHelper; 

  /**
   * The data rate to be used for the next N2-AP link to be created
   */
  DataRate m_n2apLinkDataRate;

  /**
   * The delay to be used for the next N2-AP link to be created
   */
  Time     m_n2apLinkDelay;

  /**
   * The MTU of the next N2-AP link to be created. 
   */
  uint16_t m_n2apLinkMtu;

  /**
   * UDP port where the UDP Socket is bound, fixed by the standard as 
   * 36412 (it should be sctp, but it is not supported in ns-3)
   */
  uint16_t m_n2apUdpPort;

  /**
   * Map storing for each eNB the corresponding AMF NetDevice
   */
  std::map<uint16_t, Ptr<NetDevice> > m_cellIdAmfDeviceMap;

  
  /** 
   * helper to assign addresses to X2 NetDevices 
   */
  Ipv4AddressHelper m_x2Ipv4AddressHelper;   

  /**
   * The data rate to be used for the next X2 link to be created
   */
  DataRate m_x2LinkDataRate;

  /**
   * The delay to be used for the next X2 link to be created
   */
  Time     m_x2LinkDelay;

  /**
   * The MTU of the next X2 link to be created. Note that,
   * because of some big X2 messages, you need a big MTU.
   */
  uint16_t m_x2LinkMtu;

};




} // namespace ns3

#endif // POINT_TO_POINT_NGC_HELPER_H
