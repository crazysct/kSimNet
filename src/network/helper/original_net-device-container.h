/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
#ifndef ORIGINAL_NET_DEVICE_CONTAINER_H
#define ORIGINAL_NET_DEVICE_CONTAINER_H

#include <stdint.h>
#include <vector>
#include "ns3/original_net-device.h"

namespace ns3 {

/**
 * \brief holds a vector of ns3::NetDevice1 pointers
 *
 * Typically ns-3 NetDevices are installed on nodes using a net device
 * helper.  The helper Install method takes a NodeContainer which holds 
 * some number of Ptr<Node>.  For each of the Nodes in the NodeContainer
 * the helper will instantiate a net device, add a MAC address and a queue
 * to the device and install it to the node.  For each of the devices, the
 * helper also adds the device into a Container for later use by the caller.
 * This is that container used to hold the Ptr<NetDevice1> which are 
 * instantiated by the device helper.
 */
class NetDeviceContainer1
{
public:
  /// NetDevice1 container iterator
  typedef std::vector<Ptr<NetDevice1> >::const_iterator Iterator;

  /**
   * Create an empty NetDeviceContainer1.
   */
  NetDeviceContainer1 ();

  /**
   * \param dev a device to add to the container
   *
   * Create a NetDeviceContainer1 with exactly one net device that has previously
   * been instantiated
   */
  NetDeviceContainer1 (Ptr<NetDevice1> dev);

  /**
   * Create a NetDeviceContainer1 with exactly one device which has been 
   * previously instantiated and assigned a name using the Object name
   * service.  This NetDevice1 is specified by its assigned name. 
   *
   * \param devName The name of the device to add to the container
   *
   * Create a NetDeviceContainer1 with exactly one device
   */
  NetDeviceContainer1 (std::string devName);

  /**
   * \param a a device container
   * \param b another device container
   *
   * Create a device container which is a concatenation of the two input
   * NetDeviceContainers.
   *
   * \note A frequently seen idiom that uses these constructors involves the
   * implicit conversion by constructor of Ptr<NetDevice1>.  When used, two 
   * Ptr<NetDevice1> will be passed to this constructor instead of NetDeviceContainer1&.
   * C++ will notice the implicit conversion path that goes through the 
   * NetDeviceContainer1 (Ptr<NetDevice1> dev) constructor above.  Using this conversion
   * one may provide optionally provide arguments of Ptr<NetDevice1> to these 
   * constructors.
   */
  NetDeviceContainer1 (const NetDeviceContainer1 &a, const NetDeviceContainer1 &b);

  /**
   * \brief Get an iterator which refers to the first NetDevice1 in the 
   * container.
   *
   * NetDevices can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the iterator method and is typically used in a 
   * for-loop to run through the NetDevices
   *
   * \code
   *   NetDeviceContainer1::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       (*i)->method ();  // some NetDevice1 method
   *     }
   * \endcode
   *
   * \returns an iterator which refers to the first NetDevice1 in the container.
   */
  Iterator Begin (void) const;

  /**
   * \brief Get an iterator which indicates past-the-last NetDevice1 in the 
   * container.
   *
   * NetDevices can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the iterator method and is typically used in a 
   * for-loop to run through the NetDevices
   *
   * \code
   *   NetDeviceContainer1::Iterator i;
   *   for (i = container.Begin (); i != container.End (); ++i)
   *     {
   *       (*i)->method ();  // some NetDevice1 method
   *     }
   * \endcode
   *
   * \returns an iterator which indicates an ending condition for a loop.
   */
  Iterator End (void) const;

  /**
   * \brief Get the number of Ptr<NetDevice1> stored in this container.
   *
   * NetDevices can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the direct method and is typically used to
   * define an ending condition in a for-loop that runs through the stored
   * NetDevices
   *
   * \code
   *   uint32_t nDevices = container.GetN ();
   *   for (uint32_t i = 0 i < nDevices; ++i)
   *     {
   *       Ptr<NetDevice1> p = container.Get (i)
   *       i->method ();  // some NetDevice1 method
   *     }
   * \endcode
   *
   * \returns the number of Ptr<NetDevice1> stored in this container.
   */
  uint32_t GetN (void) const;

  /**
   * \brief Get the Ptr<NetDevice1> stored in this container at a given
   * index.
   *
   * NetDevices can be retrieved from the container in two ways.  First,
   * directly by an index into the container, and second, using an iterator.
   * This method is used in the direct method and is used to retrieve the
   * indexed Ptr<NetDevice1>.
   *
   * \code
   *   uint32_t nDevices = container.GetN ();
   *   for (uint32_t i = 0 i < nDevices; ++i)
   *     {
   *       Ptr<NetDevice1> p = container.Get (i)
   *       i->method ();  // some NetDevice1 method
   *     }
   * \endcode
   *
   * \param i the index of the requested device pointer.
   * \returns the requested device pointer.
   */
  Ptr<NetDevice1> Get (uint32_t i) const;

  /**
   * \brief Append the contents of another NetDeviceContainer1 to the end of
   * this container.
   *
   * \param other The NetDeviceContainer1 to append.
   */
  void Add (NetDeviceContainer1 other);

  /**
   * \brief Append a single Ptr<NetDevice1> to this container.
   *
   * \param device The Ptr<NetDevice1> to append.
   */
  void Add (Ptr<NetDevice1> device);

  /**
   * \brief Append to this container the single Ptr<NetDevice1> referred to
   * via its object name service registered name.
   *
   * \param deviceName The name of the NetDevice1 Object to add to the container.
   */
  void Add (std::string deviceName);

private:
  std::vector<Ptr<NetDevice1> > m_devices; //!< NetDevices smart pointers
};

} // namespace ns3

#endif /* NET_DEVICE_CONTAINER_H */
