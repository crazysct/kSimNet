
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/object-factory.h"
#include "ns3/original_queue.h"
#include "ns3/original_csma-net-device.h"
#include "ns3/original_csma-channel.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"

#include "ns3/trace-helper.h"
#include "original_csma-helper.h"
//#include <ns3/original_csma-helper.h>
#include <string>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("CsmaHelper1");

CsmaHelper1::CsmaHelper1 ()
{
  m_queueFactory.SetTypeId ("ns3::DropTailQueue");
  m_deviceFactory.SetTypeId ("ns3::CsmaNetDevice11");
  m_channelFactory.SetTypeId ("ns3::CsmaChannel1");
}

void 
CsmaHelper1::SetQueue (std::string type,
                      std::string n1, const AttributeValue &v1,
                      std::string n2, const AttributeValue &v2,
                      std::string n3, const AttributeValue &v3,
                      std::string n4, const AttributeValue &v4)
{
  m_queueFactory.SetTypeId (type);
  m_queueFactory.Set (n1, v1);
  m_queueFactory.Set (n2, v2);
  m_queueFactory.Set (n3, v3);
  m_queueFactory.Set (n4, v4);
}

void 
CsmaHelper1::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

void 
CsmaHelper1::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  m_channelFactory.Set (n1, v1);
}

void 
CsmaHelper1::EnablePcapInternal (std::string prefix, Ptr<NetDevice1> nd, bool promiscuous, bool explicitFilename)
{
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type CsmaNetDevice1.
  //
  Ptr<CsmaNetDevice1> device = nd->GetObject<CsmaNetDevice1> ();
  if (device == 0)
    {
      NS_LOG_INFO ("CsmaHelper1::EnablePcapInternal(): Device " << device << " not of type ns3::CsmaNetDevice1");
      return;
    }

  PcapHelper pcapHelper;

  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice1 (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out, 
                                                     PcapHelper::DLT_EN10MB);
  if (promiscuous)
    {
      pcapHelper.HookDefaultSink<CsmaNetDevice1> (device, "PromiscSniffer", file);
    }
  else
    {
      pcapHelper.HookDefaultSink<CsmaNetDevice1> (device, "Sniffer", file);
    }
}

void 
CsmaHelper1::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream, 
  std::string prefix, 
  Ptr<NetDevice1> nd,
  bool explicitFilename)
{
  //
  // All of the ascii enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type CsmaNetDevice1.
  //
  Ptr<CsmaNetDevice1> device = nd->GetObject<CsmaNetDevice1> ();
  if (device == 0)
    {
      NS_LOG_INFO ("CsmaHelper1::EnableAsciiInternal(): Device " << device << " not of type ns3::CsmaNetDevice1");
      return;
    }

  //
  // Our default trace sinks are going to use packet printing, so we have to 
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create 
  // one using the usual trace filename conventions and do a Hook*WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy 
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice1 (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      //
      // The MacRx trace source provides our "r" event.
      //
      asciiTraceHelper.HookDefaultReceiveSinkWithoutContext<CsmaNetDevice1> (device, "MacRx", theStream);

      //
      // The "+", '-', and 'd' events are driven by trace sources actually in the
      // transmit queue.
      //
      Ptr<Queue1> queue = device->GetQueue ();
      asciiTraceHelper.HookDefaultEnqueueSinkWithoutContext<Queue1> (queue, "Enqueue", theStream);
      asciiTraceHelper.HookDefaultDropSinkWithoutContext<Queue1> (queue, "Drop", theStream);
      asciiTraceHelper.HookDefaultDequeueSinkWithoutContext<Queue1> (queue, "Dequeue", theStream);

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to providd a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for 
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // Note that we are going to use the default trace sinks provided by the 
  // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
  // but the default trace sinks are actually publicly available static 
  // functions that are always there waiting for just such a case.
  //
  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  oss << "/NodeList/" << nd->GetNode ()->GetId () << "/DeviceList/" << deviceid << "/$ns3::CsmaNetDevice1/MacRx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultReceiveSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CsmaNetDevice1/TxQueue/Enqueue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultEnqueueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CsmaNetDevice1/TxQueue/Dequeue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDequeueSinkWithContext, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::CsmaNetDevice1/TxQueue/Drop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, stream));
}

NetDeviceContainer1
CsmaHelper1::Install (Ptr<Node> node) const
{
  Ptr<CsmaChannel1> channel = m_channelFactory.Create ()->GetObject<CsmaChannel1> ();
  return Install (node, channel);
}

NetDeviceContainer1
CsmaHelper1::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return Install (node);
}

NetDeviceContainer1
CsmaHelper1::Install (Ptr<Node> node, Ptr<CsmaChannel1> channel) const
{
  return NetDeviceContainer1 (InstallPriv (node, channel));
}

NetDeviceContainer1
CsmaHelper1::Install (Ptr<Node> node, std::string channelName) const
{
  Ptr<CsmaChannel1> channel = Names::Find<CsmaChannel1> (channelName);
  return NetDeviceContainer1 (InstallPriv (node, channel));
}

NetDeviceContainer1
CsmaHelper1::Install (std::string nodeName, Ptr<CsmaChannel1> channel) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return NetDeviceContainer1 (InstallPriv (node, channel));
}

NetDeviceContainer1
CsmaHelper1::Install (std::string nodeName, std::string channelName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  Ptr<CsmaChannel1> channel = Names::Find<CsmaChannel1> (channelName);
  return NetDeviceContainer1 (InstallPriv (node, channel));
}

NetDeviceContainer1 
CsmaHelper1::Install (const NodeContainer &c) const
{
  Ptr<CsmaChannel1> channel = m_channelFactory.Create ()->GetObject<CsmaChannel1> ();

  return Install (c, channel);
}

NetDeviceContainer1 
CsmaHelper1::Install (const NodeContainer &c, Ptr<CsmaChannel1> channel) const
{
  NetDeviceContainer1 devs;

  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      devs.Add (InstallPriv (*i, channel));
    }

  return devs;
}

NetDeviceContainer1 
CsmaHelper1::Install (const NodeContainer &c, std::string channelName) const
{
  Ptr<CsmaChannel1> channel = Names::Find<CsmaChannel1> (channelName);
  return Install (c, channel);
}

int64_t
CsmaHelper1::AssignStreams (NetDeviceContainer1 c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<NetDevice1> netDevice;
  for (NetDeviceContainer1::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<CsmaNetDevice1> csma = DynamicCast<CsmaNetDevice1> (netDevice);
      if (csma)
        {
          currentStream += csma->AssignStreams (currentStream);
        }
    }
  return (currentStream - stream);
}

Ptr<NetDevice1>
CsmaHelper1::InstallPriv (Ptr<Node> node, Ptr<CsmaChannel1> channel) const
{
  Ptr<CsmaNetDevice1> device = m_deviceFactory.Create<CsmaNetDevice1> ();
  device->SetAddress (Mac48Address::Allocate ());
  node->AddDevice1 (device);
  Ptr<Queue1> queue = m_queueFactory.Create<Queue1> ();
  device->SetQueue (queue);
  device->Attach (channel);

  return device;
}

} // namespace ns3
