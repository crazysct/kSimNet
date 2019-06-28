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
 *          Support for real N2AP link
 */

#ifndef EMU_NGC_HELPER_H
#define EMU_NGC_HELPER_H

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
class NgcUeNas;
class NgcAmfApplication;
class NgcN2apEnb;
class NgcN2apAmf;

/**
 * \ingroup nr
 *
 * \brief Create an NGC network using EmuFdNetDevice 
 *
 * This Helper will create an NGC network topology comprising of a
 * single node that implements both the SMF and UPF functionality, and
 * an AMF node. The N2-U, X2-U and X2-C interfaces are realized using
 * EmuFdNetDevice; in particular, one device is used to send all the
 * traffic related to these interfaces. 
 */
class EmuNgcHelper : public NgcHelper
{
public:
  
  /** 
   * Constructor
   */
  EmuNgcHelper ();

  /** 
   * Destructor
   */  
  virtual ~EmuNgcHelper ();
  
  // inherited from Object
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  virtual void DoInitialize ();
  virtual void DoDispose ();

  // inherited from NgcHelper
  virtual void AddEnb (Ptr<Node> enbNode, Ptr<NetDevice> nrEnbNetDevice, uint16_t cellId);
  virtual void AddUe (Ptr<NetDevice> ueNrDevice, uint64_t imsi);
  virtual void AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2);
  virtual uint8_t ActivateEpsBearer (Ptr<NetDevice> ueNrDevice, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer);
  virtual uint8_t ActivateEpsBearer (Ptr<NetDevice> ueNrDevice, Ptr<NgcUeNas> ueNas, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer);
  virtual Ptr<Node> GetUpfNode ();
  virtual Ipv4InterfaceContainer AssignUeIpv4Address (NetDeviceContainer ueDevices);
  virtual Ipv4Address GetUeDefaultGatewayAddress ();



private:

  /** 
   * helper to assign addresses to UE devices as well as to the TUN device of the SMF/UPF
   */
  Ipv4AddressHelper m_ueAddressHelper; 

  /** 
   * helper to assign addresses to N2-AP NetDevices 
   */
  Ipv4AddressHelper m_n2apIpv4AddressHelper; 


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
   * helper to assign addresses to N2-U NetDevices 
   */
  Ipv4AddressHelper m_ngcIpv4AddressHelper; 

  /**
   * UDP port where the GTP-U Socket is bound, fixed by the standard as 2152
   */
  uint16_t m_gtpuUdpPort;

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
   * Map storing for each IMSI the corresponding eNB NetDevice
   * 
   */
  std::map<uint64_t, Ptr<NetDevice> > m_imsiEnbDeviceMap;

  /**
   * Container for Ipv4Interfaces of the SMF/UPF
   */
  Ipv4InterfaceContainer m_smfIpIfaces; 

  /**
   * The name of the device used for the N2-U interface of the SMF
   */
  std::string m_smfDeviceName;

  /**
   * The name of the device used for the N2-U interface of the eNB
   */
  std::string m_enbDeviceName;

  /**
   * MAC address used for the SMF
   */
  std::string m_smfMacAddress;

  /**
   * First 5 bytes of the Enb MAC address base
   */
  std::string m_enbMacAddressBase;
};




} // namespace ns3

#endif // EMU_NGC_HELPER_H
