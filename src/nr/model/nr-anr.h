/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 * Copyright (c) 2013 Budiarto Herman
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
 * Original work authors (from nr-enb-rrc.cc):
 * - Nicola Baldo <nbaldo@cttc.es>
 * - Marco Miozzo <mmiozzo@cttc.es>
 * - Manuel Requena <manuel.requena@cttc.es>
 *
 * Converted to ANR interface by:
 * - Budiarto Herman <budiarto.herman@magister.fi>
 */

#ifndef NR_ANR_H
#define NR_ANR_H

#include <ns3/object.h>
#include <ns3/nr-rrc-sap.h>
#include <ns3/nr-anr-sap.h>
#include <map>

namespace ns3 {


class NrAnrSapProvider;
class NrAnrSapUser;
class NrNeighbourRelation;

/**
 * \brief Automatic Neighbour Relation function.
 *
 * ANR is a conceptually a list of neighbouring cells called the Neighbour
 * Relation Table (NRT). ANR has the capability of automatically inserting new
 * entries into NRT based on measurement reports obtained from the eNodeB RRC
 * instance. Besides this, ANR also supports manual insertion and accepts
 * queries for the NRT content.
 *
 * The NrHelper class automatically installs one ANR instance for each eNodeB
 * RRC instance. When installed, ANR will assist the eNodeB RRC's handover
 * function, e.g., by preventing an X2-based handover execution if there is no
 * X2 interface to the target neighbour cell. If this is not desired, it can be
 * disabled by the following code sample:
 *
 *     Config::SetDefault ("ns3::NrHelper::AnrEnabled", BooleanValue (false));
 *     Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
 *
 * The communication between an ANR instance and the eNodeB RRC instance is done
 * through the *ANR SAP* interface. The ANR instance corresponds to the
 * "provider" part of this interface, while the eNodeB RRC instance takes the
 * role of the "user" part. The following code skeleton establishes the
 * connection between both instances:
 *
 *     Ptr<NrEnbRrc> u = ...;
 *     Ptr<NrAnr> p = ...;
 *     u->SetNrAnrSapProvider (p->GetNrAnrSapProvider ());
 *     p->SetNrAnrSapUser (u->GetNrAnrSapUser ());
 *
 * However, user rarely needs to use the above code, since it has already been
 * taken care by NrHelper::InstallEnbDevice.
 *
 * The current ANR model is inspired from Section 22.3.2a and 22.3.3 of 3GPP
 * TS 36.300.
 *
 * \sa SetNrAnrSapProvider, SetNrAnrSapUser
 */
class NrAnr : public Object
{
public:
  /**
   * \brief Creates an ANR instance.
   * \param servingCellId the cell ID of the eNodeB instance whom this ANR
   *                      instance is to be associated with
   */
  NrAnr (uint16_t servingCellId);
  virtual ~NrAnr ();

  // inherited from Object
  static TypeId GetTypeId ();

  /**
   * \brief Provide an advance information about a related neighbouring cell
   *        and add it as a new Neighbour Relation entry.
   * \param cellId the cell ID of the new neighbour
   *
   * This function simulates the Neighbour Relation addition operation by
   * network operations and maintenance, as depicted in Section 22.3.2a of
   * 3GPP TS 36.300.
   *
   * An entry added by this function will have the NoRemove flag set to TRUE and
   * the NoHo flag set to TRUE. Hence, the cell may not act as the target cell
   * of a handover, unless a measurement report of the cell is received, which
   * will update the NoHo flag to FALSE.
   */
  void AddNeighbourRelation (uint16_t cellId);

  /**
   * \brief Remove an existing Neighbour Relation entry.
   * \param cellId the cell ID to be removed from the NRT
   *
   * This function simulates the Neighbour Relation removal operation by
   * network operations and maintenance, as depicted in Section 22.3.2a of
   * 3GPP TS 36.300.
   */
  void RemoveNeighbourRelation (uint16_t cellId);

  /**
   * \brief Set the "user" part of the ANR SAP interface that this ANR instance
   *        will interact with.
   * \param s a reference to the "user" part of the interface, typically a
   *          member of an NrEnbRrc instance
   */
  virtual void SetNrAnrSapUser (NrAnrSapUser* s);

  /**
   * \brief Export the "provider" part of the ANR SAP interface.
   * \return the reference to the "provider" part of the interface, typically to
   *         be kept by an NrEnbRrc instance
   */
  virtual NrAnrSapProvider* GetNrAnrSapProvider ();

  // let the forwarder class access the protected and private members
  friend class MemberNrAnrSapProvider<NrAnr>;

protected:
  // inherited from Object
  virtual void DoInitialize ();
  virtual void DoDispose ();

private:

  // ANR SAP PROVIDER IMPLEMENTATION

  /**
   * \brief Implementation of NrAnrSapProvider::ReportUeMeas.
   * \param measResults a single report of one measurement identity
   */
  void DoReportUeMeas (NrRrcSap::MeasResults measResults);

  /**
   * \brief Implementation of NrAnrSapProvider::AddNeighbourRelation.
   * \param cellId the Physical Cell ID of the new neighbouring cell
   */
  void DoAddNeighbourRelation (uint16_t cellId);

  /**
   * \brief Implementation of NrAnrSapProvider::GetNoRemove.
   * \param cellId the Physical Cell ID of the neighbouring cell of interest
   * \return if true, the Neighbour Relation shall *not* be removed from the NRT
   */
  bool DoGetNoRemove (uint16_t cellId) const;

  /**
   * \brief Implementation of NrAnrSapProvider::GetNoHo.
   * \param cellId the Physical Cell ID of the neighbouring cell of interest
   * \return if true, the Neighbour Relation shall *not* be used by the eNodeB
   *         for handover reasons
   */
  bool DoGetNoHo (uint16_t cellId) const;

  /**
   * \brief Implementation of NrAnrSapProvider::GetNoX2.
   * \param cellId the Physical Cell ID of the neighbouring cell of interest
   * \return if true, the Neighbour Relation shall *not* use an X2 interface in
   *         order to initiate procedures towards the eNodeB parenting the
   *         target cell
   */
  bool DoGetNoX2 (uint16_t cellId) const;

  // ANR SAP

  /**
   * \brief Reference to the "provider" part of the ANR SAP interface, which is
   *        automatically created when this class instantiates.
   */
  NrAnrSapProvider* m_anrSapProvider;

  /**
   * \brief Reference to the "user" part of the ANR SAP interface, which is
   *        provided by the eNodeB RRC instance.
   */
  NrAnrSapUser* m_anrSapUser;

  // ATTRIBUTE

  /// The attribute Threshold.
  uint8_t m_threshold;

  /**
   * \brief Neighbour Relation between two eNodeBs (serving eNodeB and neighbour
   *        eNodeB).
   */
  struct NeighbourRelation_t
  {
    bool noRemove;
    bool noHo;
    bool noX2;
    bool detectedAsNeighbour;
  };

  //               cellId
  typedef std::map<uint16_t, NeighbourRelation_t> NeighbourRelationTable_t;

  NeighbourRelationTable_t m_neighbourRelationTable;

  // internal methods
  const NeighbourRelation_t* Find (uint16_t cellId) const;

  // The expected measurement identity
  uint8_t m_measId;

  uint16_t m_servingCellId;

}; // end of class NrAnr


} // end of namespace ns3


#endif /* NR_ANR_H */
