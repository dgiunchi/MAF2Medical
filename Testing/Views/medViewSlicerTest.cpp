/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: medViewSlicerTest.cpp,v $
Language:  C++
Date:      $Date: 2008-07-24 09:05:18 $
Version:   $Revision: 1.1 $
Authors:   Marco Petrone, Stefano Perticoni
==========================================================================
Copyright (c) 2002/2004 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "mafDefines.h" 
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include <cppunit/config/SourcePrefix.h>
#include "medViewSlicer.h"
#include "medViewSlicerTest.h"

#include <iostream>

//-------------------------------------------------------------------------
void medViewSlicerTest::CreateDestroyTest()
//-------------------------------------------------------------------------
{
  medViewSlicer *slicerView = new medViewSlicer();
  slicerView->PackageView();
  slicerView->Delete();
   
  CPPUNIT_ASSERT(true);
 
}