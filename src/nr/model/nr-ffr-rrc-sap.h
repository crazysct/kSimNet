/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Piotr Gawlowicz
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
 * Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>
 *
 */

#ifndef NR_FFR_RRC_SAP_H
#define NR_FFR_RRC_SAP_H

#include <ns3/nr-rrc-sap.h>
#include <ns3/ngc-x2-sap.h>

namespace ns3 {

/**
 * \brief Service Access Point (SAP) offered by the Frequency Reuse algorithm
 *        instance to the eNodeB RRC instance.
 *
 * This is the *NrFfrRrcSapProvider*, i.e., the part of the SAP
 * that contains the Frequency Reuse algorithm methods called by the eNodeB RRC
 * instance.
 */
class NrFfrRrcSapProvider
{
public:
  virtual ~NrFfrRrcSapProvider ();

  /**
   * \brief SetCellId
   * \param cellId the Cell Identifier
   */
  virtual void SetCellId (uint16_t cellId ) = 0;

  /**
   * \brief Configure DL and UL bandwidth in Frequency Reuse Algorithm
   *        function is called during Cell configuration
   * \param ulBandwidth UL bandwidth in number of RB
   * \param dlBandwidth DL bandwidth in number of RB
   */
  virtual void SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth) = 0;

  /**
   * \brief Send a UE measurement report to Frequency Reuse algorithm.
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   * \param measResults a single report of one measurement identity
   *
   * The received measurement report is a result of the UE measurement
   * configuration previously configured by calling
   * NrFfrRrcSapUser::AddUeMeasReportConfigForFfr. The report
   * may be stored and utilised for the purpose of making decisions within which
   * sub-band UE should be served.
   */
  virtual void ReportUeMeas (uint16_t rnti,
                             NrRrcSap::MeasResults measResults) = 0;

  /**
   * \brief RecvLoadInformation
   */
  virtual void  RecvLoadInformation (NgcX2Sap::LoadInformationParams params) = 0;

}; // end of class NrFfrRrcSapProvider


/**
 * \brief Service Access Point (SAP) offered by the eNodeB RRC instance to the
 *        Frequency Reuse algorithm instance.
 *
 * This is the *NrFfrRrcSapUser*, i.e., the part of the SAP that
 * contains the eNodeB RRC methods called by the Frequency Reuse algorithm instance.
 */
class NrFfrRrcSapUser
{
public:
  virtual ~NrFfrRrcSapUser ();

  /**
   * \brief Request a certain reporting configuration to be fulfilled by the UEs
   *        attached to the eNodeB entity.
   * \param reportConfig the UE measurement reporting configuration
   * \return the measurement identity associated with this newly added
   *         reporting configuration
   *
   * The eNodeB RRC entity is expected to configure the same reporting
   * configuration in each of the attached UEs. When later in the simulation a
   * UE measurement report is received from a UE as a result of this
   * configuration, the eNodeB RRC entity shall forward this report to the
   * Frequency Reuse algorithm through the NrFfrRrcSapProvider::ReportUeMeas
   * SAP function.
   *
   * \note This function is only valid before the simulation begins.
   */
  virtual uint8_t AddUeMeasReportConfigForFfr (NrRrcSap::ReportConfigEutra reportConfig) = 0;

  /**
   * \brief Instruct the eNodeB RRC entity to perform RrcConnectionReconfiguration
   *        to inform UE about new PdschConfigDedicated (i.e. P_a value).
   *        Also Downlink Power Allocation is done based on this value.
   * \param rnti Radio Network Temporary Identity, an integer identifying the
   *             UE which shall perform the handover
   * \param pdschConfigDedicated new PdschConfigDedicated to be configured for UE
   *
   * This function is used by the Frequency Reuse algorithm entity when it decides
   * that PDSCH for this UE should be allocated with different transmit power.
   *
   * The process to produce the decision is up to the implementation of Frequency Reuse
   * algorithm. It is typically based on the reported UE measurements, which are
   * received through the NrFfrRrcSapProvider::ReportUeMeas function.
   */
  virtual void SetPdschConfigDedicated (uint16_t rnti, NrRrcSap::PdschConfigDedicated pdschConfigDedicated) = 0;

  /**
   * \brief SendLoadInformation
   */
  virtual void  SendLoadInformation (NgcX2Sap::LoadInformationParams params) = 0;

}; // end of class NrFfrRrcSapUser


/**
 * \brief Template for the implementation of the NrFfrRrcSapProvider
 *        as a member of an owner class of type C to which all methods are
 *        forwarded.
 */
template <class C>
class MemberNrFfrRrcSapProvider : public NrFfrRrcSapProvider
{
public:
  MemberNrFfrRrcSapProvider (C* owner);

  // inherited from NrHandoverManagemenrSapProvider
  virtual void SetCellId (uint16_t cellId );
  virtual void SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth);
  virtual void ReportUeMeas (uint16_t rnti, NrRrcSap::MeasResults measResults);
  virtual void RecvLoadInformation (NgcX2Sap::LoadInformationParams params);

private:
  MemberNrFfrRrcSapProvider ();
  C* m_owner;

}; // end of class MemberNrFfrRrcSapProvider


template <class C>
MemberNrFfrRrcSapProvider<C>::MemberNrFfrRrcSapProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
void
MemberNrFfrRrcSapProvider<C>::SetCellId (uint16_t cellId )
{
  m_owner->DoSetCellId (cellId);
}

template <class C>
void
MemberNrFfrRrcSapProvider<C>::SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  m_owner->DoSetBandwidth (ulBandwidth, dlBandwidth);
}

template <class C>
void
MemberNrFfrRrcSapProvider<C>::ReportUeMeas (uint16_t rnti, NrRrcSap::MeasResults measResults)
{
  m_owner->DoReportUeMeas (rnti, measResults);
}

template <class C>
void
MemberNrFfrRrcSapProvider<C>::RecvLoadInformation (NgcX2Sap::LoadInformationParams params)
{
  m_owner->DoRecvLoadInformation (params);
}

/**
 * \brief Template for the implementation of the NrFfrRrcSapUser
 *        as a member of an owner class of type C to which all methods are
 *        forwarded.
 */
template <class C>
class MemberNrFfrRrcSapUser : public NrFfrRrcSapUser
{
public:
  MemberNrFfrRrcSapUser (C* owner);

  // inherited from NrFfrRrcSapUser
  virtual uint8_t AddUeMeasReportConfigForFfr (NrRrcSap::ReportConfigEutra reportConfig);

  virtual void SetPdschConfigDedicated (uint16_t rnti, NrRrcSap::PdschConfigDedicated pdschConfigDedicated);

  virtual void  SendLoadInformation (NgcX2Sap::LoadInformationParams params);
private:
  MemberNrFfrRrcSapUser ();
  C* m_owner;

}; // end of class NrFfrRrcSapUser

template <class C>
MemberNrFfrRrcSapUser<C>::MemberNrFfrRrcSapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
uint8_t
MemberNrFfrRrcSapUser<C>::AddUeMeasReportConfigForFfr (NrRrcSap::ReportConfigEutra reportConfig)
{
  return m_owner->DoAddUeMeasReportConfigForFfr (reportConfig);
}

template <class C>
void
MemberNrFfrRrcSapUser<C>::SetPdschConfigDedicated (uint16_t rnti, NrRrcSap::PdschConfigDedicated pdschConfigDedicated)
{
  m_owner->DoSetPdschConfigDedicated (rnti, pdschConfigDedicated);
}

template <class C>
void
MemberNrFfrRrcSapUser<C>::SendLoadInformation (NgcX2Sap::LoadInformationParams params)
{
  m_owner->DoSendLoadInformation (params);
}

} // end of namespace ns3


#endif /* NR_FFR_RRC_SAP_H */
