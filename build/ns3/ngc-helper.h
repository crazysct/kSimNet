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

#ifndef NGC_HELPER_H
#define NGC_HELPER_H

#include <ns3/object.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/data-rate.h>
#include <ns3/ngc-tft.h>
#include <ns3/eps-bearer.h>

namespace ns3 {

class Node;
class NetDevice;
class VirtualNetDevice;
class NgcSmfUpfApplication;
class NgcX2;
class NgcAmf;
class NgcUeNas;

/**
 * \ingroup nr
 *
 * \brief Base helper class to handle the creation of the NGC entities.
 *
 * This class provides the API for the implementation of helpers that
 * allow to create NGC entities and the nodes and interfaces that host
 * and connect them. 
 */
class NgcHelper : public Object
{
public:
  
  /** 
   * Constructor
   */
  NgcHelper ();

  /** 
   * Destructor
   */  
  virtual ~NgcHelper ();
  
  // inherited from Object
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  
  /** 
   * Add an eNB to the NGC
   * 
   * \param enbNode the previosuly created eNB node which is to be
   * added to the NGC
   * \param nrEnbNetDevice the NrEnbNetDevice of the eNB node
   * \param cellId ID of the eNB
   */
  virtual void AddEnb (Ptr<Node> enbNode, Ptr<NetDevice> nrEnbNetDevice, uint16_t cellId) = 0;

  /** 
   * Notify the NGC of the existance of a new UE which might attach at a later time
   * 
   * \param ueNrDevice the UE device to be attached
   * \param imsi the unique identifier of the UE
   */
  virtual void AddUe (Ptr<NetDevice> ueNrDevice, uint64_t imsi) = 0;

  /** 
   * Add an X2 interface between two eNB
   * 
   * \param enbNode1 one eNB peer of the X2 interface
   * \param enbNode2 the other eNB peer of the X2 interface
   */
  virtual void AddX2Interface (Ptr<Node> enbNode1, Ptr<Node> enbNode2) = 0;

  /** 
   * Activate an EPS bearer, setting up the corresponding N2-U tunnel.
   * 
   * 
   * 
   * \param ueNrDevice the Ipv4-enabled device of the UE, normally
   * connected via the NR radio interface
   * \param imsi the unique identifier of the UE
   * \param tft the Traffic Flow Template of the new bearer
   * \param bearer struct describing the characteristics of the EPS bearer to be activated
   */
  virtual uint8_t ActivateEpsBearer (Ptr<NetDevice> ueNrDevice, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer) = 0;

  /** 
   * Activate an EPS bearer, setting up the corresponding N2-U tunnel.
   * 
   * 
   * 
   * \param ueNrDevice the Ipv4-enabled device of the UE, normally
   * connected via the NR radio interface
   * \param the NAS of that device
   * \param imsi the unique identifier of the UE
   * \param tft the Traffic Flow Template of the new bearer
   * \param bearer struct describing the characteristics of the EPS bearer to be activated
   */
  virtual uint8_t ActivateEpsBearer (Ptr<NetDevice> ueNrDevice, Ptr<NgcUeNas> ueNas, uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer) = 0 ;//sjkang


  /** 
   * 
   * \return a pointer to the node implementing UPF
   * functionality. Note that in this particular implementation this
   * node will also hold the SMF functionality. The primary use
   * intended for this method is to allow the user to configure the Gi
   * interface of the UPF, i.e., to connect the UPF to the internet.
   */
  virtual Ptr<Node> GetUpfNode () = 0;

  /** 
   * Assign IPv4 addresses to UE devices
   * 
   * \param ueDevices the set of UE devices
   * 
   * \return the interface container, \see Ipv4AddressHelper::Assign() which has similar semantics
   */
  virtual Ipv4InterfaceContainer AssignUeIpv4Address (NetDeviceContainer ueDevices) = 0;


  /** 
   * 
   * \return the address of the Default Gateway to be used by UEs to reach the internet
   */
  virtual Ipv4Address GetUeDefaultGatewayAddress () = 0;


};




} // namespace ns3

#endif // NGC_HELPER_H
