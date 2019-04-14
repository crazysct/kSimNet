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
 * Author: Nicola Baldo <nbaldo@cttc.cat>
 */

#ifndef NGC_N11_SAP_H
#define NGC_N11_SAP_H

#include <ns3/address.h>
#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/eps-bearer.h>
#include <ns3/ngc-tft.h>
#include <list>

namespace ns3 {

class NgcN11Sap
{
public:

  virtual ~NgcN11Sap ();

  struct GtpcMessage
  {
    uint32_t teid;
  };

  /**
   * Fully-qualified TEID, see 3GPP TS 29.274 section 8.22
   * 
   */
  struct Fteid 
  {
    uint32_t teid;
    Ipv4Address address;
  };

  /**
   * TS 29.274 8.21  User Location Information (ULI)
   * 
   */
  struct Uli
  {
    uint16_t gci;
  };
  
 
};

/**
 * \ingroup nr
 *
 * AMF side of the N11 Service Access Point (SAP), provides the AMF
 * methods to be called when an N11 message is received by the AMF. 
 */
class NgcN11SapAmf : public NgcN11Sap
{
public:
  
 /**
   * 3GPP TS 29.274 version 8.3.1 Release 8 section 8.28
   * 
   */
  struct BearerContextCreated
  {

    NgcN11Sap::Fteid smfFteid;
    uint8_t epsBearerId; 
    EpsBearer bearerLevelQos; 
    Ptr<NgcTft> tft;
  };

  struct N2SMInformationCreated
  {
    
    EpsBearer flowLevelQos;

    uint8_t pduSessionID;
    uint8_t qosFlowId; //epsBearerId -> QFI
    uint8_t qosProfile;
    uint8_t cnN3TunnelInfo;
    uint8_t s_nssai; 
    uint8_t userPlaneSecurityEnforcement;
    uint8_t ueIntegrityProtectionMaximumDataRate;


    NgcN11Sap::Fteid smfFteid; //TODO: upfFteid????
    //uint8_t epsBearerId; 
    Ptr<NgcTft> tft;
  };



  /**     
   * Create Session Response message, see 3GPP TS 29.274 7.2.2
   */
  struct CreateSessionResponseMessage : public GtpcMessage
  {
    std::list<BearerContextCreated> bearerContextsCreated;
  };


  //smsohn 
  struct UpdateSMContextResponseMessage : public GtpcMessage
  {
    std::list<N2SMInformationCreated> n2SMInformationCreated;
  };


  /** 
   * send a Create Session Response message
   * 
   * \param msg the message
   */
  virtual void CreateSessionResponse (CreateSessionResponseMessage msg) = 0;

  struct BearerContextRemoved
  {
    uint8_t epsBearerId;
  };

  //smsohn
  virtual void UpdateSMContextResponse (UpdateSMContextResponseMessage msg) = 0;



  /**
   * Delete Bearer Request message, see 3GPP TS 29.274 Release 9 V9.3.0 section 7.2.9.2
   */
  struct DeleteBearerRequestMessage : public GtpcMessage
  {
    std::list<BearerContextRemoved> bearerContextsRemoved;
  };

  /**
    * \brief As per 3GPP TS 29.274 Release 9 V9.3.0, a Delete Bearer Request message shall be sent on the N11 interface by UPF to SMF and from SMF to AMF
    * \param msg the message
    */
  virtual void DeleteBearerRequest (DeleteBearerRequestMessage msg) = 0;




  /**     
   * Modify Bearer Response message, see 3GPP TS 29.274 7.2.7
   */
  struct ModifyBearerResponseMessage : public GtpcMessage
  {
    enum Cause {
      REQUEST_ACCEPTED = 0,
      REQUEST_ACCEPTED_PARTIALLY,
      REQUEST_REJECTED,
      CONTEXT_NOT_FOUND
    } cause;
  };

  /** 
   * send a Modify Bearer Response message
   * 
   * \param msg the message
   */
  virtual void ModifyBearerResponse (ModifyBearerResponseMessage msg) = 0;

};

/**
 * \ingroup nr
 *
 * SMF side of the N11 Service Access Point (SAP), provides the SMF
 * methods to be called when an N11 message is received by the SMF. 
 */
class NgcN11SapSmf : public NgcN11Sap
{
public:

  struct BearerContextToBeCreated
  {    
    NgcN11Sap::Fteid smfFteid;
    uint8_t epsBearerId; 
    EpsBearer bearerLevelQos; 
    Ptr<NgcTft> tft;
  };

  //yjshin
  struct N2SMInformationToBeCreated
  {     
    EpsBearer flowLevelQos;

    uint8_t pduSessionID;
    uint8_t qosFlowId; //epsBearerId -> QFI
    uint8_t qosProfile;
    uint8_t cnN3TunnelInfo;
    uint8_t s_nssai; 
    uint8_t userPlaneSecurityEnforcement;
    uint8_t ueIntegrityProtectionMaximumDataRate;


    NgcN11Sap::Fteid smfFteid; //TODO: upfFteid????
    //uint8_t epsBearerId; 
    Ptr<NgcTft> tft;
  };
  
  /**     
   * Create Session Request message, see 3GPP TS 29.274 7.2.1
   */
  struct CreateSessionRequestMessage : public GtpcMessage
  {
    uint64_t imsi; 
    Uli uli; 
    std::list<BearerContextToBeCreated> bearerContextsToBeCreated;    
  };

  //yjshin
  struct UpdateSMContextRequestMessage : public GtpcMessage
  {
    uint64_t imsi; 
    Uli uli; 
    std::list<N2SMInformationToBeCreated> n2SMInformationToBeCreated;    //Todo
  };


  /** 
   * send a Create Session Request message
   * 
   * \param msg the message
   */
  virtual void CreateSessionRequest (CreateSessionRequestMessage msg) = 0;

  struct BearerContextToBeRemoved
  {
    uint8_t epsBearerId;
  };

  virtual void UpdateSMContextRequest (UpdateSMContextRequestMessage msg) = 0;

  /**
   * Delete Bearer Command message, see 3GPP TS 29.274 Release 9 V9.3.0 section 7.2.17.1
   */
  struct DeleteBearerCommandMessage : public GtpcMessage
  {
    std::list<BearerContextToBeRemoved> bearerContextsToBeRemoved;
  };

  /**
    * \brief As per 3GPP TS 29.274 Release 9 V9.3.0, a Delete Bearer Command message shall be sent on the N11 interface by the AMF to the SMF
    */
  virtual void DeleteBearerCommand (DeleteBearerCommandMessage msg) = 0;


  struct BearerContextRemovedSmfUpf
  {
    uint8_t epsBearerId;
  };

  /**
   * Delete Bearer Response message, see 3GPP TS 29.274 Release 9 V9.3.0 section 7.2.10.2
   */
  struct DeleteBearerResponseMessage : public GtpcMessage
  {
    std::list<BearerContextRemovedSmfUpf> bearerContextsRemoved;
  };

  /**
    * \brief As per 3GPP TS 29.274 Release 9 V9.3.0, a Delete Bearer Command message shall be sent on the N11 interface by the AMF to the SMF
    * \param msg the message
    */
  virtual void DeleteBearerResponse (DeleteBearerResponseMessage msg) = 0;

  /**     
   * Modify Bearer Request message, see 3GPP TS 29.274 7.2.7
   */
  struct ModifyBearerRequestMessage : public GtpcMessage
  {
    Uli uli;
  };

  /** 
   * send a Modify Bearer Request message
   * 
   * \param msg the message
   */
  virtual void ModifyBearerRequest (ModifyBearerRequestMessage msg) = 0;

};







/**
 * Template for the implementation of the NgcN11SapAmf as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNgcN11SapAmf : public NgcN11SapAmf
{
public:
  MemberNgcN11SapAmf (C* owner);

  // inherited from NgcN11SapAmf
  virtual void CreateSessionResponse (CreateSessionResponseMessage msg);
  virtual void UpdateSMContextResponse (UpdateSMContextResponseMessage msg); //smsohn
  virtual void ModifyBearerResponse (ModifyBearerResponseMessage msg);
  virtual void DeleteBearerRequest (DeleteBearerRequestMessage msg);

private:
  MemberNgcN11SapAmf ();
  C* m_owner;
};

template <class C>
MemberNgcN11SapAmf<C>::MemberNgcN11SapAmf (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNgcN11SapAmf<C>::MemberNgcN11SapAmf ()
{
}

template <class C>
void MemberNgcN11SapAmf<C>::CreateSessionResponse (CreateSessionResponseMessage msg)
{
  m_owner->DoCreateSessionResponse (msg);
}

//smsohn
template <class C>
void MemberNgcN11SapAmf<C>::UpdateSMContextResponse (UpdateSMContextResponseMessage msg)
{
  m_owner->DoUpdateSMContextResponse (msg);
}



template <class C>
void MemberNgcN11SapAmf<C>::DeleteBearerRequest (DeleteBearerRequestMessage msg)
{
  m_owner->DoDeleteBearerRequest (msg);
}

template <class C>
void MemberNgcN11SapAmf<C>::ModifyBearerResponse (ModifyBearerResponseMessage msg)
{
  m_owner->DoModifyBearerResponse (msg);
}





/**
 * Template for the implementation of the NgcN11SapSmf as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNgcN11SapSmf : public NgcN11SapSmf
{
public:
  MemberNgcN11SapSmf (C* owner);

  // inherited from NgcN11SapSmf
  virtual void CreateSessionRequest (CreateSessionRequestMessage msg);
  virtual void UpdateSMContextRequest (UpdateSMContextRequestMessage msg);
  virtual void ModifyBearerRequest (ModifyBearerRequestMessage msg);
  virtual void DeleteBearerCommand (DeleteBearerCommandMessage msg);
  virtual void DeleteBearerResponse (DeleteBearerResponseMessage msg);

private:
  MemberNgcN11SapSmf ();
  C* m_owner;
};

template <class C>
MemberNgcN11SapSmf<C>::MemberNgcN11SapSmf (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNgcN11SapSmf<C>::MemberNgcN11SapSmf ()
{
}

template <class C>
void MemberNgcN11SapSmf<C>::CreateSessionRequest (CreateSessionRequestMessage msg)
{
  m_owner->DoCreateSessionRequest (msg);
}

template <class C>
void MemberNgcN11SapSmf<C>::UpdateSMContextRequest (UpdateSMContextRequestMessage msg)
{
  m_owner->DoUpdateSMContextRequest (msg);
}


template <class C>
void MemberNgcN11SapSmf<C>::ModifyBearerRequest (ModifyBearerRequestMessage msg)
{
  m_owner->DoModifyBearerRequest (msg);
}

template <class C>
void MemberNgcN11SapSmf<C>::DeleteBearerCommand (DeleteBearerCommandMessage msg)
{
  m_owner->DoDeleteBearerCommand (msg);
}

template <class C>
void MemberNgcN11SapSmf<C>::DeleteBearerResponse (DeleteBearerResponseMessage msg)
{
  m_owner->DoDeleteBearerResponse (msg);
}




} //namespace ns3

#endif /* NGC_N11_SAP_H */

