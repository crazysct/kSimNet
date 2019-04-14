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

#ifndef NGC_ENB_N2_SAP_H
#define NGC_ENB_N2_SAP_H

#include <list>
#include <stdint.h>
#include <ns3/eps-bearer.h>
#include <ns3/ipv4-address.h>

using namespace std;

namespace ns3 {


/**
 * This class implements the Service Access Point (SAP) between the
 * NrEnbRrc and the NgcEnbApplication. In particular, this class implements the
 * Provider part of the SAP, i.e., the methods exported by the
 * NgcEnbApplication and called by the NrEnbRrc.
 * 
 */
class NgcEnbN2SapProvider
{
public:
  virtual ~NgcEnbN2SapProvider ();  

  /** 
   * 
   * 
   * \param imsi 
   * \param rnti 
   */

  virtual void RegistrationRequest (uint64_t imsi, uint16_t rnti) = 0;

  //jhlim
  virtual void IdentityResponse (uint64_t imsi, uint16_t rnti) = 0;
  virtual void RegistrationComplete (uint64_t imsi, uint16_t rnti) = 0; 
  
  virtual void N2Message (uint64_t imsi, uint16_t rnti) = 0;
  virtual void N2Message (uint64_t imsi, uint16_t rnti, int dummy) = 0; // jhlim for N2apSapAmf

  /**
   *  \brief Triggers ngc-enb-application to send ERAB Release Indication message towards AMF
   *  \param imsi the UE IMSI
   *  \param rnti the UE RNTI
   *  \param bearerId Bearer Identity which is to be de-activated
   */
  virtual void DoSendReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId) = 0;
  

  struct BearerToBeSwitched
  {
    uint8_t epsBearerId;
    uint32_t teid;
  };
  
  struct PathSwitchRequestParameters
  {
    uint16_t rnti;
    uint16_t cellId;
    uint32_t amfUeN2Id;
    std::list<BearerToBeSwitched> bearersToBeSwitched;
  };

  virtual void PathSwitchRequest (PathSwitchRequestParameters params) = 0;

  /** 
   * release UE context at the N2 Application of the source eNB after
   * reception of the UE CONTEXT RELEASE X2 message from the target eNB
   * during X2-based handover 
   * 
   * \param rnti 
   */
  virtual void UeContextRelease (uint16_t rnti) = 0;
    
};
  


/**
 * This class implements the Service Access Point (SAP) between the
 * NrEnbRrc and the NgcEnbApplication. In particular, this class implements the
 * User part of the SAP, i.e., the methods exported by the NrEnbRrc
 * and called by the NgcEnbApplication.
 * 
 */
class NgcEnbN2SapUser
{
public:
  virtual ~NgcEnbN2SapUser ();
  
  /**
   * Parameters passed to DataRadioBearerSetupRequest ()
   * 
   */
  struct DataRadioBearerSetupRequestParameters
  {
    uint16_t rnti;   /**< the RNTI identifying the UE for which the
			DataRadioBearer is to be created */ 
    EpsBearer bearer; /**< the characteristics of the bearer to be setup */
    EpsBearer flow; //smsohn added

    uint8_t bearerId; /**< the EPS Bearer Identifier */
    uint8_t flowId; //smsohn added 

    uint32_t    gtpTeid; /**< N2-bearer GTP tunnel endpoint identifier, see 36.423 9.2.1 */
    Ipv4Address transportLayerAddress; /**< IP Address of the SMF, see 36.423 9.2.1 */
  };

  // jhlim
  struct IdentityRequestParameters
  {
	uint16_t rnti;
  };
  struct RegistrationAcceptParameters
  {
    uint16_t rnti;
	uint64_t guti;
  };

  /**
   * request the setup of a DataRadioBearer
   * 
   */
  virtual void DataRadioBearerSetupRequest (DataRadioBearerSetupRequestParameters params) = 0;

  //jhlim
  virtual void IdentityRequest (IdentityRequestParameters params) = 0;
  virtual void RegistrationAccept (RegistrationAcceptParameters params) = 0;

  
  struct PathSwitchRequestAcknowledgeParameters
  {
    uint16_t rnti;
  };

  virtual void PathSwitchRequestAcknowledge (PathSwitchRequestAcknowledgeParameters params) = 0;
  
};
  



/**
 * Template for the implementation of the NgcEnbN2SapProvider as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNgcEnbN2SapProvider : public NgcEnbN2SapProvider
{
public:
  MemberNgcEnbN2SapProvider (C* owner);

  // inherited from NgcEnbN2SapProvider
  virtual void RegistrationRequest (uint64_t imsi, uint16_t rnti);
  // jhlim
  virtual void IdentityResponse (uint64_t imsi, uint16_t rnti);
  virtual void RegistrationComplete (uint64_t imsi, uint16_t rnti);
 
  virtual void N2Message (uint64_t imsi, uint16_t rnti);
  virtual void N2Message (uint64_t imsi, uint16_t rnti, int dummy);

  virtual void DoSendReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId);

  virtual void PathSwitchRequest (PathSwitchRequestParameters params);
  virtual void UeContextRelease (uint16_t rnti);

private:
  MemberNgcEnbN2SapProvider ();
  C* m_owner;
};

template <class C>
MemberNgcEnbN2SapProvider<C>::MemberNgcEnbN2SapProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNgcEnbN2SapProvider<C>::MemberNgcEnbN2SapProvider ()
{
}


template <class C>
void MemberNgcEnbN2SapProvider<C>::RegistrationRequest (uint64_t imsi, uint16_t rnti)
{
  cout<<"DoRegistrationRequest for Provider is called" << endl;
  m_owner->DoRegistrationRequest (imsi, rnti);
}
// jhlim
template <class C>
void MemberNgcEnbN2SapProvider<C>::IdentityResponse (uint64_t imsi, uint16_t rnti)
{
  m_owner->DoIdentityResponse (imsi, rnti);
}
template <class C>
void MemberNgcEnbN2SapProvider<C>::RegistrationComplete (uint64_t imsi, uint16_t rnti)
{
  m_owner->DoRegistrationComplete (imsi, rnti);
}

template <class C>
void MemberNgcEnbN2SapProvider<C>::N2Message (uint64_t imsi, uint16_t rnti)
{
  cout<<"DoInitialUeMessage for Provider is called" << endl;
  //m_owner->DoInitialUeMessage (imsi, rnti, 0); // jhlim
  m_owner->DoN2Message (imsi, rnti);
}

template <class C>
void MemberNgcEnbN2SapProvider<C>::DoSendReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId)
{
  //m_owner->DoReleaseIndication (imsi, rnti, bearerId, 0); // jhlim
	m_owner->DoReleaseIndication (imsi, rnti, bearerId); // jhlim
}

template <class C>
void MemberNgcEnbN2SapProvider<C>::PathSwitchRequest (PathSwitchRequestParameters params)
{
  //m_owner->DoPathSwitchRequest (params, 0); // jhlim
  m_owner->DoPathSwitchRequest (params); // jhlim
}


template <class C>
void MemberNgcEnbN2SapProvider<C>::N2Message (uint64_t imsi, uint16_t rnti, int dummy)
{
  cout<<"DoInitialUeMessage for Amf is called" << endl;
  m_owner->DoN2Message (imsi, rnti, 0);
}

/*
template <class C>
void MemberNgcEnbN2SapProvider<C>::DoSendReleaseIndication1 (uint64_t imsi, uint16_t rnti, uint8_t bearerId)
{
  cout<<"DoReleaseIndication is called" << endl;
  m_owner->DoReleaseIndication (imsi, rnti, bearerId, 0);
}
template <class C>
void MemberNgcEnbN2SapProvider<C>::PathSwitchRequest1 (PathSwitchRequestParameters params)
{
  cout<<"DoPathSwitchRequest is called" << endl;
  m_owner->DoPathSwitchRequest (params, 0);
}
*/

template <class C>
void MemberNgcEnbN2SapProvider<C>::UeContextRelease (uint16_t rnti)
{
  m_owner->DoUeContextRelease (rnti);
}

/**
 * Template for the implementation of the NgcEnbN2SapUser as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNgcEnbN2SapUser : public NgcEnbN2SapUser
{
public:
  MemberNgcEnbN2SapUser (C* owner);

  // inherited from NgcEnbN2SapUser
  virtual void DataRadioBearerSetupRequest (DataRadioBearerSetupRequestParameters params);
  virtual void PathSwitchRequestAcknowledge (PathSwitchRequestAcknowledgeParameters params);
  // jhlim
  virtual void IdentityRequest (IdentityRequestParameters params);
  virtual void RegistrationAccept (RegistrationAcceptParameters params);

private:
  MemberNgcEnbN2SapUser ();
  C* m_owner;
};

template <class C>
MemberNgcEnbN2SapUser<C>::MemberNgcEnbN2SapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNgcEnbN2SapUser<C>::MemberNgcEnbN2SapUser ()
{
}

template <class C>
void MemberNgcEnbN2SapUser<C>::DataRadioBearerSetupRequest (DataRadioBearerSetupRequestParameters params)
{
  m_owner->DoDataRadioBearerSetupRequest (params);
}
// jhlim
template <class C>
void MemberNgcEnbN2SapUser<C>::IdentityRequest (IdentityRequestParameters params)
{
  m_owner->DoIdentityRequest (params);
}
template <class C>
void MemberNgcEnbN2SapUser<C>::RegistrationAccept (RegistrationAcceptParameters params)
{
  m_owner->DoRegistrationAccept (params);
}
template <class C>
void MemberNgcEnbN2SapUser<C>::PathSwitchRequestAcknowledge (PathSwitchRequestAcknowledgeParameters params)
{
  m_owner->DoPathSwitchRequestAcknowledge (params);
}

} // namespace ns3

#endif // NGC_ENB_N2_SAP_H
