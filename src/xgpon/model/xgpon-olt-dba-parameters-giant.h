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

#ifndef XGPON_OLT_DBA_PARAMETERS_GIANT_H_
#define XGPON_OLT_DBA_PARAMETERS_GIANT_H_


#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

#include "xgpon-olt-net-device.h"
#include "xgpon-phy.h"
#include "xgpon-olt-ploam-engine.h"
#include "xgpon-link-info.h"
#include "xgpon-burst-profile.h"

namespace ns3 {
enum XgponGiantBandwidthType
  {
    XGPON_GIANT_BW_UNDIFINED = 0,       // The bandwidth of this parameter is not defined. Probably the parameter was not initialized yet.
    XGPON_GIANT_BW_FIXED = 1,           // Fixed Bandwidth /
    XGPON_GIANT_BW_ASSURED = 2,         // Assured Bandwidth
    XGPON_GIANT_BW_NON_ASSURED = 3,     // Non Assured bandwidth. When setting up non assured bandwidth some assured polling interval should be given.
    XGPON_GIANT_BW_BEST_EFFORT = 4,     //  Best Effort bandwidth.  When setting up best effort bandwidth some assured polling interval should be given.
  };


class XgponOltDbaParametersGiant: public Object
{

private:
  //member variables
  uint32_t m_serviceIntervalTimer, m_allocationBytes;
  uint64_t m_serviceInterval;
  XgponGiantBandwidthType m_bandwidthType;
  bool m_servedFlag;

public:
  /**
  * \brief Constructor
  */
  XgponOltDbaParametersGiant ();
  virtual ~XgponOltDbaParametersGiant();

  XgponOltDbaParametersGiant (uint32_t serviceRate, uint64_t serviceInterval, XgponGiantBandwidthType bandwidthType);

  /**
  * \brief decrements SImax_timer or SImin_timer
  */
  void UpdateTimer ();

  /**
  * \brief set SImax_timer to SImax or SImin_timer to SImin
  */
  void ResetTimer ();

  //Member variable accessors
  uint32_t GetAllocationBytes();
  uint64_t GetServiceInterval();
  uint32_t GetTimerValue();
  XgponGiantBandwidthType GetBandwidthType();

  void SetAllocationBytes(uint32_t);
  void SetServiceInterval(uint64_t);
  void SetTimerValue(uint32_t);
  void SetBandwidthType(XgponGiantBandwidthType);

  ///Functions required by NS-3
   static TypeId GetTypeId (void);
   virtual TypeId GetInstanceTypeId (void) const;

};

}// namespace ns3

#endif
