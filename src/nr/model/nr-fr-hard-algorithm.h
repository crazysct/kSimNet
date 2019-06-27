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

#ifndef NR_FR_HARD_ALGORITHM_H
#define NR_FR_HARD_ALGORITHM_H

#include <ns3/nr-ffr-algorithm.h>
#include <ns3/nr-ffr-sap.h>
#include <ns3/nr-ffr-rrc-sap.h>
#include <ns3/nr-rrc-sap.h>

namespace ns3 {


/**
 * \brief Hard Frequency Reuse algorithm implementation which uses only 1 sub-band.
 */
class NrFrHardAlgorithm : public NrFfrAlgorithm
{
public:
  /**
   * \brief Creates a trivial ffr algorithm instance.
   */
  NrFrHardAlgorithm ();

  virtual ~NrFrHardAlgorithm ();

  // inherited from Object
  static TypeId GetTypeId ();

  // inherited from NrFfrAlgorithm
  virtual void SetNrFfrSapUser (NrFfrSapUser* s);
  virtual NrFfrSapProvider* GetNrFfrSapProvider ();

  virtual void SetNrFfrRrcSapUser (NrFfrRrcSapUser* s);
  virtual NrFfrRrcSapProvider* GetNrFfrRrcSapProvider ();

  // let the forwarder class access the protected and private members
  friend class MemberNrFfrSapProvider<NrFrHardAlgorithm>;
  friend class MemberNrFfrRrcSapProvider<NrFrHardAlgorithm>;

protected:
  // inherited from Object
  virtual void DoInitialize ();
  virtual void DoDispose ();

  virtual void Reconfigure ();

  // FFR SAP PROVIDER IMPLEMENTATION
  virtual std::vector <bool> DoGetAvailableDlRbg ();
  virtual bool DoIsDlRbgAvailableForUe (int i, uint16_t rnti);
  virtual std::vector <bool> DoGetAvailableUlRbg ();
  virtual bool DoIsUlRbgAvailableForUe (int i, uint16_t rnti);
  virtual void DoReportDlCqiInfo (const struct NrFfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params);
  virtual void DoReportUlCqiInfo (const struct NrFfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params);
  virtual void DoReportUlCqiInfo ( std::map <uint16_t, std::vector <double> > ulCqiMap );
  virtual uint8_t DoGetTpc (uint16_t rnti);
  virtual uint8_t DoGetMinContinuousUlBandwidth ();

  // FFR SAP RRC PROVIDER IMPLEMENTATION
  virtual void DoReportUeMeas (uint16_t rnti, NrRrcSap::MeasResults measResults);
  virtual void DoRecvLoadInformation (NgcX2Sap::LoadInformationParams params);

private:
  void SetDownlinkConfiguration (uint16_t cellId, uint8_t bandwidth);
  void SetUplinkConfiguration (uint16_t cellId, uint8_t bandwidth);
  void InitializeDownlinkRbgMaps ();
  void InitializeUplinkRbgMaps ();

  // FFR SAP
  NrFfrSapUser* m_ffrSapUser;
  NrFfrSapProvider* m_ffrSapProvider;

  // FFR RRF SAP
  NrFfrRrcSapUser* m_ffrRrcSapUser;
  NrFfrRrcSapProvider* m_ffrRrcSapProvider;

  uint8_t m_dlOffset;
  uint8_t m_dlSubBand;

  uint8_t m_ulOffset;
  uint8_t m_ulSubBand;

  std::vector <bool> m_dlRbgMap;
  std::vector <bool> m_ulRbgMap;

}; // end of class NrFrHardAlgorithm


} // end of namespace ns3


#endif /* NR_FR_HARD_ALGORITHM_H */
