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
 * Author: Michele Polese <michele.polese@gmail.com>
 * Inspired by ngc-x2-header.h
 */

#ifndef NGC_N2AP_HEADER_H
#define NGC_N2AP_HEADER_H

#include "ns3/ngc-n2ap-sap.h"
#include "ns3/header.h"

#include <list>


namespace ns3 {


class NgcN2APHeader : public Header
{
public:
  NgcN2APHeader ();
  virtual ~NgcN2APHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  uint8_t GetProcedureCode () const;
  void SetProcedureCode (uint8_t procedureCode);

  void SetLengthOfIes (uint32_t lengthOfIes);
  void SetNumberOfIes (uint32_t numberOfIes);


  enum ProcedureCode_t {
    RegistrationRequest        = 71,
    InitialUeMessage        = 71,
    N2Message               = 73,
    PathSwitchRequest       = 58,
    ErabReleaseIndication   = 37,
    InitialContextSetupResponse = 43,
    InitialContextSetupRequest = 41,
    PathSwitchRequestAck = 59,
	IdentityRequest = 77,
	IdentityResponse = 78,
	RegistrationAccept = 79,
	RegistrationComplete = 80,

    N2Request		    = 42
  };


private:
  uint8_t m_procedureCode;

  uint32_t m_lengthOfIes;
  uint32_t m_numberOfIes;
};


// Header for registration Request message
class NgcN2APRegistrationRequestHeader : public Header
{
public:
  NgcN2APRegistrationRequestHeader ();
  virtual ~NgcN2APRegistrationRequestHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  uint64_t GetSTmsi () const;
  void SetSTmsi (uint64_t stmsi);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint64_t          m_stmsi;
  uint64_t          m_amfUeN2Id;
  uint16_t          m_enbUeN2Id;
  uint16_t          m_ecgi;
};

// Header for N2 message
class NgcN2APN2MessageHeader : public Header
{
public:
  NgcN2APN2MessageHeader ();
  virtual ~NgcN2APN2MessageHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  uint64_t GetSTmsi () const;
  void SetSTmsi (uint64_t stmsi);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint64_t          m_stmsi;
  uint64_t          m_amfUeN2Id;
  uint16_t          m_enbUeN2Id;
  uint16_t          m_ecgi;
};


class NgcN2APErabReleaseIndicationHeader : public Header
{
public:
  NgcN2APErabReleaseIndicationHeader ();
  virtual ~NgcN2APErabReleaseIndicationHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  std::list<NgcN2apSap::ErabToBeReleasedIndication> GetErabToBeReleaseIndication () const;
  void SetErabReleaseIndication (std::list<NgcN2apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeN2Id;
  uint64_t          m_amfUeN2Id;
  std::list<NgcN2apSap::ErabToBeReleasedIndication> m_erabToBeReleaseIndication;
};


class NgcN2APInitialContextSetupResponseHeader : public Header
{
public:
  NgcN2APInitialContextSetupResponseHeader ();
  virtual ~NgcN2APInitialContextSetupResponseHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  std::list<NgcN2apSap::ErabSetupItem> GetErabSetupItem () const;
  void SetErabSetupItem (std::list<NgcN2apSap::ErabSetupItem> erabSetupList);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeN2Id;
  uint64_t          m_amfUeN2Id;
  std::list<NgcN2apSap::ErabSetupItem> m_erabSetupList;
};

class NgcN2APPathSwitchRequestHeader : public Header
{
public:
  NgcN2APPathSwitchRequestHeader ();
  virtual ~NgcN2APPathSwitchRequestHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  std::list<NgcN2apSap::ErabSwitchedInDownlinkItem> GetErabSwitchedInDownlinkItemList () const;
  void SetErabSwitchedInDownlinkItemList (std::list<NgcN2apSap::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeN2Id;
  uint16_t          m_ecgi;
  uint64_t          m_amfUeN2Id;
  std::list<NgcN2apSap::ErabSwitchedInDownlinkItem> m_erabToBeSwitchedInDownlinkList;
};

class NgcN2APInitialContextSetupRequestHeader : public Header
{
public:
  NgcN2APInitialContextSetupRequestHeader ();
  virtual ~NgcN2APInitialContextSetupRequestHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  // jhlim
  uint64_t GetGuti () const;
  void SetGuti (uint64_t guti);

  std::list<NgcN2apSap::ErabToBeSetupItem> GetErabToBeSetupItem () const;
  void SetErabToBeSetupItem (std::list<NgcN2apSap::ErabToBeSetupItem> erabToBeSetupList);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeN2Id;
  uint64_t          m_amfUeN2Id;
  uint64_t			m_guti; // jhlim
  std::list<NgcN2apSap::ErabToBeSetupItem> m_erabsToBeSetupList;
};

class NgcN2APN2RequestHeader : public Header
{
public:
  NgcN2APN2RequestHeader ();
  virtual ~NgcN2APN2RequestHeader ();
  
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  
  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);
  
  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);
  
  std::list<NgcN2apSap::ErabToBeSetupItem> GetErabToBeSetupItem () const;
  void SetErabToBeSetupItem (std::list<NgcN2apSap::ErabToBeSetupItem> erabToBeSetupList);
  
  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeN2Id;
  uint64_t          m_amfUeN2Id;
  std::list<NgcN2apSap::ErabToBeSetupItem> m_erabsToBeSetupList;
};


class NgcN2APPathSwitchRequestAcknowledgeHeader : public Header
{
public:
  NgcN2APPathSwitchRequestAcknowledgeHeader ();
  virtual ~NgcN2APPathSwitchRequestAcknowledgeHeader ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  std::list<NgcN2apSap::ErabSwitchedInUplinkItem> GetErabSwitchedInUplinkItemList () const;
  void SetErabSwitchedInUplinkItemList (std::list<NgcN2apSap::ErabSwitchedInUplinkItem> erabToBeSwitchedInDownlinkList);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeN2Id;
  uint16_t          m_ecgi;
  uint64_t          m_amfUeN2Id;
  std::list<NgcN2apSap::ErabSwitchedInUplinkItem> m_erabToBeSwitchedInUplinkList;
};

/* jhlim: copy InitialContextSetupRequest  */
/*
class NgcN2APIdentityRequest : public Header
{
public:
  NgcN2APIdentityRequest ();
  virtual ~NgcN2APIdentityRequest ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeN2Id;
  uint16_t          m_ecgi;
  uint64_t          m_amfUeN2Id;
};
class NgcN2APIdentityResponse : public Header
{
public:
  NgcN2APIdentityResponse ();
  virtual ~NgcN2APIdentityResponse ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeN2Id;
  uint16_t          m_ecgi;
  uint64_t          m_amfUeN2Id;
};
class NgcN2APRegistrationAccept : public Header
{
public:
  NgcN2APRegistrationAccept ();
  virtual ~NgcN2APRegistrationAccept ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  uint64_t GetGuti() const;
  void SetGuti (uint64_t guti);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeN2Id;
  uint16_t          m_ecgi;
  uint64_t          m_amfUeN2Id;
  uint64_t			m_guti;
};
class NgcN2APRegistrationComplete : public Header
{
public:
  NgcN2APRegistrationComplete ();
  virtual ~NgcN2APRegistrationComplete ();

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;


  uint64_t GetAmfUeN2Id () const;
  void SetAmfUeN2Id (uint64_t amfUeN2Id);

  uint16_t GetEnbUeN2Id () const;
  void SetEnbUeN2Id (uint16_t enbUeN2Id);

  uint16_t GetEcgi () const;
  void SetEcgi (uint16_t ecgi);

  uint64_t GetGuti() const;
  void SetGuti (uint64_t guti);

  uint32_t GetLengthOfIes () const;
  uint32_t GetNumberOfIes () const;

private:
  uint32_t          m_numberOfIes;
  uint32_t          m_headerLength;
  uint16_t          m_enbUeN2Id;
  uint16_t          m_ecgi;
  uint64_t          m_amfUeN2Id;
  uint64_t			m_guti;
};
*/


}

#endif //NGC_N2AP_HEADER_H
