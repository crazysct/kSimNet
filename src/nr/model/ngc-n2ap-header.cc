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
 * Inspired by ngc-x2-header.cc
 */

#include "ns3/log.h"
#include "ns3/ngc-n2ap-header.h"
#include <list>


// TODO 
// According to 36.413 9.2.3.4: enbUeN2Id should be 3 byte, but in the SAP interface 
// already defined in the ns-3 release is 2 byte
// The same holds for amfUeN2Id, which should be 4 byte, but is 8 byte in the SAP interface

namespace ns3 {


NS_LOG_COMPONENT_DEFINE ("NgcN2APHeader");

NS_OBJECT_ENSURE_REGISTERED (NgcN2APHeader);

NgcN2APHeader::NgcN2APHeader ()
  : m_procedureCode (0xfa),
    m_lengthOfIes (0xfa),
    m_numberOfIes (0xfa)
{
}

NgcN2APHeader::~NgcN2APHeader ()
{
  m_procedureCode = 0xfb;
  m_lengthOfIes = 0xfb;
  m_numberOfIes = 0xfb;
}

TypeId
NgcN2APHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2APHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcN2APHeader> ()
  ;
  return tid;
}

TypeId
NgcN2APHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcN2APHeader::GetSerializedSize (void) const
{
  return 6; // 6 bytes in this header
}

void
NgcN2APHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 (m_procedureCode);

  i.WriteU8 (0x00); // 36.413 9.1.2.2 Criticality - if not recognized, 0 == reject
  i.WriteU8 (m_lengthOfIes + 3);
  i.WriteHtonU16 (0);
  i.WriteU8 (m_numberOfIes);
}

uint32_t
NgcN2APHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_procedureCode = i.ReadU8 ();

  i.ReadU8 ();
  m_lengthOfIes = i.ReadU8 () - 3;
  i.ReadNtohU16 ();
  m_numberOfIes = i.ReadU8 ();
  
  return GetSerializedSize ();
}

void
NgcN2APHeader::Print (std::ostream &os) const
{
  os << " ProcedureCode=" << (uint32_t) m_procedureCode;
  os << " LengthOfIEs=" << (uint32_t) m_lengthOfIes;
  os << " NumberOfIEs=" << (uint32_t) m_numberOfIes;
}

uint8_t
NgcN2APHeader::GetProcedureCode () const
{
  return m_procedureCode;
}

void
NgcN2APHeader::SetProcedureCode (uint8_t procedureCode)
{
  m_procedureCode = procedureCode;
}


void
NgcN2APHeader::SetLengthOfIes (uint32_t lengthOfIes)
{
  m_lengthOfIes = lengthOfIes;
}

void
NgcN2APHeader::SetNumberOfIes (uint32_t numberOfIes)
{
  m_numberOfIes = numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcN2APRegistrationRequestHeader);

NgcN2APRegistrationRequestHeader::NgcN2APRegistrationRequestHeader ()
  : m_numberOfIes (1 + 1 + 1 + 1 + 1 + 1 + 1),
    m_headerLength (3 + 2 + 6 + 4 + 2 + 9 + 9),
    m_stmsi (0xfffffffa),
    m_amfUeN2Id (0xfffffffa),
    m_enbUeN2Id (0xfffa),
    m_ecgi (0xfffa)
{
}

NgcN2APRegistrationRequestHeader::~NgcN2APRegistrationRequestHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_stmsi = 0xfffffffb;
  m_enbUeN2Id = 0xfffb;
  m_ecgi = 0xfffb;
  m_amfUeN2Id = 0xfffffffb;
}

TypeId
NgcN2APRegistrationRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2APRegistrationRequestHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcN2APRegistrationRequestHeader> ()
  ;
  return tid;
}

TypeId
NgcN2APRegistrationRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcN2APRegistrationRequestHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcN2APRegistrationRequestHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcN2APHeader
  i.WriteHtonU16 (m_enbUeN2Id);     // m_enbUeN2Id
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteU8 (0);                    // NAS PDU, not implemented
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteU32 (0);                   // TAI, not implemented
  i.WriteU8 (0);
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteHtonU16 (m_ecgi);          // E-UTRAN CGI, it should have a different size
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteU8 (0);                    // RRC Establishment cause
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteU64 (m_stmsi);             // S-TMSI
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteU64 (m_amfUeN2Id);         // amfUeN2Id, not in the standard?
  i.WriteU8 (0);                    // criticality = REJECT

}

uint32_t
NgcN2APRegistrationRequestHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_enbUeN2Id = i.ReadNtohU16 ();
  i.ReadU8 ();
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU8();
  i.ReadU8();
  m_headerLength += 2;
  m_numberOfIes++;

  i.ReadU32 ();                   // TAI, not implemented
  i.ReadU8 ();
  i.ReadU8 ();                    
  m_headerLength += 6;
  m_numberOfIes++;

  m_ecgi = i.ReadNtohU16 ();    // E-UTRAN CGI, it should have a different size
  i.ReadU8();
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU8();
  i.ReadU8();
  m_headerLength += 2;
  m_numberOfIes++;

  m_stmsi = i.ReadU64 ();             // S-TMSI
  i.ReadU8 ();      
  m_headerLength += 9;
  m_numberOfIes++;

  m_amfUeN2Id = i.ReadU64 ();             // AMF UE ID
  i.ReadU8 ();      
  m_headerLength += 9;
  m_numberOfIes++;

  return GetSerializedSize();
}

void
NgcN2APRegistrationRequestHeader::Print (std::ostream &os) const
{
  os << "AmfUeN2apId = " << m_amfUeN2Id;
  os << " EnbUeN2Id = " << m_enbUeN2Id;
  os << " ECGI = " << m_ecgi;
  os << " S-TMSI = " << m_stmsi;
}

uint64_t 
NgcN2APRegistrationRequestHeader::GetAmfUeN2Id () const 
{
  return m_amfUeN2Id;
}

void 
NgcN2APRegistrationRequestHeader::SetAmfUeN2Id (uint64_t amfUeN2Id) 
{
  m_amfUeN2Id = amfUeN2Id;
}

uint16_t 
NgcN2APRegistrationRequestHeader::GetEnbUeN2Id () const
{
  return m_enbUeN2Id;
}

void 
NgcN2APRegistrationRequestHeader::SetEnbUeN2Id (uint16_t enbUeN2Id)
{
  m_enbUeN2Id = enbUeN2Id;
}

uint64_t 
NgcN2APRegistrationRequestHeader::GetSTmsi () const 
{
  return m_stmsi;
}

void 
NgcN2APRegistrationRequestHeader::SetSTmsi (uint64_t stmsi) 
{
  m_stmsi = stmsi;
}

uint16_t 
NgcN2APRegistrationRequestHeader::GetEcgi () const 
{
  return m_ecgi;
}

void 
NgcN2APRegistrationRequestHeader::SetEcgi (uint16_t ecgi)
{
  m_ecgi = ecgi;
}

uint32_t
NgcN2APRegistrationRequestHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcN2APRegistrationRequestHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

NS_OBJECT_ENSURE_REGISTERED (NgcN2APN2MessageHeader);

NgcN2APN2MessageHeader::NgcN2APN2MessageHeader ()
  : m_numberOfIes (1 + 1 + 1 + 1 + 1 + 1 + 1),
    m_headerLength (3 + 2 + 6 + 4 + 2 + 9 + 9),
    m_stmsi (0xfffffffa),
    m_amfUeN2Id (0xfffffffa),
    m_enbUeN2Id (0xfffa),
    m_ecgi (0xfffa)
{
}

NgcN2APN2MessageHeader::~NgcN2APN2MessageHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_stmsi = 0xfffffffb;
  m_enbUeN2Id = 0xfffb;
  m_ecgi = 0xfffb;
  m_amfUeN2Id = 0xfffffffb;
}

TypeId
NgcN2APN2MessageHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2APN2MessageHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcN2APN2MessageHeader> ()
  ;
  return tid;
}

TypeId
NgcN2APN2MessageHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcN2APN2MessageHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcN2APN2MessageHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcN2APHeader
  i.WriteHtonU16 (m_enbUeN2Id);     // m_enbUeN2Id
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteU8 (0);                    // NAS PDU, not implemented
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteU32 (0);                   // TAI, not implemented
  i.WriteU8 (0);
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteHtonU16 (m_ecgi);          // E-UTRAN CGI, it should have a different size
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteU8 (0);                    // RRC Establishment cause
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteU64 (m_stmsi);             // S-TMSI
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteU64 (m_amfUeN2Id);         // amfUeN2Id, not in the standard?
  i.WriteU8 (0);                    // criticality = REJECT

}

uint32_t
NgcN2APN2MessageHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_enbUeN2Id = i.ReadNtohU16 ();
  i.ReadU8 ();
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU8();
  i.ReadU8();
  m_headerLength += 2;
  m_numberOfIes++;

  i.ReadU32 ();                   // TAI, not implemented
  i.ReadU8 ();
  i.ReadU8 ();                    
  m_headerLength += 6;
  m_numberOfIes++;

  m_ecgi = i.ReadNtohU16 ();    // E-UTRAN CGI, it should have a different size
  i.ReadU8();
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU8();
  i.ReadU8();
  m_headerLength += 2;
  m_numberOfIes++;

  m_stmsi = i.ReadU64 ();             // S-TMSI
  i.ReadU8 ();      
  m_headerLength += 9;
  m_numberOfIes++;

  m_amfUeN2Id = i.ReadU64 ();             // AMF UE ID
  i.ReadU8 ();      
  m_headerLength += 9;
  m_numberOfIes++;

  return GetSerializedSize();
}

void
NgcN2APN2MessageHeader::Print (std::ostream &os) const
{
  os << "AmfUeN2apId = " << m_amfUeN2Id;
  os << " EnbUeN2Id = " << m_enbUeN2Id;
  os << " ECGI = " << m_ecgi;
  os << " S-TMSI = " << m_stmsi;
}

uint64_t 
NgcN2APN2MessageHeader::GetAmfUeN2Id () const 
{
  return m_amfUeN2Id;
}

void 
NgcN2APN2MessageHeader::SetAmfUeN2Id (uint64_t amfUeN2Id) 
{
  m_amfUeN2Id = amfUeN2Id;
}

uint16_t 
NgcN2APN2MessageHeader::GetEnbUeN2Id () const
{
  return m_enbUeN2Id;
}

void 
NgcN2APN2MessageHeader::SetEnbUeN2Id (uint16_t enbUeN2Id)
{
  m_enbUeN2Id = enbUeN2Id;
}

uint64_t 
NgcN2APN2MessageHeader::GetSTmsi () const 
{
  return m_stmsi;
}

void 
NgcN2APN2MessageHeader::SetSTmsi (uint64_t stmsi) 
{
  m_stmsi = stmsi;
}

uint16_t 
NgcN2APN2MessageHeader::GetEcgi () const 
{
  return m_ecgi;
}

void 
NgcN2APN2MessageHeader::SetEcgi (uint16_t ecgi)
{
  m_ecgi = ecgi;
}

uint32_t
NgcN2APN2MessageHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcN2APN2MessageHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcN2APErabReleaseIndicationHeader);

NgcN2APErabReleaseIndicationHeader::NgcN2APErabReleaseIndicationHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (9 + 3 + 4 + 1),
    m_enbUeN2Id (0xfffa),
    m_amfUeN2Id (0xfffffffa)
{
  m_erabToBeReleaseIndication.clear();
}

NgcN2APErabReleaseIndicationHeader::~NgcN2APErabReleaseIndicationHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeN2Id = 0xfffb;
  m_amfUeN2Id = 0xfffffffb;
  m_erabToBeReleaseIndication.clear();
}

TypeId
NgcN2APErabReleaseIndicationHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2APErabReleaseIndicationHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcN2APErabReleaseIndicationHeader> ()
  ;
  return tid;
}

TypeId
NgcN2APErabReleaseIndicationHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcN2APErabReleaseIndicationHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcN2APErabReleaseIndicationHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcN2APHeader
  i.WriteU64 (m_amfUeN2Id);         // amfUeN2Id
  i.WriteU8 (0);                    // criticality = REJECT

  i.WriteHtonU16 (m_enbUeN2Id);     // m_enbUeN2Id
  i.WriteU8 (0);                    // criticality = REJECT

  std::list <NgcN2apSap::ErabToBeReleasedIndication>::size_type sz = m_erabToBeReleaseIndication.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcN2apSap::ErabToBeReleasedIndication>::const_iterator l_iter = m_erabToBeReleaseIndication.begin(); l_iter != m_erabToBeReleaseIndication.end(); ++l_iter) // content of ErabToBeReleasedIndication
  {
    i.WriteU8 (l_iter->erabId);
  }
  i.WriteU8(0); // criticality = REJECT, just one for the whole list

}

uint32_t
NgcN2APErabReleaseIndicationHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_amfUeN2Id = i.ReadU64 ();         // amfUeN2Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_enbUeN2Id = i.ReadNtohU16 ();     // m_enbUeN2Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of ErabToBeReleasedIndication
  {
    NgcN2apSap::ErabToBeReleasedIndication erabItem;
    erabItem.erabId = i.ReadU8 ();

    m_erabToBeReleaseIndication.push_back(erabItem);
    m_headerLength += 1;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;
  
  return GetSerializedSize();
}

void
NgcN2APErabReleaseIndicationHeader::Print (std::ostream &os) const
{
  os << "AmfUeN2apId = " << m_amfUeN2Id;
  os << " EnbUeN2Id = " << m_enbUeN2Id;
  for(std::list <NgcN2apSap::ErabToBeReleasedIndication>::const_iterator l_iter = m_erabToBeReleaseIndication.begin(); l_iter != m_erabToBeReleaseIndication.end(); ++l_iter)
  { 
    os << " ErabId " << l_iter->erabId;
  }

}

uint64_t 
NgcN2APErabReleaseIndicationHeader::GetAmfUeN2Id () const 
{
  return m_amfUeN2Id;
}

void 
NgcN2APErabReleaseIndicationHeader::SetAmfUeN2Id (uint64_t amfUeN2Id) 
{
  m_amfUeN2Id = amfUeN2Id;
}

uint16_t 
NgcN2APErabReleaseIndicationHeader::GetEnbUeN2Id () const
{
  return m_enbUeN2Id;
}

void 
NgcN2APErabReleaseIndicationHeader::SetEnbUeN2Id (uint16_t enbUeN2Id)
{
  m_enbUeN2Id = enbUeN2Id;
}

std::list<NgcN2apSap::ErabToBeReleasedIndication>
NgcN2APErabReleaseIndicationHeader::GetErabToBeReleaseIndication () const 
{
  return m_erabToBeReleaseIndication;
}

void 
NgcN2APErabReleaseIndicationHeader::SetErabReleaseIndication (std::list<NgcN2apSap::ErabToBeReleasedIndication> erabToBeReleaseIndication)
{
  m_headerLength += erabToBeReleaseIndication.size();
  m_erabToBeReleaseIndication = erabToBeReleaseIndication;
}

uint32_t
NgcN2APErabReleaseIndicationHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcN2APErabReleaseIndicationHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcN2APInitialContextSetupResponseHeader);

NgcN2APInitialContextSetupResponseHeader::NgcN2APInitialContextSetupResponseHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (9 + 3 + 4 + 1),
    m_enbUeN2Id (0xfffa),
    m_amfUeN2Id (0xfffffffa)
{
  m_erabSetupList.clear();
}

NgcN2APInitialContextSetupResponseHeader::~NgcN2APInitialContextSetupResponseHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeN2Id = 0xfffb;
  m_amfUeN2Id = 0xfffffffb;
  m_erabSetupList.clear();
}

TypeId
NgcN2APInitialContextSetupResponseHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2APInitialContextSetupResponseHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcN2APInitialContextSetupResponseHeader> ()
  ;
  return tid;
}

TypeId
NgcN2APInitialContextSetupResponseHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcN2APInitialContextSetupResponseHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcN2APInitialContextSetupResponseHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcN2APHeader
  i.WriteU64 (m_amfUeN2Id);         // amfUeN2Id
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteHtonU16 (m_enbUeN2Id);     // m_enbUeN2Id
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  std::list <NgcN2apSap::ErabSetupItem>::size_type sz = m_erabSetupList.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcN2apSap::ErabSetupItem>::const_iterator l_iter = m_erabSetupList.begin(); l_iter != m_erabSetupList.end(); ++l_iter) // content of m_erabSetupList
  {
    i.WriteU8 (l_iter->erabId);
    i.WriteHtonU32 (l_iter->enbTransportLayerAddress.Get ());
    i.WriteHtonU32 (l_iter->enbTeid);
    i.WriteU8 (1 << 6);               // criticality = IGNORE each
  }
  i.WriteU8 (1 << 6);               // criticality = IGNORE

}

uint32_t
NgcN2APInitialContextSetupResponseHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_amfUeN2Id = i.ReadU64 ();         // amfUeN2Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_enbUeN2Id = i.ReadNtohU16 ();     // m_enbUeN2Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of ErabToBeReleasedIndication
  {
    NgcN2apSap::ErabSetupItem erabItem;
    erabItem.erabId = i.ReadU8 ();
    erabItem.enbTransportLayerAddress = Ipv4Address (i.ReadNtohU32 ());
    erabItem.enbTeid = i.ReadNtohU32 ();
    i.ReadU8 ();

    m_erabSetupList.push_back(erabItem);
    m_headerLength += 10;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;
  
  return GetSerializedSize();
}

void
NgcN2APInitialContextSetupResponseHeader::Print (std::ostream &os) const
{
  os << "AmfUeN2apId = " << m_amfUeN2Id;
  os << " EnbUeN2Id = " << m_enbUeN2Id;
  for (std::list <NgcN2apSap::ErabSetupItem>::const_iterator l_iter = m_erabSetupList.begin(); l_iter != m_erabSetupList.end(); ++l_iter) // content of m_erabSetupList
  {
    os << " ErabId " << l_iter->erabId;
    os << " enbTransportLayerAddress " << l_iter->enbTransportLayerAddress;
    os << " enbTeid " << l_iter->enbTeid;
  }

}

uint64_t 
NgcN2APInitialContextSetupResponseHeader::GetAmfUeN2Id () const 
{
  return m_amfUeN2Id;
}

void 
NgcN2APInitialContextSetupResponseHeader::SetAmfUeN2Id (uint64_t amfUeN2Id) 
{
  m_amfUeN2Id = amfUeN2Id;
}

uint16_t 
NgcN2APInitialContextSetupResponseHeader::GetEnbUeN2Id () const
{
  return m_enbUeN2Id;
}

void 
NgcN2APInitialContextSetupResponseHeader::SetEnbUeN2Id (uint16_t enbUeN2Id)
{
  m_enbUeN2Id = enbUeN2Id;
}

std::list<NgcN2apSap::ErabSetupItem>
NgcN2APInitialContextSetupResponseHeader::GetErabSetupItem () const 
{
  return m_erabSetupList;
}

void 
NgcN2APInitialContextSetupResponseHeader::SetErabSetupItem (std::list<NgcN2apSap::ErabSetupItem> erabSetupList)
{
  m_headerLength += erabSetupList.size() * 10;
  m_erabSetupList = erabSetupList;
}

uint32_t
NgcN2APInitialContextSetupResponseHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcN2APInitialContextSetupResponseHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcN2APPathSwitchRequestHeader);

NgcN2APPathSwitchRequestHeader::NgcN2APPathSwitchRequestHeader ()
  : m_numberOfIes (1 + 1 + 1 + 1 + 1),
    m_headerLength (3 + 4 + 1 +9 + 3 + 6),
    m_enbUeN2Id (0xfffa),
    m_ecgi (0xfffa),
    m_amfUeN2Id (0xfffffffa)
{
  m_erabToBeSwitchedInDownlinkList.clear();
}

NgcN2APPathSwitchRequestHeader::~NgcN2APPathSwitchRequestHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeN2Id = 0xfffb;
  m_ecgi = 0xfffb;
  m_amfUeN2Id = 0xfffffffb;
  m_erabToBeSwitchedInDownlinkList.clear();
}

TypeId
NgcN2APPathSwitchRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2APPathSwitchRequestHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcN2APPathSwitchRequestHeader> ()
  ;
  return tid;
}

TypeId
NgcN2APPathSwitchRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcN2APPathSwitchRequestHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}


void
NgcN2APPathSwitchRequestHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcN2APHeader 
  i.WriteHtonU16 (m_enbUeN2Id);     // m_enbUeN2Id
  i.WriteU8 (0);               // criticality = REJECT

  std::list <NgcN2apSap::ErabSwitchedInDownlinkItem>::size_type sz = m_erabToBeSwitchedInDownlinkList.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcN2apSap::ErabSwitchedInDownlinkItem>::const_iterator l_iter = m_erabToBeSwitchedInDownlinkList.begin(); l_iter != m_erabToBeSwitchedInDownlinkList.end(); ++l_iter) // content of ErabToBeReleasedIndication // content of m_erabToBeSwitchedInDownlinkList
  {
    i.WriteU8 (l_iter->erabId);
    i.WriteHtonU32 (l_iter->enbTransportLayerAddress.Get ());
    i.WriteHtonU32 (l_iter->enbTeid);
    i.WriteU8 (0);               // criticality = REJECT each
  }
  i.WriteU8 (0);               // criticality = REJECT

  i.WriteU64 (m_amfUeN2Id);         // amfUeN2Id
  i.WriteU8 (0);               // criticality = REJECT

  i.WriteU16 (m_ecgi);      // ecgi
  i.WriteU8 (1 << 6);       // criticality = IGNORE

  i.WriteU32 (0);                   // TAI, not implemented
  i.WriteU8 (0);
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  // TODO add 9.2.1.40

}

uint32_t
NgcN2APPathSwitchRequestHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_enbUeN2Id = i.ReadNtohU16 ();     // m_enbUeN2Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of ErabToBeReleasedIndication
  {
    NgcN2apSap::ErabSwitchedInDownlinkItem erabItem;
    erabItem.erabId = i.ReadU8 ();
    erabItem.enbTransportLayerAddress = Ipv4Address (i.ReadNtohU32 ());
    erabItem.enbTeid = i.ReadNtohU32 ();
    i.ReadU8 ();

    m_erabToBeSwitchedInDownlinkList.push_back(erabItem);
    m_headerLength += 10;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;

  m_amfUeN2Id = i.ReadU64 ();         // amfUeN2Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_ecgi = i.ReadU16 ();      // ecgi
  i.ReadU8();
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU32 ();                   // TAI, not implemented
  i.ReadU8 ();
  i.ReadU8 ();               // criticality = IGNORE

  m_headerLength += 6;
  m_numberOfIes++;

  return GetSerializedSize();
}

void
NgcN2APPathSwitchRequestHeader::Print (std::ostream &os) const
{
  os << "AmfUeN2apId = " << m_amfUeN2Id;
  os << " EnbUeN2Id = " << m_enbUeN2Id;
  os << " ECGI = " << m_ecgi; 
  for (std::list <NgcN2apSap::ErabSwitchedInDownlinkItem>::const_iterator l_iter = m_erabToBeSwitchedInDownlinkList.begin(); l_iter != m_erabToBeSwitchedInDownlinkList.end(); ++l_iter) // content of ErabToBeReleasedIndication // content of m_erabToBeSwitchedInDownlinkList
  {
    os << " ErabId " << l_iter->erabId;
    os << " enbTransportLayerAddress " << l_iter->enbTransportLayerAddress;
    os << " enbTeid " << l_iter->enbTeid;
  }

}

uint64_t 
NgcN2APPathSwitchRequestHeader::GetAmfUeN2Id () const 
{
  return m_amfUeN2Id;
}

void 
NgcN2APPathSwitchRequestHeader::SetAmfUeN2Id (uint64_t amfUeN2Id) 
{
  m_amfUeN2Id = amfUeN2Id;
}

uint16_t 
NgcN2APPathSwitchRequestHeader::GetEnbUeN2Id () const
{
  return m_enbUeN2Id;
}

void 
NgcN2APPathSwitchRequestHeader::SetEnbUeN2Id (uint16_t enbUeN2Id)
{
  m_enbUeN2Id = enbUeN2Id;
}

std::list<NgcN2apSap::ErabSwitchedInDownlinkItem>
NgcN2APPathSwitchRequestHeader::GetErabSwitchedInDownlinkItemList () const 
{
  return m_erabToBeSwitchedInDownlinkList;
}

void 
NgcN2APPathSwitchRequestHeader::SetErabSwitchedInDownlinkItemList (std::list<NgcN2apSap::ErabSwitchedInDownlinkItem> erabSetupList)
{
	m_headerLength += erabSetupList.size()*10;
  m_erabToBeSwitchedInDownlinkList = erabSetupList;
}

uint16_t
NgcN2APPathSwitchRequestHeader::GetEcgi() const
{
  return m_ecgi;
}

void
NgcN2APPathSwitchRequestHeader::SetEcgi(uint16_t ecgi) 
{
  m_ecgi = ecgi;
}

uint32_t
NgcN2APPathSwitchRequestHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcN2APPathSwitchRequestHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcN2APInitialContextSetupRequestHeader);

NgcN2APInitialContextSetupRequestHeader::NgcN2APInitialContextSetupRequestHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (9 + 3 + 9 + 4 + 1),
    m_enbUeN2Id (0xfffa),
    m_amfUeN2Id (0xfffffffa)
{
  m_erabsToBeSetupList.clear();
}

NgcN2APInitialContextSetupRequestHeader::~NgcN2APInitialContextSetupRequestHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeN2Id = 0xfffb;
  m_amfUeN2Id = 0xfffffffb;
  m_erabsToBeSetupList.clear();
}

TypeId
NgcN2APInitialContextSetupRequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2APInitialContextSetupRequestHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcN2APInitialContextSetupRequestHeader> ()
  ;
  return tid;
}

TypeId
NgcN2APInitialContextSetupRequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcN2APInitialContextSetupRequestHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcN2APInitialContextSetupRequestHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcN2APHeader
  i.WriteU64 (m_amfUeN2Id);         // amfUeN2Id
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteHtonU16 (m_enbUeN2Id);     // m_enbUeN2Id
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteHtonU64 (0);               // aggregate maximum bitrate, not implemented
  i.WriteU8 (0);

  std::list <NgcN2apSap::ErabToBeSetupItem>::size_type sz = m_erabsToBeSetupList.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcN2apSap::ErabToBeSetupItem>::const_iterator l_iter = m_erabsToBeSetupList.begin(); l_iter != m_erabsToBeSetupList.end(); ++l_iter) // content of m_erabsToBeSetupList
    {
      i.WriteU8 (l_iter->erabId);
      i.WriteHtonU16 (l_iter->erabLevelQosParameters.qci);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.gbrDl);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.gbrUl);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.mbrDl);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.mbrUl);
      i.WriteU8 (l_iter->erabLevelQosParameters.arp.priorityLevel);
      i.WriteU8 (l_iter->erabLevelQosParameters.arp.preemptionCapability);
      i.WriteU8 (l_iter->erabLevelQosParameters.arp.preemptionVulnerability);
      i.WriteHtonU32 (l_iter->transportLayerAddress.Get ());
      i.WriteHtonU32 (l_iter->smfTeid);

      i.WriteU8(0); // a criticaloty each, REJECT
    }
  i.WriteU8 (0);               // criticality = REJECT

  //TODO 9.2.140, 9.2.1.41

}

uint32_t
NgcN2APInitialContextSetupRequestHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_amfUeN2Id = i.ReadU64 ();         // amfUeN2Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_enbUeN2Id = i.ReadNtohU16 ();     // m_enbUeN2Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU64 ();               // aggregate maximum bitrate, not implemented
  i.ReadU8 ();
  m_headerLength += 9;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of m_erabToBeSetupList
  {
    NgcN2apSap::ErabToBeSetupItem erabItem;
    erabItem.erabId = i.ReadU8 ();
 
    erabItem.erabLevelQosParameters = EpsBearer ((EpsBearer::Qci) i.ReadNtohU16 ());
    erabItem.erabLevelQosParameters.gbrQosInfo.gbrDl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.gbrQosInfo.gbrUl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.gbrQosInfo.mbrDl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.gbrQosInfo.mbrUl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.arp.priorityLevel = i.ReadU8 ();
    erabItem.erabLevelQosParameters.arp.preemptionCapability = i.ReadU8 ();
    erabItem.erabLevelQosParameters.arp.preemptionVulnerability = i.ReadU8 ();

    erabItem.transportLayerAddress = Ipv4Address (i.ReadNtohU32 ());
    erabItem.smfTeid = i.ReadNtohU32 ();

    i.ReadU8 ();

    m_erabsToBeSetupList.push_back (erabItem);
    m_headerLength += 46;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;
  
  return GetSerializedSize();
}

void
NgcN2APInitialContextSetupRequestHeader::Print (std::ostream &os) const
{
  os << " AmfUeN2Id = " << m_amfUeN2Id;
  os << " EnbUeN2Id = " << m_enbUeN2Id;
  os << " NumOfBearers = " << m_erabsToBeSetupList.size ();

  std::list <NgcN2apSap::ErabToBeSetupItem>::size_type sz = m_erabsToBeSetupList.size ();
  if (sz > 0)
    {
      os << " [";
    }
  int j = 0;  
  for (std::list <NgcN2apSap::ErabToBeSetupItem>::const_iterator l_iter = m_erabsToBeSetupList.begin(); l_iter != m_erabsToBeSetupList.end(); ++l_iter) // content of m_erabsToBeSetupList
  {
    os << l_iter->erabId;
    if (j < (int) sz - 1)
      {
        os << ", ";
      }
    else
      {
        os << "]";
      }
    j++;  
  }
}

uint64_t 
NgcN2APInitialContextSetupRequestHeader::GetAmfUeN2Id () const 
{
  return m_amfUeN2Id;
}

void 
NgcN2APInitialContextSetupRequestHeader::SetAmfUeN2Id (uint64_t amfUeN2Id) 
{
  m_amfUeN2Id = amfUeN2Id;
}

uint16_t 
NgcN2APInitialContextSetupRequestHeader::GetEnbUeN2Id () const
{
  return m_enbUeN2Id;
}

void 
NgcN2APInitialContextSetupRequestHeader::SetEnbUeN2Id (uint16_t enbUeN2Id)
{
  m_enbUeN2Id = enbUeN2Id;
}
// jhlim
uint64_t 
NgcN2APInitialContextSetupRequestHeader::GetGuti () const
{
  return m_guti;
}

void 
NgcN2APInitialContextSetupRequestHeader::SetGuti (uint64_t guti)
{
  m_guti = guti;
}

std::list<NgcN2apSap::ErabToBeSetupItem>
NgcN2APInitialContextSetupRequestHeader::GetErabToBeSetupItem () const 
{
  return m_erabsToBeSetupList;
}

void 
NgcN2APInitialContextSetupRequestHeader::SetErabToBeSetupItem (std::list<NgcN2apSap::ErabToBeSetupItem> erabSetupList)
{
  m_headerLength += erabSetupList.size()*47;
  m_erabsToBeSetupList = erabSetupList;
}

uint32_t
NgcN2APInitialContextSetupRequestHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcN2APInitialContextSetupRequestHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}
/////////////////////////////////////////////////////////////////////


NS_OBJECT_ENSURE_REGISTERED (NgcN2APN2RequestHeader);

NgcN2APN2RequestHeader::NgcN2APN2RequestHeader ()
  : m_numberOfIes (1 + 1 + 1),
    m_headerLength (9 + 3 + 9 + 4 + 1),
    m_enbUeN2Id (0xfffa),
    m_amfUeN2Id (0xfffffffa)
{
  m_erabsToBeSetupList.clear();
}

NgcN2APN2RequestHeader::~NgcN2APN2RequestHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeN2Id = 0xfffb;
  m_amfUeN2Id = 0xfffffffb;
  m_erabsToBeSetupList.clear();
}

TypeId
NgcN2APN2RequestHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2APN2RequestHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcN2APN2RequestHeader> ()
  ;
  return tid;
}

TypeId
NgcN2APN2RequestHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcN2APN2RequestHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}

void
NgcN2APN2RequestHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcN2APHeader
  i.WriteU64 (m_amfUeN2Id);         // amfUeN2Id
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteHtonU16 (m_enbUeN2Id);     // m_enbUeN2Id
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  i.WriteHtonU64 (0);               // aggregate maximum bitrate, not implemented
  i.WriteU8 (0);

  std::list <NgcN2apSap::ErabToBeSetupItem>::size_type sz = m_erabsToBeSetupList.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcN2apSap::ErabToBeSetupItem>::const_iterator l_iter = m_erabsToBeSetupList.begin(); l_iter != m_erabsToBeSetupList.end(); ++l_iter) // content of m_erabsToBeSetupList
    {
      i.WriteU8 (l_iter->erabId);
      i.WriteHtonU16 (l_iter->erabLevelQosParameters.qci);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.gbrDl);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.gbrUl);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.mbrDl);
      i.WriteHtonU64 (l_iter->erabLevelQosParameters.gbrQosInfo.mbrUl);
      i.WriteU8 (l_iter->erabLevelQosParameters.arp.priorityLevel);
      i.WriteU8 (l_iter->erabLevelQosParameters.arp.preemptionCapability);
      i.WriteU8 (l_iter->erabLevelQosParameters.arp.preemptionVulnerability);
      i.WriteHtonU32 (l_iter->transportLayerAddress.Get ());
      i.WriteHtonU32 (l_iter->smfTeid);

      i.WriteU8(0); // a criticaloty each, REJECT
    }
  i.WriteU8 (0);               // criticality = REJECT

  //TODO 9.2.140, 9.2.1.41

}

uint32_t
NgcN2APN2RequestHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_amfUeN2Id = i.ReadU64 ();         // amfUeN2Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_enbUeN2Id = i.ReadNtohU16 ();     // m_enbUeN2Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU64 ();               // aggregate maximum bitrate, not implemented
  i.ReadU8 ();
  m_headerLength += 9;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of m_erabToBeSetupList
  {
    NgcN2apSap::ErabToBeSetupItem erabItem;
    erabItem.erabId = i.ReadU8 ();
 
    erabItem.erabLevelQosParameters = EpsBearer ((EpsBearer::Qci) i.ReadNtohU16 ());
    erabItem.erabLevelQosParameters.gbrQosInfo.gbrDl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.gbrQosInfo.gbrUl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.gbrQosInfo.mbrDl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.gbrQosInfo.mbrUl = i.ReadNtohU64 ();
    erabItem.erabLevelQosParameters.arp.priorityLevel = i.ReadU8 ();
    erabItem.erabLevelQosParameters.arp.preemptionCapability = i.ReadU8 ();
    erabItem.erabLevelQosParameters.arp.preemptionVulnerability = i.ReadU8 ();

    erabItem.transportLayerAddress = Ipv4Address (i.ReadNtohU32 ());
    erabItem.smfTeid = i.ReadNtohU32 ();

    i.ReadU8 ();

    m_erabsToBeSetupList.push_back (erabItem);
    m_headerLength += 46;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;
  
  return GetSerializedSize();
}

void
NgcN2APN2RequestHeader::Print (std::ostream &os) const
{
  os << " AmfUeN2Id = " << m_amfUeN2Id;
  os << " EnbUeN2Id = " << m_enbUeN2Id;
  os << " NumOfBearers = " << m_erabsToBeSetupList.size ();

  std::list <NgcN2apSap::ErabToBeSetupItem>::size_type sz = m_erabsToBeSetupList.size ();
  if (sz > 0)
    {
      os << " [";
    }
  int j = 0;  
  for (std::list <NgcN2apSap::ErabToBeSetupItem>::const_iterator l_iter = m_erabsToBeSetupList.begin(); l_iter != m_erabsToBeSetupList.end(); ++l_iter) // content of m_erabsToBeSetupList
  {
    os << l_iter->erabId;
    if (j < (int) sz - 1)
      {
        os << ", ";
      }
    else
      {
        os << "]";
      }
    j++;  
  }
}

uint64_t 
NgcN2APN2RequestHeader::GetAmfUeN2Id () const 
{
  return m_amfUeN2Id;
}

void 
NgcN2APN2RequestHeader::SetAmfUeN2Id (uint64_t amfUeN2Id) 
{
  m_amfUeN2Id = amfUeN2Id;
}

uint16_t 
NgcN2APN2RequestHeader::GetEnbUeN2Id () const
{
  return m_enbUeN2Id;
}

void 
NgcN2APN2RequestHeader::SetEnbUeN2Id (uint16_t enbUeN2Id)
{
  m_enbUeN2Id = enbUeN2Id;
}

std::list<NgcN2apSap::ErabToBeSetupItem>
NgcN2APN2RequestHeader::GetErabToBeSetupItem () const 
{
  return m_erabsToBeSetupList;
}

void 
NgcN2APN2RequestHeader::SetErabToBeSetupItem (std::list<NgcN2apSap::ErabToBeSetupItem> erabSetupList)
{
  m_headerLength += erabSetupList.size()*47;
  m_erabsToBeSetupList = erabSetupList;
}

uint32_t
NgcN2APN2RequestHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcN2APN2RequestHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}




/////////////////////////////////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (NgcN2APPathSwitchRequestAcknowledgeHeader);

NgcN2APPathSwitchRequestAcknowledgeHeader::NgcN2APPathSwitchRequestAcknowledgeHeader ()
  : m_numberOfIes (1 + 1 + 1 + 1),
    m_headerLength (3 + 4 + 1 + 9 + 3 + 6),
    m_enbUeN2Id (0xfffa),
    m_ecgi (0xfffa),
    m_amfUeN2Id (0xfffffffa)
{
  m_erabToBeSwitchedInUplinkList.clear();
}

NgcN2APPathSwitchRequestAcknowledgeHeader::~NgcN2APPathSwitchRequestAcknowledgeHeader ()
{ 
  m_numberOfIes = 0;
  m_headerLength = 0;
  m_enbUeN2Id = 0xfffb;
  m_ecgi = 0xfffb;
  m_amfUeN2Id = 0xfffffffb;
  m_erabToBeSwitchedInUplinkList.clear();
}

TypeId
NgcN2APPathSwitchRequestAcknowledgeHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcN2APPathSwitchRequestAcknowledgeHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NgcN2APPathSwitchRequestAcknowledgeHeader> ()
  ;
  return tid;
}

TypeId
NgcN2APPathSwitchRequestAcknowledgeHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NgcN2APPathSwitchRequestAcknowledgeHeader::GetSerializedSize (void) const
{
  return m_headerLength;
}


void
NgcN2APPathSwitchRequestAcknowledgeHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  // message type is already in NgcN2APHeader 
  i.WriteHtonU16 (m_enbUeN2Id);     // m_enbUeN2Id
  i.WriteU8 (0);               // criticality = REJECT

  std::vector <NgcN2apSap::ErabSwitchedInUplinkItem>::size_type sz = m_erabToBeSwitchedInUplinkList.size (); 
  i.WriteHtonU32 (sz);              // number of bearers
  for (std::list <NgcN2apSap::ErabSwitchedInUplinkItem>::const_iterator l_iter = m_erabToBeSwitchedInUplinkList.begin(); l_iter != m_erabToBeSwitchedInUplinkList.end(); ++l_iter) // content of m_erabsToBeSetupList
  {
    i.WriteU8 (l_iter->erabId);
    i.WriteHtonU32 (l_iter->transportLayerAddress.Get ());
    i.WriteHtonU32 (l_iter->enbTeid);
    i.WriteU8 (0);               // criticality = REJECT each
  }
  i.WriteU8 (0);               // criticality = REJECT

  i.WriteU64 (m_amfUeN2Id);         // amfUeN2Id
  i.WriteU8 (0);               // criticality = REJECT

  i.WriteU16 (m_ecgi);      // ecgi
  i.WriteU8 (1 << 6);       // criticality = IGNORE

  i.WriteU32 (0);                   // TAI, not implemented
  i.WriteU8 (0);
  i.WriteU8 (1 << 6);               // criticality = IGNORE

  // TODO add 9.2.1.40

}

uint32_t
NgcN2APPathSwitchRequestAcknowledgeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_headerLength = 0;
  m_numberOfIes = 0;

  m_enbUeN2Id = i.ReadNtohU16 ();     // m_enbUeN2Id
  i.ReadU8 ();           
  m_headerLength += 3;
  m_numberOfIes++;

  int sz = i.ReadNtohU32(); // number of bearers
  m_headerLength += 4;

  for (int j = 0; j < (int) sz; j++) // content of ErabToBeReleasedIndication
  {
    NgcN2apSap::ErabSwitchedInUplinkItem erabItem;
    erabItem.erabId = i.ReadU8 ();
    erabItem.transportLayerAddress = Ipv4Address (i.ReadNtohU32 ());
    erabItem.enbTeid = i.ReadNtohU32 ();
    i.ReadU8 ();

    m_erabToBeSwitchedInUplinkList.push_back(erabItem);
    m_headerLength += 9;
  }
  i.ReadU8();
  m_headerLength += 1;
  m_numberOfIes++;

  m_amfUeN2Id = i.ReadU64 ();         // amfUeN2Id
  i.ReadU8 ();                 
  m_headerLength += 9;
  m_numberOfIes++;

  m_ecgi = i.ReadU16 ();      // ecgi
  i.ReadU8();
  m_headerLength += 3;
  m_numberOfIes++;

  i.ReadU32 ();                   // TAI, not implemented
  i.ReadU8 ();
  i.ReadU8 ();               // criticality = IGNORE

  m_headerLength += 6;
  m_numberOfIes++;

  return GetSerializedSize();
}

void
NgcN2APPathSwitchRequestAcknowledgeHeader::Print (std::ostream &os) const
{
  os << "AmfUeN2apId = " << m_amfUeN2Id;
  os << " EnbUeN2Id = " << m_enbUeN2Id;
  os << " ECGI = " << m_ecgi; 
  for (std::list <NgcN2apSap::ErabSwitchedInUplinkItem>::const_iterator l_iter = m_erabToBeSwitchedInUplinkList.begin(); l_iter != m_erabToBeSwitchedInUplinkList.end(); ++l_iter) // content of m_erabsToBeSetupList
  {
    os << " ErabId " << l_iter->erabId;
    os << " TransportLayerAddress " << l_iter->transportLayerAddress;
    os << " enbTeid " << l_iter->enbTeid;
  }

}

uint64_t 
NgcN2APPathSwitchRequestAcknowledgeHeader::GetAmfUeN2Id () const 
{
  return m_amfUeN2Id;
}

void 
NgcN2APPathSwitchRequestAcknowledgeHeader::SetAmfUeN2Id (uint64_t amfUeN2Id) 
{
  m_amfUeN2Id = amfUeN2Id;
}

uint16_t 
NgcN2APPathSwitchRequestAcknowledgeHeader::GetEnbUeN2Id () const
{
  return m_enbUeN2Id;
}

void 
NgcN2APPathSwitchRequestAcknowledgeHeader::SetEnbUeN2Id (uint16_t enbUeN2Id)
{
  m_enbUeN2Id = enbUeN2Id;
}

std::list<NgcN2apSap::ErabSwitchedInUplinkItem>
NgcN2APPathSwitchRequestAcknowledgeHeader::GetErabSwitchedInUplinkItemList () const 
{
  return m_erabToBeSwitchedInUplinkList;
}

void 
NgcN2APPathSwitchRequestAcknowledgeHeader::SetErabSwitchedInUplinkItemList (std::list<NgcN2apSap::ErabSwitchedInUplinkItem> erabSetupList)
{
  m_headerLength += erabSetupList.size() * 10;
  m_erabToBeSwitchedInUplinkList = erabSetupList;
}

uint16_t
NgcN2APPathSwitchRequestAcknowledgeHeader::GetEcgi() const
{
  return m_ecgi;
}

void
NgcN2APPathSwitchRequestAcknowledgeHeader::SetEcgi(uint16_t ecgi) 
{
  m_ecgi = ecgi;
}

uint32_t
NgcN2APPathSwitchRequestAcknowledgeHeader::GetLengthOfIes () const
{
  return m_headerLength;
}

uint32_t
NgcN2APPathSwitchRequestAcknowledgeHeader::GetNumberOfIes () const
{
  return m_numberOfIes;
}

/////////////////////////////////////////////////////////////////////

}; // end of namespace ns3
