/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es>,
 *         Marco Miozzo <mmiozzo@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#ifndef NR_ENB_CPHY_SAP_H
#define NR_ENB_CPHY_SAP_H

#include <stdint.h>
#include <map>
#include <ns3/ptr.h>

#include <ns3/nr-rrc-sap.h>

namespace ns3 {

class NrEnbNetDevice;

/**
 * Service Access Point (SAP) offered by the UE PHY to the UE RRC for control purposes
 *
 * This is the PHY SAP Provider, i.e., the part of the SAP that contains
 * the PHY methods called by the MAC
 */
class NrEnbCphySapProvider
{
public:

  /** 
   * destructor
   */
  virtual ~NrEnbCphySapProvider ();

  /** 
   * 
   * 
   * \param cellId the Cell Identifier
   */
  virtual void SetCellId (uint16_t cellId) = 0;

  /**
   * \param ulBandwidth the UL bandwidth in PRBs
   * \param dlBandwidth the DL bandwidth in PRBs
   */
  virtual void SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth) = 0;

  /**
   * \param ulEarfcn the UL EARFCN
   * \param dlEarfcn the DL EARFCN
   */
  virtual void SetEarfcn (uint16_t ulEarfcn, uint16_t dlEarfcn) = 0;
  
  /** 
   * Add a new UE to the cell
   * 
   * \param rnti the UE id relative to this cell
   */
  virtual void AddUe (uint16_t rnti) = 0;

  /** 
   * Remove an UE from the the cell
   * 
   * \param rnti the UE id relative to this cell
   */
  virtual void RemoveUe (uint16_t rnti) = 0;
  
  /**
   * Set the UE transmission power offset P_A
   *
   * \param rnti the UE id relative to this cell
   * \param pa transmission power offset
   */
  virtual void SetPa (uint16_t rnti, double pa) = 0;

  /**
   * \param rnti the RNTI of the user
   * \param txMode the transmissionMode of the user
   */
  virtual void SetTransmissionMode (uint16_t rnti, uint8_t txMode) = 0;

  /**
   * \param rnti the RNTI of the user
   * \param srsCi the SRS Configuration Index of the user
   */
  virtual void SetSrsConfigurationIndex (uint16_t rnti, uint16_t srsCi) = 0;

  /** 
   * 
   * \param mib the Master Information Block to be sent on the BCH
   */
  virtual void SetMasterInformationBlock (NrRrcSap::MasterInformationBlock mib) = 0;

  /**
   *
   * \param sib1 the System Information Block Type 1 to be sent on the BCH
   */
  virtual void SetSystemInformationBlockType1 (NrRrcSap::SystemInformationBlockType1 sib1) = 0;

  /**
   *
   * \return Reference Signal Power for SIB2
   */
  virtual int8_t GetReferenceSignalPower () = 0;
};


/**
 * Service Access Point (SAP) offered by the UE PHY to the UE RRC for control purposes
 *
 * This is the CPHY SAP User, i.e., the part of the SAP that contains the RRC
 * methods called by the PHY
*/
class NrEnbCphySapUser
{
public:
  
  /** 
   * destructor
   */
  virtual ~NrEnbCphySapUser ();

  struct UeAssociatedSinrInfo
  {
    std::map<uint64_t, double> ueImsiSinrMap;
  };

  virtual void UpdateUeSinrEstimate(NrEnbCphySapUser::UeAssociatedSinrInfo info) = 0;

};


/**
 * Template for the implementation of the NrEnbCphySapProvider as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNrEnbCphySapProvider : public NrEnbCphySapProvider
{
public:
  MemberNrEnbCphySapProvider (C* owner);

  // inherited from NrEnbCphySapProvider
  virtual void SetCellId (uint16_t cellId);
  virtual void SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth);
  virtual void SetEarfcn (uint16_t ulEarfcn, uint16_t dlEarfcn);
  virtual void AddUe (uint16_t rnti);
  virtual void RemoveUe (uint16_t rnti);
  virtual void SetPa (uint16_t rnti, double pa);
  virtual void SetTransmissionMode (uint16_t  rnti, uint8_t txMode);
  virtual void SetSrsConfigurationIndex (uint16_t  rnti, uint16_t srsCi);
  virtual void SetMasterInformationBlock (NrRrcSap::MasterInformationBlock mib);
  virtual void SetSystemInformationBlockType1 (NrRrcSap::SystemInformationBlockType1 sib1);
  virtual int8_t GetReferenceSignalPower ();
  
private:
  MemberNrEnbCphySapProvider ();
  C* m_owner;
};

template <class C>
MemberNrEnbCphySapProvider<C>::MemberNrEnbCphySapProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNrEnbCphySapProvider<C>::MemberNrEnbCphySapProvider ()
{
}

template <class C>
void 
MemberNrEnbCphySapProvider<C>::SetCellId (uint16_t cellId)
{
  m_owner->DoSetCellId (cellId);
}


template <class C>
void 
MemberNrEnbCphySapProvider<C>::SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  m_owner->DoSetBandwidth (ulBandwidth, dlBandwidth);
}

template <class C>
void 
MemberNrEnbCphySapProvider<C>::SetEarfcn (uint16_t ulEarfcn, uint16_t dlEarfcn)
{
  m_owner->DoSetEarfcn (ulEarfcn, dlEarfcn);
}

template <class C>
void 
MemberNrEnbCphySapProvider<C>::AddUe (uint16_t rnti)
{
  m_owner->DoAddUe (rnti);
}

template <class C>
void 
MemberNrEnbCphySapProvider<C>::RemoveUe (uint16_t rnti)
{
  m_owner->DoRemoveUe (rnti);
}

template <class C>
void 
MemberNrEnbCphySapProvider<C>::SetPa (uint16_t rnti, double pa)
{
  m_owner->DoSetPa (rnti, pa);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::SetTransmissionMode (uint16_t  rnti, uint8_t txMode)
{
  m_owner->DoSetTransmissionMode (rnti, txMode);
}

template <class C>
void 
MemberNrEnbCphySapProvider<C>::SetSrsConfigurationIndex (uint16_t  rnti, uint16_t srsCi)
{
  m_owner->DoSetSrsConfigurationIndex (rnti, srsCi);
}

template <class C> 
void 
MemberNrEnbCphySapProvider<C>::SetMasterInformationBlock (NrRrcSap::MasterInformationBlock mib)
{
  m_owner->DoSetMasterInformationBlock (mib);
}

template <class C>
void
MemberNrEnbCphySapProvider<C>::SetSystemInformationBlockType1 (NrRrcSap::SystemInformationBlockType1 sib1)
{
  m_owner->DoSetSystemInformationBlockType1 (sib1);
}

template <class C>
int8_t
MemberNrEnbCphySapProvider<C>::GetReferenceSignalPower ()
{
  return m_owner->DoGetReferenceSignalPower ();
}

/**
 * Template for the implementation of the NrEnbCphySapUser as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class MemberNrEnbCphySapUser : public NrEnbCphySapUser
{
public:
  MemberNrEnbCphySapUser (C* owner);
  virtual void UpdateUeSinrEstimate(UeAssociatedSinrInfo info);


  // methods inherited from NrEnbCphySapUser go here

private:
  MemberNrEnbCphySapUser ();
  C* m_owner;
};

template <class C>
MemberNrEnbCphySapUser<C>::MemberNrEnbCphySapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
MemberNrEnbCphySapUser<C>::MemberNrEnbCphySapUser ()
{
}

template <class C>
void
MemberNrEnbCphySapUser<C>::UpdateUeSinrEstimate(UeAssociatedSinrInfo info)
{
  return m_owner->DoUpdateUeSinrEstimate(info);
}







} // namespace ns3


#endif // NR_ENB_CPHY_SAP_H
