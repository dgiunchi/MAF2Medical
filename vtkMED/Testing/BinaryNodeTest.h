/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: BinaryNodeTest.h,v $
Language:  C++
Date:      $Date: 2011-03-14 13:45:48 $
Version:   $Revision: 1.1.2.1 $
Authors:   Daniele Giunchi
==========================================================================
Copyright (c) 2002/2004 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#ifndef __CPP_UNIT_EdgeTest_H__
#define __CPP_UNIT_EdgeTest_H__

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>


class BinaryNodeTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( BinaryNodeTest );
  CPPUNIT_TEST( TestDynamicAllocation );
  CPPUNIT_TEST( TestStaticAllocation );
  CPPUNIT_TEST( TestDynamicAllocation );
  CPPUNIT_TEST( TestStaticAllocation );
  CPPUNIT_TEST( TestCenterCount );
  CPPUNIT_TEST( TestCumulativeCenterCount );
  CPPUNIT_TEST( TestIndex );
  CPPUNIT_TEST( TestCornerIndex );
  CPPUNIT_TEST( TestCornerIndexPosition );
  CPPUNIT_TEST( TestWidth );
  CPPUNIT_TEST( TestCenterAndWidth );
  CPPUNIT_TEST( TestDepthAndOffset );
  CPPUNIT_TEST_SUITE_END();

protected:
  void TestDynamicAllocation();
  void TestStaticAllocation();
  void TestCenterCount();
  void TestCumulativeCenterCount();
  void TestIndex();
  void TestCornerIndex();
  void TestCornerIndexPosition();
  void TestWidth();
  void TestCenterAndWidth();
  void TestDepthAndOffset();
};


int
main( int argc, char* argv[] )
{
  // Create the event manager and test controller
  CPPUNIT_NS::TestResult controller;

  // Add a listener that colllects test result
  CPPUNIT_NS::TestResultCollector result;
  controller.addListener( &result );        

  // Add a listener that print dots as test run.
  CPPUNIT_NS::BriefTestProgressListener progress;
  controller.addListener( &progress );      

  // Add the top suite to the test runner
  CPPUNIT_NS::TestRunner runner;
  runner.addTest( BinaryNodeTest::suite());
  runner.run( controller );

  // Print test in a compiler compatible format.
  CPPUNIT_NS::CompilerOutputter outputter( &result, CPPUNIT_NS::stdCOut() );
  outputter.write(); 

  return result.wasSuccessful() ? 0 : 1;
}

#endif