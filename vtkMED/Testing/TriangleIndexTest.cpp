/*=========================================================================
Program:   Multimod Application Framework
Module:    $RCSfile: TriangleIndexTest.cpp,v $
Language:  C++
Date:      $Date: 2010-11-19 10:08:15 $
Version:   $Revision: 1.1.2.1 $
Authors:   Matteo Giacomoni
==========================================================================
Copyright (c) 2002/2004 
CINECA - Interuniversity Consortium (www.cineca.it)
=========================================================================*/

#include "medDefines.h"
//----------------------------------------------------------------------------
// NOTE: Every CPP file in the MAF must include "mafDefines.h" as first.
// This force to include Window,wxWidgets and VTK exactly in this order.
// Failing in doing this will result in a run-time error saying:
// "Failure#0: The value of ESP was not properly saved across a function call"
//----------------------------------------------------------------------------

#include <cppunit/config/SourcePrefix.h>
#include "TriangleIndexTest.h"

#include "vtkMEDPoissonSurfaceReconstruction.h"

//-------------------------------------------------------------------------
void TriangleIndexTest::TestDynamicAllocation()
//-------------------------------------------------------------------------
{
  TriangleIndex *var = new TriangleIndex();

  delete var;
}
//-------------------------------------------------------------------------
void TriangleIndexTest::TestStaticAllocation()
//-------------------------------------------------------------------------
{
  TriangleIndex var;
}