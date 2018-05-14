/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 University College Cork (UCC), Ireland
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
 * Author: Xiuchao Wu <xw2@cs.ucc.ie>
 */
#include "ns3/log.h"

#include "xgpon-fifo-queue.h"



NS_LOG_COMPONENT_DEFINE ("XgponFifoQueue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (XgponFifoQueue);

TypeId
XgponFifoQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::XgponFifoQueue")
    .SetParent<XgponQueue> ()
    .AddConstructor<XgponFifoQueue> ()
  ;

  return tid;
}
TypeId
XgponFifoQueue::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}



XgponFifoQueue::XgponFifoQueue (): XgponQueue ()
{
}
XgponFifoQueue::~XgponFifoQueue ()
{
}




bool 
XgponFifoQueue::DoEnqueue (const Ptr<Packet>& p)
{
  NS_LOG_FUNCTION (this << p);

  if (m_mode == XGPON_QUEUE_MODE_PACKETS && m_nPackets >= m_maxPackets)
    {
      NS_LOG_LOGIC ("Queue full (at max packets) -- droppping pkt");
      Drop (p);
      return false;
    }

  if (m_mode == XGPON_QUEUE_MODE_BYTES && (m_nBytes + p->GetSize () >= m_maxBytes)) 
    {
      NS_LOG_LOGIC ("Queue full (packet would exceed max bytes) -- droppping pkt");
      Drop (p);
      return false;
    }

  m_packets.push(p);


  NS_LOG_LOGIC ("Number packets " << m_nPackets);
  NS_LOG_LOGIC ("Number bytes " <<  m_nBytes);

  return true;
}



const Ptr<Packet>
XgponFifoQueue::DoDequeue (void)
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> p;

  if (m_remainingSegment==0 && m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  if (m_remainingSegment==0)
    {
      p = m_packets.front ();
      m_packets.pop();
    }
  else
    {
      p = m_remainingSegment;
      m_remainingSegment = 0;
    }  

  NS_LOG_LOGIC ("Popped " << p);

  NS_LOG_LOGIC ("Number packets " << m_packets.size ());
  NS_LOG_LOGIC ("Number bytes " << m_nBytes);

  return p;
}





const Ptr<const Packet>
XgponFifoQueue::DoPeek (void) const
{
  NS_LOG_FUNCTION (this);

  if (m_remainingSegment==0 && m_packets.empty ())
    {
      NS_LOG_LOGIC ("Queue empty");
      return 0;
    }

  if (m_remainingSegment==0)
    {
      return m_packets.front ();
    }
  else
    {
      return m_remainingSegment;
    }
}







}; // namespace ns3
