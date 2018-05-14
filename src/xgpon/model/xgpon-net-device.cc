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

#include "xgpon-net-device.h"


NS_LOG_COMPONENT_DEFINE ("XgponNetDevice");

namespace ns3{

NS_OBJECT_ENSURE_REGISTERED (XgponNetDevice);

TypeId 
XgponNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::XgponNetDevice")
    .SetParent<PonNetDevice> ()

    //Trace Sources of the per-device virtual queue: enqueue, dequeue, and drop events
    .AddTraceSource ("Enqueue", "Enqueue a packet in the virtual per-device queue.",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_traceEnqueueVirtual))
    .AddTraceSource ("Dequeue", "Dequeue a packet from the virtual per-device queue.",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_traceDequeueVirtual))
    .AddTraceSource ("Drop", "Drop a packet stored in the virtual per-device queue.",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_traceDropVirtual))

    //
    // Trace sources designed to simulate a packet sniffer facility (tcpdump).
    // Note that ONU does not reassemble packets for other ONUs.
    // Thus, there is no difference between the following two trace sources.
    //
    .AddTraceSource ("Sniffer", 
                     "Trace source simulating a non-promiscuous packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_snifferTrace))
    .AddTraceSource ("PromiscSniffer", 
                     "Trace source simulating a promiscuous packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_promiscSnifferTrace))

    .AddTraceSource ("DeviceStatistics", "Trace sources for the whole network device statistics",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_deviceStatisticsTrace))

#if 0
    // Not currently implemented for this device

    //
    // Trace sources at the "top" of the net device, where packets transition
    // to/from higher layers.
    //
    .AddTraceSource ("MacTx", 
                     "Trace source indicating a packet has arrived for transmission by this device",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_macTxTrace))
    .AddTraceSource ("MacTxDrop", 
                     "Trace source indicating a packet has been dropped by the device before transmission",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_macTxDropTrace))
    .AddTraceSource ("MacPromiscRx", 
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_macPromiscRxTrace))
    .AddTraceSource ("MacRx", 
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_macRxTrace))
    .AddTraceSource ("MacRxDrop", 
                     "Trace source indicating a packet was dropped before being forwarded up the stack",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_macRxDropTrace))


    //
    // Trace souces at the "bottom" of the net device, where packets transition
    // to/from the channel.
    //
    .AddTraceSource ("PhyTxBegin", 
                     "Trace source indicating a packet has begun transmitting over the channel",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_phyTxBeginTrace))
    .AddTraceSource ("PhyTxEnd", 
                     "Trace source indicating a packet has been completely transmitted over the channel",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_phyTxEndTrace))
    .AddTraceSource ("PhyTxDrop", 
                     "Trace source indicating a packet has been dropped by the device during transmission",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_phyTxDropTrace))


    .AddTraceSource ("PhyRxBegin", 
                     "Trace source indicating a packet has begun being received by the device",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_phyRxBeginTrace))

    .AddTraceSource ("PhyRxEnd", 
                     "Trace source indicating a packet has been completely received by the device",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_phyRxEndTrace))
    .AddTraceSource ("PhyRxDrop", 
                     "Trace source indicating a packet has been dropped by the device during reception",
                     MakeTraceSourceAccessor (&XgponNetDevice::m_phyRxDropTrace))

#endif
  ;
  return tid;
}
TypeId 
XgponNetDevice::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}



XgponNetDevice::XgponNetDevice () : PonNetDevice(), m_commonPhy(0)
{
  m_stat.initialize ();
  SetMtu(DEFAULT_XGPON_MTU);   //set MTU
}
XgponNetDevice::~XgponNetDevice ()
{
}




void 
XgponNetDevice::SendSduToUpperLayer (const Ptr<Packet>& sdu)
{
  NS_LOG_FUNCTION(this);

  uint16_t protocol=2048;
  Address from;

  //for sniffering that one packet is received through this netdevice.
  TraceForSniffers (sdu);

  ///////////statistics
  m_stat.m_rxFromXgponBytes += sdu->GetSize();


  m_rxCallback (this, sdu, protocol, from);
  return;
}




bool 
XgponNetDevice::Send (const Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  bool rst = DoSend(packet, dest, protocolNumber);

  //trace the virtual per-device queue event
  if(rst) 
  { 
    m_traceEnqueueVirtual (packet); 
    m_stat.m_rxFromUpperLayerBytes += packet->GetSize();  //statistics
  }
  else 
  { 
    m_traceDropVirtual (packet); 
    m_stat.m_overallQueueDropBytes += packet->GetSize();  //statistics
  }

  return rst;
}

bool 
XgponNetDevice::SendFrom (const Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  bool rst = DoSendFrom(packet, source, dest, protocolNumber);

  //trace the virtual per-device queue event
  if(rst) 
  { 
    m_traceEnqueueVirtual (packet); 
    m_stat.m_rxFromUpperLayerBytes += packet->GetSize();  //statistics
  }
  else 
  { 
    m_traceDropVirtual (packet); 
    m_stat.m_overallQueueDropBytes += packet->GetSize();  //statistics
  }

  return rst;
}




void 
XgponNetDevice::TraceVirtualQueueDequeueEvent (const Ptr<Packet>& packet)
{
  m_traceDequeueVirtual (packet);
}

void 
XgponNetDevice::TraceForSniffers (const Ptr<Packet>& packet)
{
  m_snifferTrace (packet);
  m_promiscSnifferTrace (packet);
}






void 
XgponNetDeviceStatistics::initialize ()
{
    m_rxFromUpperLayerPkts = 0;
    m_rxFromXgponPkts = 0;

    m_passToUpperLayerPkts = 0;
    m_passToXgponPkts = 0;

    m_overallQueueDropPkts = 0;


    m_rxFromUpperLayerBytes = 0;
    m_rxFromXgponBytes = 0;

    m_passToUpperLayerBytes = 0;
    m_passToXgponBytes = 0;

    m_overallQueueDropBytes = 0;
}


}//namespace ns3
