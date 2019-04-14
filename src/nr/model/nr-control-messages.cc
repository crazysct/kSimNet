/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 *         Marco Miozzo <marco.miozzo@cttc.es>
 */

#include "nr-control-messages.h"
#include "ns3/address-utils.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "nr-net-device.h"
#include "nr-ue-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrControlMessage");

NrControlMessage::NrControlMessage (void)
{
}


NrControlMessage::~NrControlMessage (void)
{
}


void
NrControlMessage::SetMessageType (NrControlMessage::MessageType type)
{
  m_type = type;
}


NrControlMessage::MessageType
NrControlMessage::GetMessageType (void)
{
  return m_type;
}


// ----------------------------------------------------------------------------------------------------------


DlDciNrControlMessage::DlDciNrControlMessage (void)
{
  SetMessageType (NrControlMessage::DL_DCI);
}


DlDciNrControlMessage::~DlDciNrControlMessage (void)
{

}

void
DlDciNrControlMessage::SetDci (NrDlDciListElement_s dci)
{
  m_dci = dci;

}


NrDlDciListElement_s
DlDciNrControlMessage::GetDci (void)
{
  return m_dci;
}


// ----------------------------------------------------------------------------------------------------------


UlDciNrControlMessage::UlDciNrControlMessage (void)
{
  SetMessageType (NrControlMessage::UL_DCI);
}


UlDciNrControlMessage::~UlDciNrControlMessage (void)
{

}

void
UlDciNrControlMessage::SetDci (NrUlDciListElement_s dci)
{
  m_dci = dci;

}


NrUlDciListElement_s
UlDciNrControlMessage::GetDci (void)
{
  return m_dci;
}


// ----------------------------------------------------------------------------------------------------------


DlCqiNrControlMessage::DlCqiNrControlMessage (void)
{
  SetMessageType (NrControlMessage::DL_CQI);
}


DlCqiNrControlMessage::~DlCqiNrControlMessage (void)
{

}

void
DlCqiNrControlMessage::SetDlCqi (NrCqiListElement_s dlcqi)
{
  m_dlCqi = dlcqi;

}


NrCqiListElement_s
DlCqiNrControlMessage::GetDlCqi (void)
{
  return m_dlCqi;
}



// ----------------------------------------------------------------------------------------------------------


BsrNrControlMessage::BsrNrControlMessage (void)
{
  SetMessageType (NrControlMessage::BSR);
}


BsrNrControlMessage::~BsrNrControlMessage (void)
{

}

void
BsrNrControlMessage::SetBsr (NrMacCeListElement_s bsr)
{
  m_bsr = bsr;

}


NrMacCeListElement_s
BsrNrControlMessage::GetBsr (void)
{
  return m_bsr;
}



// ----------------------------------------------------------------------------------------------------------


RachPreambleNrControlMessage::RachPreambleNrControlMessage (void)
{
  SetMessageType (NrControlMessage::RACH_PREAMBLE);
}

void
RachPreambleNrControlMessage::SetRapId (uint32_t rapId)
{
  m_rapId = rapId;
}

uint32_t 
RachPreambleNrControlMessage::GetRapId () const
{
  return m_rapId;
}


// ----------------------------------------------------------------------------------------------------------


RarNrControlMessage::RarNrControlMessage (void)
{
  SetMessageType (NrControlMessage::RAR);
}


void
RarNrControlMessage::SetRaRnti (uint16_t raRnti)
{
  m_raRnti = raRnti;
}

uint16_t 
RarNrControlMessage::GetRaRnti () const
{
  return m_raRnti;
}


void
RarNrControlMessage::AddRar (Rar rar)
{
  m_rarList.push_back (rar);
}

std::list<RarNrControlMessage::Rar>::const_iterator 
RarNrControlMessage::RarListBegin () const
{
  return m_rarList.begin ();
}

std::list<RarNrControlMessage::Rar>::const_iterator 
RarNrControlMessage::RarListEnd () const
{
  return m_rarList.end ();
}


// ----------------------------------------------------------------------------------------------------------



MibNrControlMessage::MibNrControlMessage (void)
{
  SetMessageType (NrControlMessage::MIB);
}


void
MibNrControlMessage::SetMib (NrRrcSap::MasterInformationBlock  mib)
{
  m_mib = mib;
}

NrRrcSap::MasterInformationBlock 
MibNrControlMessage::GetMib () const
{
  return m_mib;
}


// ----------------------------------------------------------------------------------------------------------



Sib1NrControlMessage::Sib1NrControlMessage (void)
{
  SetMessageType (NrControlMessage::SIB1);
}


void
Sib1NrControlMessage::SetSib1 (NrRrcSap::SystemInformationBlockType1 sib1)
{
  m_sib1 = sib1;
}

NrRrcSap::SystemInformationBlockType1
Sib1NrControlMessage::GetSib1 () const
{
  return m_sib1;
}


// ---------------------------------------------------------------------------



DlHarqFeedbackNrControlMessage::DlHarqFeedbackNrControlMessage (void)
{
  SetMessageType (NrControlMessage::DL_HARQ);
}


DlHarqFeedbackNrControlMessage::~DlHarqFeedbackNrControlMessage (void)
{

}

void
DlHarqFeedbackNrControlMessage::SetDlHarqFeedback (NrDlInfoListElement_s m)
{
  m_dlInfoListElement = m;
}


NrDlInfoListElement_s
DlHarqFeedbackNrControlMessage::GetDlHarqFeedback (void)
{
  return m_dlInfoListElement;
}


} // namespace ns3

