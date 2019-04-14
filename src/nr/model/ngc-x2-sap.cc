/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Dual Connectivity functionalities
 */

#include "ns3/ngc-x2-sap.h"

namespace ns3 {


NgcX2Sap::~NgcX2Sap ()
{
}

NgcX2Sap::ErabToBeSetupItem::ErabToBeSetupItem () :
  erabLevelQosParameters (EpsBearer (EpsBearer::GBR_CONV_VOICE))
{
}

NgcX2SapProvider::~NgcX2SapProvider ()
{
}

NgcX2SapUser::~NgcX2SapUser ()
{
}

NgcX2PdcpUser::~NgcX2PdcpUser()
{
}

NgcX2PdcpProvider::~NgcX2PdcpProvider()
{
}

NgcX2RlcUser::~NgcX2RlcUser()
{
}

NgcX2RlcProvider::~NgcX2RlcProvider()
{
}

} // namespace ns3
