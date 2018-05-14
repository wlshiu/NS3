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


/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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

#include "ns3/log.h"

#include "ns3/enum.h"
#include "ns3/uinteger.h"

#include "xgpon-queue.h"



NS_LOG_COMPONENT_DEFINE ("XgponQueue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (XgponQueue);


TypeId 
XgponQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::XgponQueue")
    .SetParent<Object> ()
    .AddAttribute ("Mode", 
                   "Whether to use bytes (see MaxBytes) or packets (see MaxPackets) as the maximum queue size metric.",
                   EnumValue (XGPON_QUEUE_MODE_BYTES),
                   MakeEnumAccessor (&XgponQueue::SetMode),
                   MakeEnumChecker (XGPON_QUEUE_MODE_BYTES, "XGPON_QUEUE_MODE_BYTES",
                                    XGPON_QUEUE_MODE_PACKETS, "XGPON_QUEUE_MODE_PACKETS"))
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets that could be held by this FIFOQueue.",
                   UintegerValue (1000),      //1000 packets
                   MakeUintegerAccessor (&XgponQueue::m_maxPackets),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("MaxBytes", 
                   "The maximum number of bytes that could be held by this FIFOQueue.",
                   UintegerValue (1000 * 1024),  //1Mbytes, close to 10Gbps * 1ms (rtt of a xgpon).
                   MakeUintegerAccessor (&XgponQueue::m_maxBytes),
                   MakeUintegerChecker<uint32_t> ())

    .AddTraceSource ("Enqueue", "Enqueue a packet in the queue.",
                     MakeTraceSourceAccessor (&XgponQueue::m_traceEnqueue))
    .AddTraceSource ("Dequeue", "Dequeue a packet from the queue.",
                     MakeTraceSourceAccessor (&XgponQueue::m_traceDequeue))
    .AddTraceSource ("Drop", "Drop a packet stored in the queue.",
                     MakeTraceSourceAccessor (&XgponQueue::m_traceDrop))
  ;
  return tid;
}
TypeId
XgponQueue::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}




XgponQueue::XgponQueue() : 
  m_nPackets (0),
  m_nBytes (0),
  m_nWords4Scheduling(0),
  m_remainingSegment (0),
  m_nTotalReceivedBytes (0),
  m_nTotalReceivedPackets (0),
  m_nTotalDroppedBytes (0),
  m_nTotalDroppedPackets (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}
XgponQueue::~XgponQueue()
{
  NS_LOG_FUNCTION_NOARGS ();
}



bool 
XgponQueue::Enqueue (const Ptr<Packet>& p)
{
  NS_LOG_FUNCTION (this << p);

  bool retval = DoEnqueue (p);
  if (retval)
    {
      NS_LOG_LOGIC ("m_traceEnqueue (p)");
      m_traceEnqueue (p);

      m_nPackets++;

      uint32_t size = p->GetSize ();
      m_nBytes += size;

      uint32_t sizeWord = CalculatePacketSize4Scheduling(size);
      m_nWords4Scheduling += sizeWord;


      m_nTotalReceivedBytes += size;
      m_nTotalReceivedPackets++;
    }
  return retval;
}


const Ptr<Packet>
XgponQueue::Dequeue (void)
{
  NS_LOG_FUNCTION (this);

  const Ptr<Packet> packet = DoDequeue ();

  if (packet != 0)
    {
      NS_ASSERT (m_nBytes >= packet->GetSize ());
      NS_ASSERT (m_nPackets > 0);

      m_nPackets--;

      uint32_t size = packet->GetSize ();
      m_nBytes -= size;

      uint32_t sizeWord = CalculatePacketSize4Scheduling(size);
      m_nWords4Scheduling -= sizeWord;

      NS_LOG_LOGIC ("m_traceDequeue (packet)");
      m_traceDequeue (packet);
    }
  return packet;
}



void
XgponQueue::Drop (const Ptr<Packet>& p)
{
  NS_LOG_FUNCTION (this << p);

  m_nTotalDroppedPackets++;
  m_nTotalDroppedBytes += p->GetSize ();

  NS_LOG_LOGIC ("m_traceDrop (p)");
  m_traceDrop (p);
}



void
XgponQueue::PushFrontRemainingSegment (const Ptr<Packet>& pkt)
{
  NS_ASSERT_MSG((m_remainingSegment ==0), "Something is WRONG!!! Segments exist.");

  //We need not worry that the queue will be overflowed.
  //The segment to be pushed back is just one part of the packet just poped from this queue.
  m_remainingSegment = pkt;

  m_nPackets++;

  uint32_t size = pkt->GetSize ();
  m_nBytes += size;

  uint32_t sizeWord = CalculatePacketSize4Scheduling(size);
  m_nWords4Scheduling += sizeWord;

  return;
}




} // namespace ns3
