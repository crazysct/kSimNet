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
 * Author: Nicola Baldo <nbaldo@cttc.es> wrote the NgcAmf class
 * Author: Michele Polese <michele.polese@gmail.com> wrote the Application version
 */

#ifndef NGC_AMF_APPLICATION_H
#define NGC_AMF_APPLICATION_H

#include <ns3/object.h>
#include <ns3/ngc-n2ap-sap.h>
#include <ns3/ngc-n11-sap.h>
#include <ns3/application.h>


#include <map>
#include <list>

namespace ns3 {

class Node;
class NetDevice;

/**
 * \brief This object implements as an application the AMF functionality.
 *
 */
class NgcAmfApplication : public Application
{

  friend class MemberNgcN2apSapAmf<NgcAmfApplication>;
  friend class MemberNgcN11SapAmf<NgcAmfApplication>;
  
public:
  
  /** 
   * Constructor
   */
  NgcAmfApplication ();

  /** 
   * Destructor
   */  
  virtual ~NgcAmfApplication ();
  
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
   * \param the AMF provider, given by the N2AP object associated to this application
   */
  void SetN2apSapAmfProvider(NgcN2apSapAmfProvider* provider);

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
   * \param the eNB N2UAddr 
   */
  void AddEnb (uint16_t ecgi, Ipv4Address enbN2UAddr);
  
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


private:

  // N2-AP SAP AMF forwarded methods
  void DoRegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi);
  void DoInitialUeMessage (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi);
  void DoN2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi);
  void DoInitialContextSetupResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapAmf::ErabSetupItem> erabSetupList);
  void DoPathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<NgcN2apSapAmf::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList);
  void DoErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<NgcN2apSapAmf::ErabToBeReleasedIndication> erabToBeReleaseIndication);

  // N11 SAP AMF forwarded methods
  void DoModifyBearerResponse (NgcN11SapAmf::ModifyBearerResponseMessage msg);
  void DoUpdateSMContextResponse (NgcN11SapAmf::UpdateSMContextResponseMessage msg);
  void DoCreateSessionResponse (NgcN11SapAmf::CreateSessionResponseMessage msg);
  void DoDeleteBearerRequest (NgcN11SapAmf::DeleteBearerRequestMessage msg);

  // jhlim
  void SendNamfCommunicationUeContextTransfer(uint64_t imsi);
  void DoNamfCommunicationUeContextTransfer(uint64_t imsi);
  void SendNamfCommunicationUeContextTransferResponse(uint64_t imsi, uint64_t context);
  void DoNamfCommunicationUeContextTransferResponse(uint64_t imsi, uint64_t context);
  void SendNamfCommunicationRegistrationCompleteNotify(uint64_t imsi);
  bool IsGuti(uint64_t imsi);
  void DoRegistrationComplete(uint64_t amfUeN2Id, uint16_t enbUeN2Id);
  void DoIdentityResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id);

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

  //yjshin
  struct FlowInfo
  {
    Ptr<NgcTft> tft;
    EpsBearer flow; //Todo
    uint8_t flowId;
  };
  
  /**
   * Hold info on a UE
   * 
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
   * Hold info on a ENB
   * 
   */
  struct EnbInfo : public SimpleRefCount<EnbInfo>
  {
    uint16_t gci;
    Ipv4Address n2uAddr;
  };

  /**
   * EnbInfo stored by EGCI
   * 
   */
  std::map<uint16_t, Ptr<EnbInfo> > m_enbInfoMap;


  

  NgcN2apSapAmf* m_n2apSapAmf;
  NgcN2apSapAmfProvider* m_n2apSapAmfProvider;

  NgcN11SapAmf* m_n11SapAmf;
  NgcN11SapSmf* m_n11SapSmf;
  
};




} // namespace ns3

#endif // NGC_AMF_APPLICATION_H
