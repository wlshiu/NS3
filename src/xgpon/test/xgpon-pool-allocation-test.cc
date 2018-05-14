/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Xiuchao Wu <wuxiuchao@gmail.com>
 */

// Include a header file from your module to test.

// An essential include is test.h
#include "ns3/test.h"

#include "ns3/xgpon-xgtc-bw-allocation.h"
#include "ns3/ptr.h"


using namespace ns3;


/*
 * Test the XgponXgtcBwAllocation new/delete override.
 */
class XgponXgtcBwAllocationTestCase : public TestCase
{
public:
  XgponXgtcBwAllocationTestCase  ();
  virtual ~XgponXgtcBwAllocationTestCase  ();

private:
  virtual void DoRun (void);

};

XgponXgtcBwAllocationTestCase::XgponXgtcBwAllocationTestCase  ()
  : TestCase ("Test the dynamic allocation and delete of XgponXgtcBwAllocation structure")
{
}

XgponXgtcBwAllocationTestCase::~XgponXgtcBwAllocationTestCase ()
{
}

void
XgponXgtcBwAllocationTestCase::DoRun (void)
{
  Ptr<XgponXgtcBwAllocation> bwAllocs[100];

  for(int i=0; i<50; i++) bwAllocs[i] = Create<XgponXgtcBwAllocation> ();
  NS_TEST_ASSERT_MSG_EQ (XgponXgtcBwAllocation::GetPoolSize (), 0, "The pool of XgponXgtcBwAllocation has a wrong size (should be 0).");

  for(int i=0; i<20; i++) bwAllocs[i] = 0;
  NS_TEST_ASSERT_MSG_EQ (XgponXgtcBwAllocation::GetPoolSize (), 20, "The pool of XgponXgtcBwAllocation has a wrong size (should be 20).");
  
  XgponXgtcBwAllocation::DisablePoolAllocation();
  NS_TEST_ASSERT_MSG_EQ (XgponXgtcBwAllocation::GetPoolSize (), 0, "The pool of XgponXgtcBwAllocation has a wrong size (should be 0).");
}







class XgponPoolAllocationTestSuite : public TestSuite
{
public:
  XgponPoolAllocationTestSuite ();
};

XgponPoolAllocationTestSuite::XgponPoolAllocationTestSuite ()
  : TestSuite ("xgpon-pool-allocation", UNIT)
{
  AddTestCase (new XgponXgtcBwAllocationTestCase, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static XgponPoolAllocationTestSuite xgponPoolAllocationTestSuite;

