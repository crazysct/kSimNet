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

#ifndef NGC_SMF_UPF_APPLICATION_H
#define NGC_SMF_UPF_APPLICATION_H

#include <ns3/address.h>
#include <ns3/socket.h>
#include <ns3/virtual-net-device.h>
#include <ns3/traced-callback.h>
#include <ns3/callback.h>
#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/eps-bearer.h>
#include <ns3/ngc-tft.h>
#include <ns3/ngc-tft-classifier.h>
#include <ns3/nr-common.h>
#include <ns3/application.h>
#include <ns3/ngc-n2ap-sap.h>
#include <ns3/ngc-n11-sap.h>
#include <map>

namespace ns3 {

/**
 * \ingroup nr
 *
 * This application implements the SMF/UPF functionality.
 */
class NgcSmfUpfApplication : public Application
{
  friend class MemberNgcN11SapSmf<NgcSmfUpfApplication>;

public:

  // inherited from Object
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   * Constructor that binds the tap device to the callback methods.
   *
   * \param tunDevice TUN VirtualNetDevice used to tunnel IP packets from
   * the Gi interface of the UPF/SMF over the
   * internet over GTP-U/UDP/IP on the N2-U interface
   * \param n2uSocket socket used to send GTP-U packets to the eNBs
   */
  NgcSmfUpfApplication (const Ptr<VirtualNetDevice> tunDevice, const Ptr<Socket> n2uSocket);

  /** 
   * Destructor
   */
  virtual ~NgcSmfUpfApplication (void);
  
  /** 
   * Method to be assigned to the callback of the Gi TUN VirtualNetDevice. It
   * is called when the SMF/UPF receives a data packet from the
   * internet (including IP headers) that is to be sent to the UE via
   * its associated eNB, tunneling IP over GTP-U/UDP/IP.
   * 
   * \param packet 
   * \param source 
   * \param dest 
   * \param protocolNumber 
   * \return true always 
   */
  bool RecvFromTunDevice (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);


  /** 
   * Method to be assigned to the recv callback of the N2-U socket. It
   * is called when the SMF/UPF receives a data packet from the eNB
   * that is to be forwarded to the internet. 
   * 
   * \param socket pointer to the N2-U socket
   */
  void RecvFromN2uSocket (Ptr<Socket> socket);

  /** 
   * Send a packet to the internet via the Gi interface of the SMF/UPF
   * 
   * \param packet 
   */
  void SendToTunDevice (Ptr<Packet> packet, uint32_t teid);


  /** 
   * Send a packet to the SMF via the N2-U interface
   * 
   * \param packet packet to be sent
   * \param enbN2uAddress the address of the eNB
   * \param teid the Tunnel Enpoint IDentifier
   */
  void SendToN2uSocket (Ptr<Packet> packet, Ipv4Address enbN2uAddress, uint32_t teid);
  

  /** 
   * Set the AMF side of the N11 SAP 
   * 
   * \param s the AMF side of the N11 SAP 
   */
  void SetN11SapAmf (NgcN11SapAmf * s);

  /** 
   * 
   * \return the SMF side of the N11 SAP 
   */
  NgcN11SapSmf* GetN11SapSmf ();


  /** 
   * Let the SMF be aware of a new eNB 
   * 
   * \param cellId the cell identifier
   * \param enbAddr the address of the eNB
   * \param smfAddr the address of the SMF
   */
  void AddEnb (uint16_t cellId, Ipv4Address enbAddr, Ipv4Address smfAddr);

  /** 
   * Let the SMF be aware of a new UE
   * 
   * \param imsi the unique identifier of the UE
   */
  void AddUe (uint64_t imsi);

  /** 
   * set the address of a previously added UE
   * 
   * \param imsi the unique identifier of the UE
   * \param ueAddr the IPv4 address of the UE
   */
  void SetUeAddress (uint64_t imsi, Ipv4Address ueAddr);

private:

  // N11 SAP SMF methods
  void DoCreateSessionRequest (NgcN11SapSmf::CreateSessionRequestMessage msg);
  void DoUpdateSMContextRequest (NgcN11SapSmf::UpdateSMContextRequestMessage msg);
  void DoModifyBearerRequest (NgcN11SapSmf::ModifyBearerRequestMessage msg);  

  void DoDeleteBearerCommand (NgcN11SapSmf::DeleteBearerCommandMessage req);
  void DoDeleteBearerResponse (NgcN11SapSmf::DeleteBearerResponseMessage req);


  /**
   * store info for each UE connected to this SMF
   */
  class UeInfo : public SimpleRefCount<UeInfo>
  {
public:
    UeInfo ();  

    /** 
     * 
     * \param tft the Traffic Flow Template of the new bearer to be added
     * \param epsBearerId the ID of the EPS Bearer to be activated
     * \param teid  the TEID of the new bearer
     */
    void AddBearer (Ptr<NgcTft> tft, uint8_t epsBearerId, uint32_t teid);

    /** 
     * \brief Function, deletes contexts of bearer on SMF and UPF side
     * \param bearerId, the Bearer Id whose contexts to be removed
     */
    void RemoveBearer (uint8_t bearerId);

    /**
     * 
     * 
     * \param p the IP packet from the internet to be classified
     * 
     * \return the corresponding bearer ID > 0 identifying the bearer
     * among all the bearers of this UE;  returns 0 if no bearers
     * matches with the previously declared TFTs
     */
    uint32_t Classify (Ptr<Packet> p);

    /** 
     * \return the address of the eNB to which the UE is connected
     */
    Ipv4Address GetEnbAddr ();

    /** 
     * set the address of the eNB to which the UE is connected
     * 
     * \param addr the address of the eNB
     */
    void SetEnbAddr (Ipv4Address addr);

    /** 
     * \return the address of the UE
     */
    Ipv4Address GetUeAddr ();

    /** 
     * set the address of the UE
     * 
     * \param addr the address of the UE
     */
    void SetUeAddr (Ipv4Address addr);


  private:
    NgcTftClassifier m_tftClassifier;
    Ipv4Address m_enbAddr;
    Ipv4Address m_ueAddr;
    std::map<uint8_t, uint32_t> m_teidByBearerIdMap;
  };


 /**
  * UDP socket to send and receive GTP-U packets to and from the N2-U interface
  */
  Ptr<Socket> m_n2uSocket;
  
  /**
   * TUN VirtualNetDevice used for tunneling/detunneling IP packets
   * from/to the internet over GTP-U/UDP/IP on the N2 interface 
   */
  Ptr<VirtualNetDevice> m_tunDevice;

  /**
   * Map telling for each UE address the corresponding UE info 
   */
  std::map<Ipv4Address, Ptr<UeInfo> > m_ueInfoByAddrMap;

  /**
   * Map telling for each IMSI the corresponding UE info 
   */
  std::map<uint64_t, Ptr<UeInfo> > m_ueInfoByImsiMap;

  /**
   * UDP port to be used for GTP
   */
  uint16_t m_gtpuUdpPort;

  uint32_t m_teidCount;

  /**
   * AMF side of the N11 SAP
   * 
   */
  NgcN11SapAmf* m_n11SapAmf;

  /**
   * SMF side of the N11 SAP
   * 
   */
  NgcN11SapSmf* m_n11SapSmf;

  struct EnbInfo
  {
    Ipv4Address enbAddr;
    Ipv4Address smfAddr;    
  };

  std::map<uint16_t, EnbInfo> m_enbInfoByCellId;
};

} //namespace ns3

#endif /* NGC_SMF_UPF_APPLICATION_H */

