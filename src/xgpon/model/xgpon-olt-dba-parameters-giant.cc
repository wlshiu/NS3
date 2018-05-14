/*
 * Copyright (c)  2012 The Provost, Fellows and Scholars of the
 * College of the Holy and Undivided Trinity of Queen Elizabeth near Dublin.
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
 * Authors: Heather Crowley <crowleyh@tcd.ie>
 *          Pedro Alvarez <pinheirp@tcd.ie>
 */

#include "xgpon-olt-dba-parameters-giant.h"
#include "xgpon-olt-dba-engine-giant.h"

#include "ns3/log.h"

#include "ns3/enum.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

namespace ns3{

NS_OBJECT_ENSURE_REGISTERED (XgponOltDbaParametersGiant);

TypeId
XgponOltDbaParametersGiant::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::XgponOltDbaParametersGiant")
      .SetParent<Object> ()
      .AddConstructor<XgponOltDbaParametersGiant> ()
      .AddAttribute ("ServiceIntervalTimer",
                     "Records maximum (in the case of fixed and assured bandwidth) or minimum "
                     "(non-assured bandwidth) interval between the consecutive intervals of a connection).",
                     UintegerValue (20000),
                     MakeUintegerAccessor (&XgponOltDbaParametersGiant::m_serviceIntervalTimer),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("ServiceInterval",
                     "The value to which the ServiceIntervalTimer is reset.",
                     UintegerValue (20000),
                     MakeUintegerAccessor (&XgponOltDbaParametersGiant::m_serviceInterval),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("AllocationBytes",
                     "The MINIMUM (in the case of fixed and assured) or MAXIMUM (non-assured) bandwidth"
                     "that can be assigned to an AllocId.",
                     UintegerValue (20000),
                     MakeUintegerAccessor (&XgponOltDbaParametersGiant::m_allocationBytes),
                     MakeUintegerChecker<uint32_t> ())
      .AddAttribute ("BandwidthType",
                     "indicated whether it is fixed, assured, non-assured or best effort",
                      UintegerValue (0),
                      MakeUintegerAccessor (&XgponOltDbaParametersGiant::m_bandwidthType),
                      MakeUintegerChecker<uint32_t> ())
      ;

    return tid;
  }

TypeId
XgponOltDbaParametersGiant::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

XgponOltDbaParametersGiant::XgponOltDbaParametersGiant()
 :m_servedFlag(false)
{

}
XgponOltDbaParametersGiant::~XgponOltDbaParametersGiant()
{

}


XgponOltDbaParametersGiant::XgponOltDbaParametersGiant (uint32_t serviceRate, uint64_t serviceInterval, XgponGiantBandwidthType bandwidthType)
:m_servedFlag(false)
{
  m_serviceInterval = serviceInterval;
  m_allocationBytes = serviceRate*serviceInterval;
  m_serviceIntervalTimer = serviceInterval;
  //TODO check if this is the correct initial value for timer?
  //should it start at 1 (fixed/assured), 0(non-assured), or start at service interval to count down?

  m_bandwidthType = bandwidthType;
}


//decrements the timer by 1
void
XgponOltDbaParametersGiant:: UpdateTimer ()
{
  if (m_serviceIntervalTimer>XgponOltDbaEngineGiant::TIMER_EXPIRE_VALUE)
    m_serviceIntervalTimer--;
}

//sets SImax timer timer to SImax (fixed or assured) and SImin timer to SImin (non assured)
void
XgponOltDbaParametersGiant::ResetTimer ()
{
  m_serviceIntervalTimer = m_serviceInterval-1; //Timer counts from SiValue-1 to 0
}

uint32_t
XgponOltDbaParametersGiant::GetAllocationBytes()
{
  return m_allocationBytes;
}

uint32_t
XgponOltDbaParametersGiant::GetTimerValue()
{
  return m_serviceIntervalTimer;
}

uint64_t
XgponOltDbaParametersGiant::GetServiceInterval()
{
  return m_serviceInterval;
}

XgponGiantBandwidthType
XgponOltDbaParametersGiant::GetBandwidthType()
{
  return m_bandwidthType;
}

void
XgponOltDbaParametersGiant::SetAllocationBytes(uint32_t allocationBytes)
{
  m_allocationBytes = allocationBytes;
}

void
XgponOltDbaParametersGiant::SetServiceInterval(uint64_t serviceInterval)
{
  m_serviceInterval = serviceInterval;
}

void
XgponOltDbaParametersGiant::SetTimerValue(uint32_t timerValue)
{
  m_serviceIntervalTimer = timerValue;
}

void
XgponOltDbaParametersGiant::SetBandwidthType(XgponGiantBandwidthType bandwidthType)
{
  m_bandwidthType = bandwidthType;
}

}//end of namespace ns3
