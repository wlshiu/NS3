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

#include <ns3/simulator.h>
#include "ns3/log.h"

#include "pon-net-device.h"
#include "pon-channel.h"



NS_LOG_COMPONENT_DEFINE ("PonNetDevice");

namespace ns3{

NS_OBJECT_ENSURE_REGISTERED (PonNetDevice);

TypeId 
PonNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PonNetDevice")
    .SetParent<NetDevice> ()
    ;
  return tid;
}
TypeId 
PonNetDevice::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}




PonNetDevice::PonNetDevice() : NetDevice(), m_channel(0), m_node(0)
{
    this->nextStateChangeEventId = this->ScheduleState(REFRESH, NanoSeconds(500));

}
PonNetDevice::~PonNetDevice()
{
}




void 
PonNetDevice::SetChannel (const Ptr<PonChannel>& channel)
{
  m_channel = channel;
}
Ptr<Channel> 
PonNetDevice::GetChannel (void) const
{
  NS_ASSERT_MSG((m_channel!=0), "The PON channel has not been set for this network device!!!");
  return m_channel;
}





void
PonNetDevice::DoDispose (void)
{
  m_channel = 0;
  m_node = 0;
  NetDevice::DoDispose ();
}

void
PonNetDevice::SetState(xgPonDeviceStates state, Ptr<PonNetDevice> ponDev)
{
    //Get current time
    Time now = Simulator::Now();

    //Calculate time difference between previous and current update
    Time timeDifference = now - ponDev->latestStateUpdate;

    //Subtract battery consumption if you have an associated energy model and an attached source

    //Reschedule refresh
    Simulator::Cancel(ponDev->nextStateChangeEventId);
    ponDev->nextStateChangeEventId = ponDev->ScheduleState(REFRESH, NanoSeconds(500));

    //Set new state
    if (state == REFRESH)
    {
        //If refresh, change between SLEEP and AWARE
        if (ponDev->currentState == SLEEP)
            ponDev->currentState = AWARE;
        else
            ponDev->currentState = SLEEP;
    }
    else
        //If change of state, change
        ponDev->currentState = state;

    //Update latest state update with the current timestamp
    ponDev->latestStateUpdate = now;
}

EventId
PonNetDevice::ScheduleState(xgPonDeviceStates state, Time schedtime)
{
    Ptr<PonNetDevice> ponDev = this;
    EventId id = Simulator::Schedule(schedtime, &PonNetDevice::SetState, state, ponDev);
    return id;
}

} // namespace ns3
