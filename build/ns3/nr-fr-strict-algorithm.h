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

#ifndef NR_FR_STRICT_ALGORITHM_H
#define NR_FR_STRICT_ALGORITHM_H

#include <ns3/nr-ffr-algorithm.h>
#include <ns3/nr-ffr-sap.h>
#include <ns3/nr-ffr-rrc-sap.h>
#include <ns3/nr-rrc-sap.h>
#include <map>

namespace ns3 {


/**
 * \brief Strict Frequency Reuse algorithm implementation
 */
class NrFrStrictAlgorithm : public NrFfrAlgorithm
{
public:
  /**
   * \brief Creates a trivial ffr algorithm instance.
   */
  NrFrStrictAlgorithm ();

  virtual ~NrFrStrictAlgorithm ();

  // inherited from Object
  static TypeId GetTypeId ();

  // inherited from NrFfrAlgorithm
  virtual void SetNrFfrSapUser (NrFfrSapUser* s);
  virtual NrFfrSapProvider* GetNrFfrSapProvider ();

  virtual void SetNrFfrRrcSapUser (NrFfrRrcSapUser* s);
  virtual NrFfrRrcSapProvider* GetNrFfrRrcSapProvider ();

  // let the forwarder class access the protected and private members
  friend class MemberNrFfrSapProvider<NrFrStrictAlgorithm>;
  friend class MemberNrFfrRrcSapProvider<NrFrStrictAlgorithm>;

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


  uint8_t m_dlCommonSubBandwidth;
  uint8_t m_dlEgdeSubBandOffset;
  uint8_t m_dlEdgeSubBandwidth;

  uint8_t m_ulCommonSubBandwidth;
  uint8_t m_ulEgdeSubBandOffset;
  uint8_t m_ulEdgeSubBandwidth;

  std::vector <bool> m_dlRbgMap;
  std::vector <bool> m_ulRbgMap;

  std::vector <bool> m_dlEdgeRbgMap;
  std::vector <bool> m_ulEdgeRbgMap;

  enum SubBand
  {
    AreaUnset,
    CellCenter,
    CellEdge
  };

  std::map< uint16_t, uint8_t > m_ues;
  std::vector<uint16_t> m_egdeUes;

  uint8_t m_egdeSubBandThreshold;

  uint8_t m_centerAreaPowerOffset;
  uint8_t m_edgeAreaPowerOffset;

  uint8_t m_centerAreaTpc;
  uint8_t m_edgeAreaTpc;

  // The expected measurement identity
  uint8_t m_measId;

}; // end of class NrFrStrictAlgorithm


} // end of namespace ns3


#endif /* NR_FR_STRICT_ALGORITHM_H */
