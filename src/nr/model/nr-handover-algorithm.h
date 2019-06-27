/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#ifndef NR_HANDOVER_ALGORITHM_H
#define NR_HANDOVER_ALGORITHM_H

#include <ns3/object.h>
#include <ns3/nr-rrc-sap.h>

namespace ns3 {


class NrHandoverManagementSapUser;
class NrHandoverManagementSapProvider;


/**
 * \brief The abstract base class of a handover algorithm that operates using
 *        the Handover Management SAP interface.
 *
 * Handover algorithm receives measurement reports from an eNode RRC instance
 * and tells the eNodeB RRC instance when to do a handover.
 *
 * This class is an abstract class intended to be inherited by subclasses that
 * implement its virtual methods. By inheriting from this abstract class, the
 * subclasses gain the benefits of being compatible with the NrEnbNetDevice
 * class, being accessible using namespace-based access through ns-3 Config
 * subsystem, and being installed and configured by NrHelper class (see
 * NrHelper::SetHandoverAlgorithmType and
 * NrHelper::SetHandoverAlgorithmAttribute methods).
 *
 * The communication with the eNodeB RRC instance is done through the *Handover
 * Management SAP* interface. The handover algorithm instance corresponds to the
 * "provider" part of this interface, while the eNodeB RRC instance takes the
 * role of the "user" part. The following code skeleton establishes the
 * connection between both instances:
 *
 *     Ptr<NrEnbRrc> u = ...;
 *     Ptr<NrHandoverAlgorithm> p = ...;
 *     u->SetNrHandoverManagementSapProvider (p->GetNrHandoverManagementSapProvider ());
 *     p->SetNrHandoverManagementSapUser (u->GetNrHandoverManagementSapUser ());
 *
 * However, user rarely needs to use the above code, since it has already been
 * taken care by NrHelper::InstallEnbDevice.
 *
 * \sa NrHandoverManagementSapProvider, NrHandoverManagementSapUser
 */
class NrHandoverAlgorithm : public Object
{
public:
  NrHandoverAlgorithm ();
  virtual ~NrHandoverAlgorithm ();

  // inherited from Object
  static TypeId GetTypeId ();

  /**
   * \brief Set the "user" part of the Handover Management SAP interface that
   *        this handover algorithm instance will interact with.
   * \param s a reference to the "user" part of the interface, typically a
   *          member of an NrEnbRrc instance
   */
  virtual void SetNrHandoverManagementSapUser (NrHandoverManagementSapUser* s) = 0;

  /**
   * \brief Export the "provider" part of the Handover Management SAP interface.
   * \return the reference to the "provider" part of the interface, typically to
   *         be kept by an NrEnbRrc instance
   */
  virtual NrHandoverManagementSapProvider* GetNrHandoverManagementSapProvider () = 0;

protected:

  // inherited from Object
  virtual void DoDispose ();

  // HANDOVER MANAGEMENT SAP PROVIDER IMPLEMENTATION

  /**
   * \brief Implementation of NrHandoverManagementSapProvider::ReportUeMeas.
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   * \param measResults a single report of one measurement identity
   */
  virtual void DoReportUeMeas (uint16_t rnti, NrRrcSap::MeasResults measResults) = 0;

}; // end of class NrHandoverAlgorithm


} // end of namespace ns3


#endif /* NR_HANDOVER_ALGORITHM_H */
