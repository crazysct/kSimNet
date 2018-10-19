/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
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
 */

#include "ns3/simulator.h"
#include "ns3/object-vector.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "original_channel-list.h"
#include "original_channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ChannelList");

/**
 * \ingroup network
 *
 * \brief private implementation detail of the ChannelList API.
 */
class ChannelListPriv : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  ChannelListPriv ();
  ~ChannelListPriv ();

  /**
   * \param channel channel to add
   * \returns index of channel in list.
   *
   * This method is called automatically from Channel::Channel so
   * the user has little reason to call it himself.
   */
  uint32_t Add (Ptr<Channel1> channel); //sjkang_x

  /**
   * \returns a C++ iterator located at the beginning of this
   *          list.
   */
  ChannelList1::Iterator Begin (void) const;
  /**
   * \returns a C++ iterator located at the end of this
   *          list.
   */
  ChannelList1::Iterator End (void) const;

  /**
   * \param n index of requested channel.
   * \returns the Channel associated to index n.
   */
  Ptr<Channel1> GetChannel (uint32_t n); //sjkang_x

  /**
   * \returns the number of channels currently in the list.
   */
  uint32_t GetNChannels (void);

  /**
   * \brief Get the channel list object
   * \returns the channel list
   */
  static Ptr<ChannelListPriv> Get (void);

private:
  /**
   * \brief Get the channel list object
   * \returns the channel list
   */
  static Ptr<ChannelListPriv> *DoGet (void);

  /**
   * \brief Delete the channel list object
   */
  static void Delete (void);

  /**
   * \brief Dispose the channels in the list
   */
  virtual void DoDispose (void);

  std::vector<Ptr<Channel1> > m_channels; //!< channel objects container ///sjkang_x
};

NS_OBJECT_ENSURE_REGISTERED (ChannelListPriv);

TypeId 
ChannelListPriv::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::ChannelListPriv")
    .SetParent<Object> ()
    .SetGroupName("Network")
    .AddAttribute ("ChannelList", "The list of all channels created during the simulation.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&ChannelListPriv::m_channels),
                   MakeObjectVectorChecker<Channel1> ())
  ;
  return tid;
}

Ptr<ChannelListPriv> 
ChannelListPriv::Get (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return *DoGet ();
}

Ptr<ChannelListPriv> *
ChannelListPriv::DoGet (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  static Ptr<ChannelListPriv> ptr = 0;
  if (ptr == 0)
    {
      ptr = CreateObject<ChannelListPriv> ();
      Config::RegisterRootNamespaceObject (ptr);
      Simulator::ScheduleDestroy (&ChannelListPriv::Delete);
    }
  return &ptr;
}

void 
ChannelListPriv::Delete (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::UnregisterRootNamespaceObject (Get ());
  (*DoGet ()) = 0;
}

ChannelListPriv::ChannelListPriv ()
{
  NS_LOG_FUNCTION (this);
}

ChannelListPriv::~ChannelListPriv ()
{
  NS_LOG_FUNCTION (this);
}
void
ChannelListPriv::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  for (std::vector<Ptr<Channel1> >::iterator i = m_channels.begin (); //sjkang_x
       i != m_channels.end (); i++)
    {
      Ptr<Channel1> channel = *i; //sjkang
      channel->Dispose ();
      *i = 0;
    }
  m_channels.erase (m_channels.begin (), m_channels.end ());
  Object::DoDispose ();
}

uint32_t
ChannelListPriv::Add (Ptr<Channel1> channel)
{
  NS_LOG_FUNCTION (this << channel);
  uint32_t index = m_channels.size ();
  m_channels.push_back (channel);
  return index;

}

ChannelList1::Iterator 
ChannelListPriv::Begin (void) const
{
  NS_LOG_FUNCTION (this);
  return m_channels.begin ();
}

ChannelList1::Iterator 
ChannelListPriv::End (void) const
{
  NS_LOG_FUNCTION (this);
  return m_channels.end ();
}

uint32_t 
ChannelListPriv::GetNChannels (void)
{
  NS_LOG_FUNCTION (this);
  return m_channels.size ();
}

Ptr<Channel1>
ChannelListPriv::GetChannel (uint32_t n)
{
  NS_LOG_FUNCTION (this << n);
  NS_ASSERT_MSG (n < m_channels.size (), "Channel index " << n <<
                 " is out of range (only have " << m_channels.size () << " channels).");
  return m_channels[n];
}

uint32_t
ChannelList1::Add (Ptr<Channel1> channel)
{
  NS_LOG_FUNCTION_NOARGS ();
  return ChannelListPriv::Get ()->Add (channel);
}

ChannelList1::Iterator 
ChannelList1::Begin (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return ChannelListPriv::Get ()->Begin ();
}

ChannelList1::Iterator 
ChannelList1::End (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return ChannelListPriv::Get ()->End ();
}

Ptr<Channel1>
ChannelList1::GetChannel (uint32_t n)
{
  NS_LOG_FUNCTION (n);
  return ChannelListPriv::Get ()->GetChannel (n);
}

uint32_t
ChannelList1::GetNChannels (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return ChannelListPriv::Get ()->GetNChannels ();
}

} // namespace ns3
