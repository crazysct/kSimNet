/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 */

#ifndef NGC_AMF_H
#define NGC_AMF_H

#include <ns3/object.h>
#include <ns3/ngc-n2ap-sap.h>
#include <ns3/ngc-n11-sap.h>

#include <map>
#include <list>

namespace ns3 {

class Node;
class NetDevice;

/**
 * \brief This object implements the AMF functionality.
 *
 */
class NgcAmf : public Object
{

  friend class MemberNgcN2apSapAmf<NgcAmf>;
  friend class MemberNgcN11SapAmf<NgcAmf>;
  
public:
  
  /** 
   * Constructor
   */
  NgcAmf ();

  /** 
   * Destructor
   */  
  virtual ~NgcAmf ();
  
  // inherited from Object  
  static TypeId GetTypeId (void);
protected:
  virtual void DoDispose ();

public:


  /** 
   * 
   * \return the AMF side of the N2-AP SAP 
   */
  NgcN2apSapAmf* GetN2apSapAmf ();

  /** 
   * Set the SMF side of the N11 SAP 
   * 
   * \param s the SMF side of the N11 SAP 
   */
  void SetN11SapSmf (NgcN11SapSmf * s);

  /** 
   * 
   * \return the AMF side of the N11 SAP 
   */
  NgcN11SapAmf* GetN11SapAmf ();

  /** 
   * Add a new ENB to the AMF. 
   * \param ecgi E-UTRAN Cell Global ID, the unique identifier of the eNodeB
   * \param enbN2apSap the ENB side of the N2-AP SAP 
   */
  void AddEnb (uint16_t ecgi, Ipv4Address enbN2UAddr, NgcN2apSapEnb* enbN2apSap);
  
  /** 
   * Add a new UE to the AMF. This is the equivalent of storing the UE
   * credentials before the UE is ever turned on. 
   * 
   * \param imsi the unique identifier of the UE
   */
  void AddUe (uint64_t imsi);

  /** 
   * Add an EPS bearer to the list of bearers to be activated for this
   * UE. The bearer will be activated when the UE enters the ECM
   * connected state.
   * 
   * \param imsi UE identifier
   * \param tft traffic flow template of the bearer
   * \param bearer QoS characteristics of the bearer
   */

  uint8_t AddBearer (uint64_t imsi, Ptr<NgcTft> tft, EpsBearer bearer);
  
  //smsohn 
  uint8_t AddFlow (uint64_t imsi, Ptr<NgcTft> tft, EpsBearer flow);

private:

  // N2-AP SAP AMF forwarded methods
  void DoRegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi); 
  void DoN2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi);
  void DoInitialContextSetupResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapAmf::ErabSetupItem> erabSetupList);
  void DoPathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<NgcN2apSapAmf::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList);
  void DoErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapAmf::ErabToBeReleasedIndication> erabToBeReleaseIndication);

  // N11 SAP AMF forwarded methods
  void DoModifyBearerResponse (NgcN11SapAmf::ModifyBearerResponseMessage msg);
  	
  //smsohn
  void DoUpdateSMContextResponse (NgcN11SapAmf::UpdateSMContextResponseMessage msg);
 
  void DoCreateSessionResponse (NgcN11SapAmf::CreateSessionResponseMessage msg);
  void DoDeleteBearerRequest (NgcN11SapAmf::DeleteBearerRequestMessage msg);

  // jhlim
  void DoIdentityRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId);
  void DoIdentityResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id);
  void DoRegistrationAccept (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId, uint64_t guti);
  void DoRegistrationComplete (uint64_t amfUeN2Id, uint16_t enbUeN2Id);

  // hmlee
  void DoNsmfPDUSessionUpdateSMContext();
  void DoNsmfPDUSessionReleaseSMContext();

  /**
   * Hold info on an EPS bearer to be activated
   * 
   */
  struct BearerInfo
  {
    Ptr<NgcTft> tft;
    EpsBearer bearer;
    uint8_t bearerId;
  };
 
  /**
   * Hold info on an Qos Flow to be activated
   * smsohn
   */
  struct FlowInfo
  {
    Ptr<NgcTft> tft;
    EpsBearer flow; //Todo
    uint8_t flowId;
  }; 

  /**
   * Hold info on a UE
   * smsohn TODO 
   */
  struct UeInfo : public SimpleRefCount<UeInfo>
  {
    uint64_t amfUeN2Id;
    uint16_t enbUeN2Id;
    uint64_t imsi;
    uint16_t cellId;
    std::list<BearerInfo> bearersToBeActivated;
    std::list<FlowInfo> flowsToBeActivated;
    uint16_t bearerCounter;
  };
  /**
   * UeInfo stored by IMSI
   * 
   */  
  std::map<uint64_t, Ptr<UeInfo> > m_ueInfoMap;

  /**
   * \brief This Function erases all contexts of bearer from AMF side
   * \param ueInfo UE information pointer
   * \param epsBearerId Bearer Id which need to be removed corresponding to UE
   */
  void RemoveBearer (Ptr<UeInfo> ueInfo, uint8_t epsBearerId);

  /**
   * \brief This Function erases all contexts of flow from AMF side
   * \param ueInfo UE information pointer
   * \param qfi flow Id which need to be removed corresponding to UE
   */
  void RemoveFlow (Ptr<UeInfo> ueInfo, uint8_t qfi);


  /**
   * Hold info on a ENB
   * 
   */
  struct EnbInfo : public SimpleRefCount<EnbInfo>
  {
    uint16_t gci;
    Ipv4Address n2uAddr;
    NgcN2apSapEnb* n2apSapEnb;
  };

  /**
   * EnbInfo stored by EGCI
   * 
   */
  std::map<uint16_t, Ptr<EnbInfo> > m_enbInfoMap;


  

  NgcN2apSapAmf* m_n2apSapAmf;

  NgcN11SapAmf* m_n11SapAmf;
  NgcN11SapSmf* m_n11SapSmf;
  
};




} // namespace ns3

#endif // NGC_AMF_H
