/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#include "ns3/log.h"

#include "ns3/nr-pdcp-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrPdcpHeader");

NS_OBJECT_ENSURE_REGISTERED (NrPdcpHeader);

NrPdcpHeader::NrPdcpHeader ()
  : m_dcBit (0xff),
    m_sequenceNumber (0xfffa)
{
}

NrPdcpHeader::~NrPdcpHeader ()
{
  m_dcBit = 0xff;
  m_sequenceNumber = 0xfffb;
}

void
NrPdcpHeader::SetDcBit (uint8_t dcBit)
{
  m_dcBit = dcBit & 0x01;
}

void
NrPdcpHeader::SetSequenceNumber (uint16_t sequenceNumber)
{
  m_sequenceNumber = sequenceNumber & 0x7FFF;
}

uint8_t
NrPdcpHeader::GetDcBit () const
{
  return m_dcBit;
}

uint16_t
NrPdcpHeader::GetSequenceNumber () const
{
  return m_sequenceNumber;
}
uint8_t
NrPdcpHeader::GetSourceCellId() const
{
	return sourceCellId;
}
void
NrPdcpHeader::SetSourceCellId (uint8_t sourceCellId){
	this->sourceCellId =sourceCellId;
}

TypeId
NrPdcpHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrPdcpHeader")
    .SetParent<Header> ()
    .SetGroupName("Nr")
    .AddConstructor<NrPdcpHeader> ()
  ;
  return tid;
}

TypeId
NrPdcpHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void NrPdcpHeader::Print (std::ostream &os)  const
{
  os << "D/C=" << (uint16_t)m_dcBit;
  os << " SN=" << m_sequenceNumber;
  os << " Source Cell Id = " <<sourceCellId;
}

uint32_t NrPdcpHeader::GetSerializedSize (void) const
{
  return 3;
}

void NrPdcpHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;

  i.WriteU8 ( (m_dcBit << 7) | (m_sequenceNumber & 0x7F00) >> 8 );
  i.WriteU8 ( (uint8_t)(m_sequenceNumber & 0x00FF) );
  i.WriteU8(sourceCellId); //sjkang1116
}

uint32_t NrPdcpHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t byte_1;
  uint8_t byte_2;

  byte_1 = i.ReadU8 ();
  byte_2 = i.ReadU8 ();
  m_dcBit = (byte_1 & 0x80) > 7;
  
  // HACKED. ENABLE THIS TO DISTINGUISH DATA_PDU and CONTROL_PDU in nr-enb-rrc.cc, lossless HO func
  //NS_ASSERT (m_dcBit == DATA_PDU);

  m_sequenceNumber = ((byte_1 & 0x7F) << 8) | byte_2;
  sourceCellId = i.ReadU8(); //sjkang1116

  return GetSerializedSize ();
}

}; // namespace ns3
