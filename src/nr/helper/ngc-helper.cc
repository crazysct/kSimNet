/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jnin@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 *         Manuel Requena <manuel.requena@cttc.es>
 *
 * Modified by: Michele Polese <michele.polese@gmail.com>
 *          Support for real N2AP link
 */

#include <ns3/ngc-helper.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/ipv4-address.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NgcHelper");

NS_OBJECT_ENSURE_REGISTERED (NgcHelper);


NgcHelper::NgcHelper () 
{
  NS_LOG_FUNCTION (this);
}

NgcHelper::~NgcHelper ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NgcHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NgcHelper")
    .SetParent<Object> ()
    .SetGroupName("Nr")
    ;
  return tid;
}

void
NgcHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}



} // namespace ns3
