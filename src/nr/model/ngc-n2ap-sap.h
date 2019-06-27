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
 * Author: Nicola Baldo <nbaldo@cttc.cat>
 * Modified by: Michele Polese <michele.polese@gmail.com>
 */

#ifndef NGC_N2AP_SAP_H
#define NGC_N2AP_SAP_H

#include <ns3/address.h>
#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/eps-bearer.h>
#include <ns3/ngc-tft.h>
#include <list>
#include <stdio.h>

namespace ns3 {

class NgcN2apSap
{
public:
  virtual ~NgcN2apSap ();

  // useful structures as defined in 3GPP ts 36.413 

  /**
   *  E-RAB Release Indication Item IEs, 3GPP TS 36.413 version 9.8.0 section 9.1.3.7
   *
   */
  struct ErabToBeReleasedIndication
  {
    uint8_t erabId;
  };

  /**
   *  E-RAB Setup Item IEs, see 3GPP TS 36.413 9.1.4.2 
   * 
   */
  struct ErabSetupItem
  {
    uint16_t    erabId;
    Ipv4Address enbTransportLayerAddress;
    uint32_t    enbTeid;    
  };


  /**
   * E-RABs Switched in Downlink Item IE, see 3GPP TS 36.413 9.1.5.8
   * 
   */
  struct ErabSwitchedInDownlinkItem
  {
    uint16_t    erabId;
    Ipv4Address enbTransportLayerAddress;
    uint32_t    enbTeid;    
  };

  struct ErabToBeSetupItem
  {
    uint8_t    erabId;
    EpsBearer   erabLevelQosParameters;
    Ipv4Address transportLayerAddress;
    uint32_t    smfTeid;    
  };

  //smsohn
  struct QoSFlowToBeSetupItem
  {
    uint8_t    erabId;
    EpsBearer   erabLevelQosParameters;
    Ipv4Address transportLayerAddress;
    uint32_t    smfTeid;    
  };


  /**
   * E-RABs Switched in Uplink Item IE, see 3GPP TS 36.413 9.1.5.9
   * 
   */
  struct ErabSwitchedInUplinkItem
  {
    uint8_t    erabId;
    Ipv4Address transportLayerAddress;
    uint32_t    enbTeid;    
  };

};


/**
 * \ingroup nr
 *
 * AMF side of the N2-AP Service Access Point (SAP) user, provides the AMF
 * methods to be called when an N2-AP message is received by the AMF N2-AP object. 
 * N2-AP ---> AMF
 */
class NgcN2apSapAmf : public NgcN2apSap
{
public:

  /** 
   * 3GPP TS 36.413 V13.1.0 section 9.1.7.1
   * \param amfUeN2Id in practice, we use the IMSI
   * \param enbUeN2Id in practice, we use the RNTI
   * \param stmsi in practice, the imsi
   * \param ecgi in practice, the cell Id
   * 
   */
 virtual void RegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t stmsi, uint16_t ecgi) = 0;

  virtual void N2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t stmsi, uint16_t ecgi) = 0;

  /**
    * \brief As per 3GPP TS 23.401 Release 9 V9.5.0 Figure 5.4.4.2-1  eNB sends indication of Bearer Release to AMF
    * \brief As per 3GPP TS version 9.8.0 section 8.2.3.2.2, the eNB initiates the procedure by sending an E-RAB RELEASE INDICATION message towards AMF
    * \param amfUeN2Id in practice, we use the IMSI
    * \param enbUeN2Id in practice, we use the RNTI
    * \param erabToBeReleaseIndication, List of bearers to be deactivated
    *
    */
  virtual void ErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeReleasedIndication> erabToBeReleaseIndication ) = 0;

  /** 
   * INITIAL CONTEXT SETUP RESPONSE message,  see 3GPP TS 36.413 9.1.4.2 
   * 
   * \param amfUeN2Id in practice, we use the IMSI
   * \param enbUeN2Id in practice, we use the RNTI
   * \param erabSetupList
   * 
   */
  virtual void InitialContextSetupResponse (uint64_t amfUeN2Id,
                                            uint16_t enbUeN2Id,
                                            std::list<ErabSetupItem> erabSetupList) = 0;




  /**
   * PATH SWITCH REQUEST message, see 3GPP TS 36.413 9.1.5.8
   * 
   */
  virtual void PathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t gci, std::list<ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList) = 0;

  // jhlim
  virtual void IdentityResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id) = 0;
  virtual void RegistrationComplete (uint64_t amfUeN2Id, uint16_t enbUeN2Id) = 0;
  //virtual void IdentityRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId) = 0;
};


/**
 * \ingroup nr
 *
 * eNB side of the N2-AP Service Access Point (SAP) provider, provides the N2-AP methods 
 * to be called when the eNB wants to send an N2-AP message
 * eNB ---> N2-AP 
 */
class NgcN2apSapEnbProvider : public NgcN2apSap
{
public: 
   
  virtual void SendRegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t stmsi, uint16_t ecgi) = 0;

  virtual void SendN2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t stmsi, uint16_t ecgi) = 0;

  virtual void SendErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeReleasedIndication> erabToBeReleaseIndication ) = 0;

  virtual void SendInitialContextSetupResponse (uint64_t amfUeN2Id,
                                            uint16_t enbUeN2Id,
                                            std::list<ErabSetupItem> erabSetupList) = 0;

  virtual void SendPathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t gci, std::list<ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList) = 0;

  // jhlim
  virtual void SendIdentityResponse (uint64_t amfUeN2Id,
  								uint16_t enbUeN2Id) = 0;
  virtual void SendRegistrationComplete (uint64_t amfUeN2Id,
  								uint16_t enbUeN2Id) = 0;

};



/**
 * \ingroup nr
 *
 * eNB side of the N2-AP Service Access Point (SAP) user, provides the eNB
 * methods to be called when an N2-AP message is received by the eNB N2-AP object.
 * N2-AP ---> eNB 
 */
class NgcN2apSapEnb : public NgcN2apSap
{
public:

  /** 
   * 
   * 3GPP TS 36.413 9.1.4.1
   * \param amfUeN2Id in practice, we use the IMSI
   * \param enbUeN2Id in practice, we use the RNTI
   * \param erabToBeSetupList
   * \param cellId, to select the enb to which the AMF wants to communicate
   * 
   */
  virtual void InitialContextSetupRequest (uint64_t amfUeN2Id,
                                           uint16_t enbUeN2Id,
                                           std::list<ErabToBeSetupItem> erabToBeSetupList) = 0;
  // jhlim
  virtual void IdentityRequest (uint64_t amfUeN2Id,
  								uint16_t enbUeN2Id) = 0;
  virtual void RegistrationAccept (uint64_t amfUeN2Id,
  								   uint16_t enbUeN2Id,
								   uint64_t guti) = 0;

  //smsohn add parameters
  virtual void N2Request (uint64_t amfUeN2Id,
                                           uint16_t enbUeN2Id,
                                           std::list<ErabToBeSetupItem> erabToBeSetupList, uint16_t cause) = 0;


  /**
   * PATH SWITCH REQUEST ACKNOWLEDGE message, see 3GPP TS 36.413 9.1.5.9
   * 
   */
  virtual void PathSwitchRequestAcknowledge (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList) = 0;


};


/**
 * \ingroup nr
 *
 * AMF side of the N2-AP Service Access Point (SAP) provider, provides the N2-AP methods 
 * to be called when the AMF wants to send an N2-AP message
 * AMF ---> N2-AP
 */
class NgcN2apSapAmfProvider : public NgcN2apSap
{
public:
  
  virtual void SendInitialContextSetupRequest (uint64_t amfUeN2Id,
                                           uint16_t enbUeN2Id,
                                           std::list<ErabToBeSetupItem> erabToBeSetupList,
                                           uint16_t cellId) = 0;

  //smsohn add parameters
  virtual void SendN2Request (uint64_t amfUeN2Id,
                                           uint16_t enbUeN2Id,
                                           std::list<ErabToBeSetupItem> erabToBeSetupList,
                                           uint16_t cellId, 
                                           uint16_t cause) = 0;



  virtual void SendPathSwitchRequestAcknowledge (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList) = 0;

  //jhlim
  virtual void SendIdentityRequest (uint64_t amfUeN2Id,
  								uint16_t enbUeN2Id,
								uint16_t cellId,
								std::string identityRequest) = 0;
  virtual void SendRegistrationAccept (uint64_t amfUeN2Id,
  								uint16_t enbUeN2Id,
								uint16_t cellId,
								uint64_t guti) = 0;
};


/**
 * Template for the implementation of the NgcN2apSapAmfUser as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNgcN2apSapAmf : public NgcN2apSapAmf
{
public:
  MemberNgcN2apSapAmf (C* owner);

  // inherited from NgcN2apSapAmf
  virtual void RegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi); 
  virtual void N2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi);
 
  virtual void ErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeReleasedIndication> erabToBeReleaseIndication );

  virtual void InitialContextSetupResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabSetupItem> erabSetupList);
  virtual void PathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList);
  // jhlim
  //virtual void IdentityRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId);
  virtual void IdentityResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id);
  virtual void RegistrationComplete (uint64_t amfUeN2Id, uint16_t enbUeN2Id);
  // jhlim
  //virtual void DoIdentityResponse (uint64_t imsi, uint16_t rnti) = 0;
  //virtual void DoIdentityRequest (uint64_t imsi, uint16_t rnti) = 0;
  //virtual void SendIdentityRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId);
  //virtual void SendIdentityResponse (uint64_t amfUeN2Id,
  				//				uint16_t enbUeN2Id);

private:
  MemberNgcN2apSapAmf ();
  C* m_owner;
};

template <class C>
MemberNgcN2apSapAmf<C>::MemberNgcN2apSapAmf (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNgcN2apSapAmf<C>::MemberNgcN2apSapAmf ()
{
}

template <class C>
void MemberNgcN2apSapAmf<C>::RegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi)
{
  m_owner->DoRegistrationRequest(amfUeN2Id, enbUeN2Id, imsi, ecgi);
}

// jhlim
template <class C>
void MemberNgcN2apSapAmf<C>::IdentityResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id)
{
  m_owner->DoIdentityResponse(amfUeN2Id, enbUeN2Id);
}
template <class C>
void MemberNgcN2apSapAmf<C>::RegistrationComplete (uint64_t amfUeN2Id, uint16_t enbUeN2Id)
{
  m_owner->DoRegistrationComplete(amfUeN2Id, enbUeN2Id);
}

template <class C>
void MemberNgcN2apSapAmf<C>::N2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi)
{
  std::cout<<"N2Message(1,2,3,4) is called" << std::endl;
  m_owner->DoN2Message (amfUeN2Id, enbUeN2Id, imsi, ecgi);
}

template <class C>
void MemberNgcN2apSapAmf<C>::ErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeReleasedIndication> erabToBeReleaseIndication)
{
  m_owner->DoErabReleaseIndication (amfUeN2Id, enbUeN2Id, erabToBeReleaseIndication);
}

template <class C>
void MemberNgcN2apSapAmf<C>::InitialContextSetupResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabSetupItem> erabSetupList)
{
  m_owner->DoInitialContextSetupResponse (amfUeN2Id, enbUeN2Id, erabSetupList);
}

template <class C>
void MemberNgcN2apSapAmf<C>::PathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
{
  m_owner->DoPathSwitchRequest (enbUeN2Id, amfUeN2Id, cgi, erabToBeSwitchedInDownlinkList);
}



/**
 * Template for the implementation of the NgcN2apSapEnbProvider as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNgcN2apSapEnbProvider : public NgcN2apSapEnbProvider
{
public:
  MemberNgcN2apSapEnbProvider (C* owner);

  // inherited from MemberNgcN2apSapEnbProvider
  virtual void SendRegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi); 
  virtual void SendN2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi);
  virtual void SendErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeReleasedIndication> erabToBeReleaseIndication );

  virtual void SendInitialContextSetupResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabSetupItem> erabSetupList);
  virtual void SendPathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList);
 //jhlim
  virtual void SendIdentityResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id);
  virtual void SendRegistrationComplete (uint64_t amfUeN2Id, uint16_t enbUeN2Id);

private:
  MemberNgcN2apSapEnbProvider ();
  C* m_owner;
};

template <class C>
MemberNgcN2apSapEnbProvider<C>::MemberNgcN2apSapEnbProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNgcN2apSapEnbProvider<C>::MemberNgcN2apSapEnbProvider ()
{
}

template <class C>
void MemberNgcN2apSapEnbProvider<C>::SendRegistrationRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi)
{
  m_owner->DoSendRegistrationRequest (amfUeN2Id, enbUeN2Id, imsi, ecgi);
}

template <class C>
void MemberNgcN2apSapEnbProvider<C>::SendN2Message (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t imsi, uint16_t ecgi)
{
 std::cout << "Hihihihihi" << std::endl;
  m_owner->DoSendN2Message (amfUeN2Id, enbUeN2Id, imsi, ecgi);
}

template <class C>
void MemberNgcN2apSapEnbProvider<C>::SendErabReleaseIndication (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeReleasedIndication> erabToBeReleaseIndication)
{
  m_owner->DoSendErabReleaseIndication (amfUeN2Id, enbUeN2Id, erabToBeReleaseIndication);
}

template <class C>
void MemberNgcN2apSapEnbProvider<C>::SendInitialContextSetupResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabSetupItem> erabSetupList)
{
  m_owner->DoSendInitialContextSetupResponse (amfUeN2Id, enbUeN2Id, erabSetupList);
}
// jhlim
template <class C>
void MemberNgcN2apSapEnbProvider<C>::SendIdentityResponse (uint64_t amfUeN2Id, uint16_t enbUeN2Id)
{
  m_owner->DoSendIdentityResponse (amfUeN2Id, enbUeN2Id);
}
template <class C>
void MemberNgcN2apSapEnbProvider<C>::SendRegistrationComplete (uint64_t amfUeN2Id, uint16_t enbUeN2Id)
{
  m_owner->DoSendRegistrationComplete (amfUeN2Id, enbUeN2Id);
}

template <class C>
void MemberNgcN2apSapEnbProvider<C>::SendPathSwitchRequest (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList)
{
  m_owner->DoSendPathSwitchRequest (enbUeN2Id, amfUeN2Id, cgi, erabToBeSwitchedInDownlinkList);
}



/**
 * Template for the implementation of the NgcN2apSapEnb as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNgcN2apSapEnb : public NgcN2apSapEnb
{
public:
  MemberNgcN2apSapEnb (C* owner);

  // inherited from NgcN2apSapEnb
  virtual void InitialContextSetupRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeSetupItem> erabToBeSetupList);
 
  //smsohn add parameters 
  virtual void N2Request (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeSetupItem> erabToBeSetupList, uint16_t cause);
  
  virtual void PathSwitchRequestAcknowledge (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList);
  // jhlim
  virtual void IdentityRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id);
  virtual void RegistrationAccept (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t guti);

private:
  MemberNgcN2apSapEnb ();
  C* m_owner;
};

template <class C>
MemberNgcN2apSapEnb<C>::MemberNgcN2apSapEnb (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNgcN2apSapEnb<C>::MemberNgcN2apSapEnb ()
{
}

template <class C>
void MemberNgcN2apSapEnb<C>::InitialContextSetupRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeSetupItem> erabToBeSetupList)
{
  m_owner->DoInitialContextSetupRequest (amfUeN2Id, enbUeN2Id, erabToBeSetupList);
}
// jhlim
template <class C>
void MemberNgcN2apSapEnb<C>::IdentityRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id)
{
  m_owner->DoIdentityRequest (amfUeN2Id, enbUeN2Id);
}
template <class C>
void MemberNgcN2apSapEnb<C>::RegistrationAccept (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint64_t guti)
{
  m_owner->DoRegistrationAccept (amfUeN2Id, enbUeN2Id, guti);
}

//smsohn add parameters
template <class C>
void MemberNgcN2apSapEnb<C>::N2Request (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeSetupItem> erabToBeSetupList, uint16_t cause)
{
  m_owner->DoN2Request (amfUeN2Id, enbUeN2Id, erabToBeSetupList, cause);
}



template <class C>
void MemberNgcN2apSapEnb<C>::PathSwitchRequestAcknowledge (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList)
{
  m_owner->DoPathSwitchRequestAcknowledge (enbUeN2Id, amfUeN2Id, cgi, erabToBeSwitchedInUplinkList);
}


/**
 * Template for the implementation of the NgcN2apSapAmfProvider as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNgcN2apSapAmfProvider : public NgcN2apSapAmfProvider
{
public:
  MemberNgcN2apSapAmfProvider (C* owner);

  // inherited from NgcN2apSapAmfProvider
  virtual void SendInitialContextSetupRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeSetupItem> erabToBeSetupList, uint16_t cellId);
  
  //smsohn add parameters
  virtual void SendN2Request (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeSetupItem> erabToBeSetupList, uint16_t cellId, uint16_t cause);
 
  virtual void SendPathSwitchRequestAcknowledge (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList);
  // jhlim
  virtual void SendIdentityRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId, std::string identityRequest);
  virtual void SendRegistrationAccept (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId, uint64_t guti);
  

private:
  MemberNgcN2apSapAmfProvider ();
  C* m_owner;
};

template <class C>
MemberNgcN2apSapAmfProvider<C>::MemberNgcN2apSapAmfProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNgcN2apSapAmfProvider<C>::MemberNgcN2apSapAmfProvider ()
{
}

template <class C>
void MemberNgcN2apSapAmfProvider<C>::SendInitialContextSetupRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeSetupItem> erabToBeSetupList, uint16_t cellId)
{
  m_owner->DoSendInitialContextSetupRequest (amfUeN2Id, enbUeN2Id, erabToBeSetupList, cellId);
}
// jhliim
template <class C>
void MemberNgcN2apSapAmfProvider<C>::SendIdentityRequest (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId, std::string identityRequest)
{
  m_owner->DoSendIdentityRequest (amfUeN2Id, enbUeN2Id, cellId);
}
template <class C>
void MemberNgcN2apSapAmfProvider<C>::SendRegistrationAccept (uint64_t amfUeN2Id, uint16_t enbUeN2Id, uint16_t cellId, uint64_t guti)
{
  //m_owner->DoSendIdentityRequest (amfUeN2Id, enbUeN2Id, cellId);
  m_owner->DoSendRegistrationAccept (amfUeN2Id, enbUeN2Id, cellId, guti);
}


//smsohn add parameters
template <class C>
void MemberNgcN2apSapAmfProvider<C>::SendN2Request (uint64_t amfUeN2Id, uint16_t enbUeN2Id, std::list<ErabToBeSetupItem> erabToBeSetupList, uint16_t cellId, uint16_t cause)
{
  m_owner->DoSendN2Request (amfUeN2Id, enbUeN2Id, erabToBeSetupList, cellId, cause);
}

template <class C>
void MemberNgcN2apSapAmfProvider<C>::SendPathSwitchRequestAcknowledge (uint64_t enbUeN2Id, uint64_t amfUeN2Id, uint16_t cgi, std::list<ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList)
{
  m_owner->DoSendPathSwitchRequestAcknowledge (enbUeN2Id, amfUeN2Id, cgi, erabToBeSwitchedInUplinkList);
}


} //namespace ns3

#endif /* NGC_N2AP_SAP_H */

