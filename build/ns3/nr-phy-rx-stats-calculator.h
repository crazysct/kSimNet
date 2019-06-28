/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jnin@cttc.es>
 * modified by: Marco Miozzo <mmiozzo@cttc.es>
 *        Convert NrMacStatsCalculator in NrPhyRxStatsCalculator
 */

#ifndef PHY_RX_STATS_CALCULATOR_H_
#define PHY_RX_STATS_CALCULATOR_H_

#include "ns3/nr-stats-calculator.h"
#include "ns3/nstime.h"
#include "ns3/uinteger.h"
#include <string>
#include <fstream>
#include <ns3/nr-common.h>

namespace ns3 {

/**
 * \ingroup nr
 *
 * Takes care of storing the information generated at PHY layer regarding 
 * reception. Metrics saved are:
 *
 *   - Timestamp (in seconds)
 *   - Frame index
 *   - Subframe index
 *   - C-RNTI
 *   - MCS for transport block 1
 *   - Size of transport block 1
 *   - MCS for transport block 2 (0 if not used)
 *   - Size of transport block 2 (0 if not used)
 */
class NrPhyRxStatsCalculator : public NrStatsCalculator
{
public:
  /**
   * Constructor
   */
  NrPhyRxStatsCalculator ();

  /**
   * Destructor
   */
  virtual ~NrPhyRxStatsCalculator ();

  // Inherited from ns3::Object
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  /**
   * Set the name of the file where the UL Rx PHY statistics will be stored.
   *
   * \param outputFilename string with the name of the file
   */
  void SetUlRxOutputFilename (std::string outputFilename);

  /**
   * Get the name of the file where the UL RX PHY statistics will be stored.
   * @return the name of the file where the UL RX PHY statistics will be stored
   */
  std::string GetUlRxOutputFilename (void);

  /**
   * Set the name of the file where the DL RX PHY statistics will be stored.
   *
   * @param outputFilename string with the name of the file
   */
  void SetDlRxOutputFilename (std::string outputFilename);

  /**
   * Get the name of the file where the DL RX PHY statistics will be stored.
   * @return the name of the file where the DL RX PHY statistics will be stored
   */
  std::string GetDlRxOutputFilename (void);

  /**
   * Notifies the stats calculator that an downlink reception has occurred.
   * @param params Trace information regarding PHY reception stats
   */
  void DlPhyReception (NrPhyReceptionStatParameters params);

  /**
   * Notifies the stats calculator that an uplink reception has occurred.
   * @param params Trace information regarding PHY reception stats
   */
  void UlPhyReception (NrPhyReceptionStatParameters params);

  /** 
   * trace sink
   * 
   * \param phyRxStats 
   * \param path 
   * \param params 
   */
  static void DlPhyReceptionCallback (Ptr<NrPhyRxStatsCalculator> phyRxStats,
                               std::string path, NrPhyReceptionStatParameters params);

  /** 
   * trace sink
   * 
   * \param phyRxStats 
   * \param path 
   * \param params 
   */
  static void UlPhyReceptionCallback (Ptr<NrPhyRxStatsCalculator> phyRxStats,
                               std::string path, NrPhyReceptionStatParameters params);
private:

  /**
   * When writing DL RX PHY statistics first time to file,
   * columns description is added. Then next lines are
   * appended to file. This value is true if output
   * files have not been opened yet
   */
  bool m_dlRxFirstWrite;

  /**
   * When writing UL RX PHY statistics first time to file,
   * columns description is added. Then next lines are
   * appended to file. This value is true if output
   * files have not been opened yet
   */
  bool m_ulRxFirstWrite;

};

} // namespace ns3

#endif /* PHY_RX_STATS_CALCULATOR_H_ */
