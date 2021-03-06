/*=========================================================================
Program:   MAF2Medical
Module:    $RCSfile: medVMEStentModelSource.cpp,v $
Language:  C++
Date:      $Date: 2012-10-23 10:15:31 $
Version:   $Revision: 1.1.2.7 $
Authors:   Hui Wei
==========================================================================
Copyright (c) 2013
University of Bedfordshire, UK
=========================================================================*/


#include "vtkMEDStentModelSource.h"
#include "vtkPolyData.h"
#include <math.h>
#include "vtkMath.h"

/************************************************************************/
/* construct method
	give some init parameters to create stent
*/
/************************************************************************/
vtkMEDStentModelSource::vtkMEDStentModelSource(void)
  : centerLine(NULL), m_NumberOfPointsAllocatedToCenterLine(0)
{
	stentDiameter = 1.0;
	crownLength = 2.2;
	linkLength = 0.8;
	strutsNumber = 16; //how many points in one circle
	crownNumber = 10;
	linkNumber = 4;
	linkConnection = peak2peak;
	stentConfiguration = OutOfPhase;
	linkOrientation = None;
	linkAlignment =3;

	initialStentLength = 0;
  simplexMesh = SimplexMeshType::New(); // smart pointer - don't try to delete
}



/************************************************************************/
/* deconstruct method                                                   */
/************************************************************************/
vtkMEDStentModelSource::~vtkMEDStentModelSource(void)
{
  if (centerLine != NULL)
	  delete [] centerLine;
}




/**
* setters
*/
/* set diameter */
void vtkMEDStentModelSource::setStentDiameter(double value){
	stentDiameter = value;
}
/* set crown length */
void vtkMEDStentModelSource::setCrownLength(double value){
	crownLength = value;
}
/* set struct angle, no use */
void vtkMEDStentModelSource::setStrutAngle(double value){
	strutAngle = value;
}
/* set link length */
void vtkMEDStentModelSource::setLinkLength(double value){
	linkLength = value;
}
/* set struts number */
void vtkMEDStentModelSource::setStrutsNumber(int value){
	strutsNumber = value;
}
/* set link connection {peak2valley, valley2peak, peak2peak, valley2valley}*/
void vtkMEDStentModelSource::setLinkConnection(LinkConnectionType value){
	linkConnection = value;
}
/* set link alignment {-1,0,+1}*/
void vtkMEDStentModelSource::setLinkAlignment(unsigned int value){
	linkAlignment = value;
}
/* set stent configuration {InPhase, OutOfPhase}*/
void vtkMEDStentModelSource::setStentConfiguration(StentConfigurationType value){
	stentConfiguration = value;
}
/* set link orientation {InPhase, OutOfPhase}*/
void vtkMEDStentModelSource::setLinkOrientation(LinkOrientationType value){
	linkOrientation = value;
}
/* set crown number, default 10*/
void vtkMEDStentModelSource::setCrownNumber(int value){
	crownNumber = value;
}
/* set link number*/
void vtkMEDStentModelSource::setLinkNumber(int value){
	linkNumber = value;
}



void vtkMEDStentModelSource::setCenterLineFromPolyData(vtkPolyData *line){
	if(line){
		vector<vector<double> > m_StentCenterLine;
		vector<vector<double> >::const_iterator centerLineStart;
		vector<vector<double> >::const_iterator centerLineEnd;

		centerLineLength = 0;
		/**------------to get start and end------------*/
    int numOfPoints = line->GetNumberOfPoints();

		if(numOfPoints>0){
			vtkPoints *linePoints = line->GetPoints();
			double p[3],p1[3];

			vector<double> vertex;
			for(vtkIdType i = 0; i < numOfPoints; i++){
				line->GetPoint(i,p);
				if (i+1 < numOfPoints-1){
					line->GetPoint(i+1,p1);
					centerLineLength += sqrt(vtkMath::Distance2BetweenPoints(p,p1));
				}
				
				//-----for deformation---
				vertex.push_back(p[0]);
				vertex.push_back(p[1]);
				vertex.push_back(p[2]);
				m_StentCenterLine.push_back(vertex);
				vertex.clear();
				//------for store-----
				//m_StentCenterLineSerial.push_back(p[0]);
				//m_StentCenterLineSerial.push_back(p[1]);
				//m_StentCenterLineSerial.push_back(p[2]);

			}
			centerLineStart = m_StentCenterLine.begin();
			centerLineEnd = m_StentCenterLine.end();
			this->setCenterLine(centerLineStart, centerLineEnd,numOfPoints);

		}//end of if
	}//end of if line
}



/************************************************************************/
/* set center line
calculate the centers for sampling circles.
these centres are located on the vessel centre line 
with intervals successively equal to crown length and length between crowns.
*/
/************************************************************************/ 

void vtkMEDStentModelSource::setCenterLine(vector<vector<double>>::const_iterator centerLineStart,
						vector<vector<double>>::const_iterator centerLineEnd,int pointNumber){
   
	/**
	* calculate the distance between crowns
	*/
	double distanceBetweenCrown = calculateDistanceBetweenCrown();
	int maxNumber = computeCrownNumberAfterSetCenterLine(); //can be compute after set centerline polydata
	if(crownNumber>maxNumber){
		crownNumber = maxNumber;
	}
	//---------------------

	/** To adapt different configuration of stent, 
     * the control mesh have two more circles of vertices in each side than the stent.*/
	nCenterVertex = 2*(crownNumber + 2);
  AllocateCentreLine(nCenterVertex) ;
	
	int *centerIdx = new int [nCenterVertex];

	vector<vector<double>>::const_iterator iterCur = centerLineStart;
	vector<vector<double>>::const_iterator iterNext = iterCur+1;
	double direc[3];
	direc[0] = (*iterCur)[0] - (*iterNext)[0];
	direc[1] = (*iterCur)[1] - (*iterNext)[1];
	direc[2] = (*iterCur)[2] - (*iterNext)[2];
	double magnitude = sqrt(direc[0]*direc[0]+direc[1]*direc[1]+direc[2]*direc[2]);
	/**first two centers, for the extra circle for simplex mesh, no stent strut will locate on it*/
	centerLine[0][0] = (*iterCur)[0] + (crownLength+distanceBetweenCrown)*direc[0]/magnitude;
	centerLine[0][1] = (*iterCur)[1] + (crownLength+distanceBetweenCrown)*direc[1]/magnitude;
	centerLine[0][2] = (*iterCur)[2] + (crownLength+distanceBetweenCrown)*direc[2]/magnitude;
	centerIdx[0] = -1;
	centerLine[1][0] = (*iterCur)[0] + distanceBetweenCrown*direc[0]/magnitude;
	centerLine[1][1] = (*iterCur)[1] + distanceBetweenCrown*direc[1]/magnitude;
	centerLine[1][2] = (*iterCur)[2] + distanceBetweenCrown*direc[2]/magnitude;
	centerIdx[1] = -1;
	
	/**here is where stent model start,  where the stent is located */
	centerLine[2][0] = (*iterCur)[0];
	centerLine[2][1] = (*iterCur)[1];
	centerLine[2][2] = (*iterCur)[2];	
	centerIdx[2] = 0;

	int idx = 1;
	double dis,left[3],right[3],distance;
	for(int i = 3; i<nCenterVertex; i++){
		if(i%2==0) 
			distance = distanceBetweenCrown;
		else
			distance = crownLength;
		while(true){

			if (iterNext != centerLineEnd)
			{
				right[0] = (*iterNext)[0];
				right[1] = (*iterNext)[1];
				right[2] = (*iterNext)[2];
				dis = sqrt((centerLine[i-1][0]-right[0])*(centerLine[i-1][0]-right[0])
						  +(centerLine[i-1][1]-right[1])*(centerLine[i-1][1]-right[1])
						  +(centerLine[i-1][2]-right[2])*(centerLine[i-1][2]-right[2]));
				if(dis > distance)
					break;
				else{
				
					iterCur++;
					iterNext++;
					idx++;
				}
			}else{
				break;
			}
		}
		
		left[0] = (*iterCur)[0];
		left[1] = (*iterCur)[1];
		left[2] = (*iterCur)[2];
		calculateSamplingPoint(centerLine[i-1], centerLine[i], distance, left, right);
		centerIdx[i] = idx;
	}

	centerLocationIndex.clear();
	/**
	 * the beginning and the last circle of vertices on the simplex mesh will only have strutsNumber size
	 * not strutsNumber * 2, to fit the simplex mesh structure constraint.
	 * notice here, we store the beginning circle of vertices on the simplex mesh from 0 ~ (strutsNumber-1)
	 * and store the last circle of vertices on the simplex mesh from strutsNumber ~ (strutsNumber*2-1)
	 */
	int pointListSize = 2*strutsNumber*(nCenterVertex-1);
	for(int i = 0; i < strutsNumber; i++){
		centerLocationIndex.push_back(centerIdx[0]);	
	}
	for(int i = strutsNumber; i < strutsNumber*2;i++){
		centerLocationIndex.push_back(centerIdx[nCenterVertex-1]);
	}
	for(int i = 2*strutsNumber; i < pointListSize; i++){
		centerLocationIndex.push_back(centerIdx[i/(2*strutsNumber)]);
	}

	/** calculate initial stent length*/
	initialStentLength = crownNumber*crownLength
		+ (crownNumber-1)*distanceBetweenCrown;

  delete [] centerIdx ;
}



/*-------------------------
* getters
*----------------------*/
/** get struct length */
double vtkMEDStentModelSource::getStrutLength(){
	strutLength = crownLength/cos(strutAngle/2);;
	return strutLength;
}
/** get link length */
double vtkMEDStentModelSource::getLinkLength(){
	return linkLength;
}
/** get radius */
double vtkMEDStentModelSource::getRadius(){
	return stentDiameter/2;
}
/** get Initial Stent Length */
double vtkMEDStentModelSource::getInitialStentLength(){
	return initialStentLength;
}
/** get Crown Number */
int vtkMEDStentModelSource::getCrownNumber(){
	return crownNumber;
}
/** get Struts Number */
int vtkMEDStentModelSource::getStrutsNumber(){
	return strutsNumber;
}


/**
* be called from outside to create the simplex mesh and stent model
*/
void vtkMEDStentModelSource::createStent(){
	init();
	createStentSimplexMesh();
	createStruts();
	createLinks();
}
//------------------------
/** init basic values
 * if there is not a center line ,then create a default straight line
*/
void vtkMEDStentModelSource::init(){
	nCrownSimplex = crownNumber + 2;
	strutLength = crownLength/cos(strutAngle/2);
	
	 /** To adapt different configuration of stent, 
       * the control mesh have two more circles of vertices in each side than the stent.
	   */
	nCenterVertex = 2*(crownNumber + 2);	
	
	
	if (centerLine != NULL) return;
	
	/*---------------------------
	* if no centerLine is set, then create a default straight centerLine
	* the default: startPosition(0,0,0), direction(1,0,0)
	*------------------------------------*/
	double startPosition[3], direction[3];
	startPosition[0] = 0.0;
	startPosition[1] = 0.0;
	startPosition[2] = 0.0;
	direction[0] = 1.0;
	direction[1] = 0.0;
	direction[2] = 0.0;

	double distanceBetweenCrown = calculateDistanceBetweenCrown();
	initialStentLength = crownNumber*crownLength
		               + (crownNumber-1)*distanceBetweenCrown;

  AllocateCentreLine(nCenterVertex) ;
     
	double position = -1*(crownLength + distanceBetweenCrown);

	for(int i=0;i<nCenterVertex;i++){
			centerLine[i][0] = startPosition[0] + position*direction[0];
			centerLine[i][1] = startPosition[1] + position*direction[1];
			centerLine[i][2] = startPosition[2] + position*direction[2];
			if(i%2==0) 
				position += crownLength;
			else
				position += distanceBetweenCrown;
	}

	int pointListSize = 2*strutsNumber*(nCenterVertex-1);
	for(int i = 0; i < pointListSize; i++){
		centerLocationIndex.push_back(-1);
	}
}

/************************************************************************/
/* create a simplex mesh for deformation and vertices of stent                                                                     */
/************************************************************************/
void vtkMEDStentModelSource::createStentSimplexMesh()
{
  simplexMesh->Initialize() ;
  simplexMesh->GetGeometryData()->Initialize() ;
  simplexMesh->SetLastCellId(0) ;

	//----------------------------------------------------
	//-----sample vertices of simplex mesh for stent--------
	//------------------------------------------------------
	int i,j;
	double normalCircle[3];	
	double theta = vtkMath::DoublePi()/strutsNumber;  //Pi/strutsNumber;
	
	//the number of the beginning and last circle is strutsNumber，others strutsNumber*2
	int sampleNumberPerCircle = 2*strutsNumber;
	
	/*
	* notice here, we store the beginning circle of vertices on the simplex mesh from 0 ~ (strutsNumber-1)
	* and store the last circle of vertices on the simplex mesh from strutsNumber ~ (strutsNumber*2-1)
	* other circles of vertices follows successively, n-th circle occupies from n*strutsNumber ~ (n+1)*strutsNumber-1
	* here,in this comment, n is calculated from 0
	*/
	int sampleArraySize = sampleNumberPerCircle * (nCenterVertex-1); 
	double (*sampleArray)[3] = new double[sampleArraySize][3];

	for(i=0;i<nCenterVertex;i=i+2){
		normalCircle[0] = centerLine[i+1][0] - centerLine[i][0];
		normalCircle[1] = centerLine[i+1][1] - centerLine[i][1];
		normalCircle[2] = centerLine[i+1][2] - centerLine[i][2];
		
		double UVector[3], VVector[3],magnitude;
		/*
		* calculate the outer product of normalCircle with (0,1,0) to get a vector perpendicular to normalCircle
		* here (0,1,0) is a random vector chosed, any vertor which is not paralle works
		* might have little bug here, possibly the normalCircle is paralle to (0,1,0), 
		* in this extreme case, need choose another vector to replace (0,1,0)
		* BETTER ADD A SIMPLE TEST HERE TO AVOID THIS!!!
		*/
		UVector[0] = -1.0*normalCircle[2];//0;
		UVector[1] = 0;//n2;
		UVector[2] = normalCircle[0];//-n1
		if((fabs(UVector[0])+ fabs(UVector[1]) + fabs(UVector[2])) < 0.05){
			UVector[0] = normalCircle[1];
			UVector[1] = -normalCircle[0];
			UVector[2] = 0;
		} 
		magnitude = sqrt(UVector[0]*UVector[0]+UVector[1]*UVector[1] + UVector[2]*UVector[2]);
		UVector[0] *= (stentDiameter/2.0/magnitude);
		UVector[1] *= (stentDiameter/2.0/magnitude);
		UVector[2] *= (stentDiameter/2.0/magnitude);
		//outer product of normalCircle & UVector
		VVector[0] = normalCircle[1]*UVector[2]-normalCircle[2]*UVector[1];
		VVector[1] = normalCircle[2]*UVector[0]-normalCircle[0]*UVector[2];
		VVector[2] = normalCircle[0]*UVector[1]-normalCircle[1]*UVector[0];
		magnitude =sqrt(VVector[0]*VVector[0] + VVector[1]*VVector[1] + VVector[2]*VVector[2]);
		VVector[0] *= (stentDiameter/2.0/magnitude);
		VVector[1] *= (stentDiameter/2.0/magnitude);
		VVector[2] *= (stentDiameter/2.0/magnitude);

		if(i==0){
			for(int n=0;n<strutsNumber;n++){//the beginning(0) circle 
				sampleArray[n][0] = centerLine[i][0] + cos(theta*n*2)*UVector[0] + sin(theta*n*2)*VVector[0];
				sampleArray[n][1] = centerLine[i][1] + cos(theta*n*2)*UVector[1] + sin(theta*n*2)*VVector[1];
				sampleArray[n][2] = centerLine[i][2] + cos(theta*n*2)*UVector[2] + sin(theta*n*2)*VVector[2];
			}
			for(int n=0;n<sampleNumberPerCircle;n++){//the first circle
				sampleArray[sampleNumberPerCircle+n][0] = centerLine[i+1][0] + cos(theta*n)*UVector[0] + sin(theta*n)*VVector[0];
				sampleArray[sampleNumberPerCircle+n][1] = centerLine[i+1][1] + cos(theta*n)*UVector[1] + sin(theta*n)*VVector[1];
				sampleArray[sampleNumberPerCircle+n][2] = centerLine[i+1][2] + cos(theta*n)*UVector[2] + sin(theta*n)*VVector[2];
			}
			continue;
		}
		if(i == nCenterVertex-2){
			for(int n = 0;n<sampleNumberPerCircle;n++){//the second last circle
				sampleArray[i*sampleNumberPerCircle+n][0] = centerLine[i][0] + cos(theta*n)*UVector[0] + sin(theta*n)*VVector[0];
				sampleArray[i*sampleNumberPerCircle+n][1] = centerLine[i][1] + cos(theta*n)*UVector[1] + sin(theta*n)*VVector[1];
				sampleArray[i*sampleNumberPerCircle+n][2] = centerLine[i][2] + cos(theta*n)*UVector[2] + sin(theta*n)*VVector[2];
			}
			for(int n=0;n<strutsNumber;n++){//the last circle
				sampleArray[strutsNumber+n][0] = centerLine[i+1][0] + cos(theta*n*2)*UVector[0] + sin(theta*n*2)*VVector[0];
				sampleArray[strutsNumber+n][1] = centerLine[i+1][1] + cos(theta*n*2)*UVector[1] + sin(theta*n*2)*VVector[1];
				sampleArray[strutsNumber+n][2] = centerLine[i+1][2] + cos(theta*n*2)*UVector[2] + sin(theta*n*2)*VVector[2];
			}
			continue;
		}

		for(int n = 0;n<sampleNumberPerCircle;n++){
			sampleArray[i*sampleNumberPerCircle+n][0] = centerLine[i][0] + cos(theta*n)*UVector[0] + sin(theta*n)*VVector[0];
			sampleArray[i*sampleNumberPerCircle+n][1] = centerLine[i][1] + cos(theta*n)*UVector[1] + sin(theta*n)*VVector[1];
			sampleArray[i*sampleNumberPerCircle+n][2] = centerLine[i][2] + cos(theta*n)*UVector[2] + sin(theta*n)*VVector[2];
			sampleArray[(i+1)*sampleNumberPerCircle+n][0] = centerLine[i+1][0] + cos(theta*n)*UVector[0] + sin(theta*n)*VVector[0];
			sampleArray[(i+1)*sampleNumberPerCircle+n][1] = centerLine[i+1][1] + cos(theta*n)*UVector[1] + sin(theta*n)*VVector[1];
			sampleArray[(i+1)*sampleNumberPerCircle+n][2] = centerLine[i+1][2] + cos(theta*n)*UVector[2] + sin(theta*n)*VVector[2];

		}
	}

	/** Add our vertices to the simplex mesh.*/
	PointType point;
	for(i=0; i < sampleArraySize ; ++i)
    {
		point[0] = sampleArray[i][0];
		point[1] = sampleArray[i][1];
		point[2] = sampleArray[i][2];
		simplexMesh->SetPoint(i, point);
		simplexMesh->SetGeometryData(i, new SimplexMeshGeometryType );
	 }
	delete[] sampleArray;
	
	/** Specify the method used for allocating cells */
   simplexMesh->SetCellsAllocationMethod( SimplexMeshType::CellsAllocatedDynamicallyCellByCell );
   
   /** AddEdge,AddNeighbor,and add the symmetric relationships */
   //-----------------------------------------------------------------------------------
   /* pay attention to the order of adding
   *  the three vertices should be commected anticlockwise
   *  to make sure the normal calculate on the surface of simplex mesh is pointing outwards
   *-------------------------*/

   //-----------------
	/**AddEdge */
	/** the beginning circle -- vertical */
   for(j=0;j<strutsNumber-1;j++){
		simplexMesh->AddEdge(j,j+1);
	}
	simplexMesh->AddEdge(j,0);
	/** the beginning circle -- horizontal */
   for(j=0;j<strutsNumber;j++){
		   simplexMesh->AddEdge(j,sampleNumberPerCircle+j*2);
   }
	/** the middle circles -- vertical */
   for(i=1;i<nCenterVertex-1;i++){
	  for(j=0;j<sampleNumberPerCircle-1;j++){
		  simplexMesh->AddEdge(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j+1);
	  }
	  simplexMesh->AddEdge(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle);
   }
   /** the middle circles -- horizontal */
   for(i=1;i<nCenterVertex-1;i=i+2){
	   for(j=1;j<sampleNumberPerCircle;j=j+2){
		   simplexMesh->AddEdge(i*sampleNumberPerCircle+j,(i+1)*sampleNumberPerCircle+j);
	   }
   }
   for(i=2;i<nCenterVertex-2;i=i+2){
	   for(j=0;j<sampleNumberPerCircle;j=j+2){
		   simplexMesh->AddEdge(i*sampleNumberPerCircle+j,(i+1)*sampleNumberPerCircle+j);
	   }
   }
	/** the last circle -- vertical */
	for(j=strutsNumber;j<2*strutsNumber-1;j++){
		simplexMesh->AddEdge(j,j+1);
	}
	simplexMesh->AddEdge(j,strutsNumber);
   
   /** the last circle -- horizontal */
   i=nCenterVertex-2;
   for(j=0;j<strutsNumber;j++){
		simplexMesh->AddEdge(i*sampleNumberPerCircle+j*2,strutsNumber+j);
	}

   //-------------------------------------------------------------------
   /**AddNeighbor
     *the beginning circle */
   for(j=1;j<strutsNumber-1;j++){
	   simplexMesh->AddNeighbor(j,j-1);
	   simplexMesh->AddNeighbor(j,j+1);
	   simplexMesh->AddNeighbor(j,sampleNumberPerCircle+j*2);
   }
   simplexMesh->AddNeighbor(0,strutsNumber-1);
   simplexMesh->AddNeighbor(0,1);
   simplexMesh->AddNeighbor(0,sampleNumberPerCircle);
   simplexMesh->AddNeighbor(strutsNumber-1,strutsNumber-2);
   simplexMesh->AddNeighbor(strutsNumber-1,0);
   simplexMesh->AddNeighbor(strutsNumber-1,sampleNumberPerCircle+(strutsNumber-1)*2);
   /** the first circle */
   i=1;  
   for(j=1;j<sampleNumberPerCircle-2;j=j+2){
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j+1);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,(i+1)*sampleNumberPerCircle+j);
   }
   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle);
   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,(i+1)*sampleNumberPerCircle+j);
   
   for(j=2;j<sampleNumberPerCircle-1;j=j+2){
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,j/2);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j+1);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
   }
   simplexMesh->AddNeighbor(i*sampleNumberPerCircle, 0);
   simplexMesh->AddNeighbor(i*sampleNumberPerCircle,i*sampleNumberPerCircle+1);
   simplexMesh->AddNeighbor(i*sampleNumberPerCircle,(i+1)*sampleNumberPerCircle-1);
   /** the middle circles */
   for(i=2;i<nCenterVertex-2;i=i+2){
     for(j=2;j<sampleNumberPerCircle-1;j=j+2){
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j+1);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,(i+1)*sampleNumberPerCircle+j);
	}
	simplexMesh->AddNeighbor(i*sampleNumberPerCircle,(i+1)*sampleNumberPerCircle-1);
    simplexMesh->AddNeighbor(i*sampleNumberPerCircle,i*sampleNumberPerCircle+1);
    simplexMesh->AddNeighbor(i*sampleNumberPerCircle,(i+1)*sampleNumberPerCircle);
	for(j=1;j<sampleNumberPerCircle-2;j=j+2){
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,(i-1)*sampleNumberPerCircle+j);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j+1);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
	}
    simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,(i-1)*sampleNumberPerCircle+j);
	simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle);
    simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
    
   }
   for(i=3;i<nCenterVertex-2;i=i+2){
	   for(j=1;j<sampleNumberPerCircle-2;j=j+2){
		simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
		simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j+1);
		simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,(i+1)*sampleNumberPerCircle+j);
	   } 
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
       simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,(i+1)*sampleNumberPerCircle+j);
	   for(j=2;j<sampleNumberPerCircle-1;j=j+2){
		simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,(i-1)*sampleNumberPerCircle+j);
		simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j+1);
		simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);		
	   }
		simplexMesh->AddNeighbor(i*sampleNumberPerCircle,(i-1)*sampleNumberPerCircle);
		simplexMesh->AddNeighbor(i*sampleNumberPerCircle,i*sampleNumberPerCircle+1);
		simplexMesh->AddNeighbor(i*sampleNumberPerCircle,(i+1)*sampleNumberPerCircle-1);
   }

   /** the second last circle */
   i= nCenterVertex-2;
   for(j=1;j<sampleNumberPerCircle-2;j=j+2){
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,(i-1)*sampleNumberPerCircle+j);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j+1);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
	}
    simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,(i-1)*sampleNumberPerCircle+j);
	simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle);
    simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
    for(j=2;j<sampleNumberPerCircle-1;j=j+2){
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j-1);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,i*sampleNumberPerCircle+j+1);
	   simplexMesh->AddNeighbor(i*sampleNumberPerCircle+j,strutsNumber+j/2);
	}
	simplexMesh->AddNeighbor(i*sampleNumberPerCircle,(i+1)*sampleNumberPerCircle-1);
    simplexMesh->AddNeighbor(i*sampleNumberPerCircle,i*sampleNumberPerCircle+1);
    simplexMesh->AddNeighbor(i*sampleNumberPerCircle,strutsNumber);
	//the last circle
	for(j=1;j<strutsNumber-1;j++){
		simplexMesh->AddNeighbor(strutsNumber+j,i*sampleNumberPerCircle+j*2);
		simplexMesh->AddNeighbor(strutsNumber+j,strutsNumber+j+1);
		simplexMesh->AddNeighbor(strutsNumber+j,strutsNumber+j-1);
	}
	simplexMesh->AddNeighbor(strutsNumber+j,i*sampleNumberPerCircle+j*2);
	simplexMesh->AddNeighbor(strutsNumber+j,strutsNumber);
	simplexMesh->AddNeighbor(strutsNumber+j,strutsNumber+j-1);
	simplexMesh->AddNeighbor(strutsNumber,i*sampleNumberPerCircle);
	simplexMesh->AddNeighbor(strutsNumber,strutsNumber+1);
	simplexMesh->AddNeighbor(strutsNumber,sampleNumberPerCircle-1);

//------------------------------------
   /** AddFace */
  OutputCellAutoPointer m_NewSimplexCellPointer;
  /** the beginning circle : cells */
  for(j=0;j<strutsNumber-1;j++){
	m_NewSimplexCellPointer.TakeOwnership(new OutputPolygonType);
	m_NewSimplexCellPointer->SetPointId(0, j);
	m_NewSimplexCellPointer->SetPointId(1, j+1);
	m_NewSimplexCellPointer->SetPointId(2, sampleNumberPerCircle+j*2+2);
	m_NewSimplexCellPointer->SetPointId(3, sampleNumberPerCircle+j*2+1);
	m_NewSimplexCellPointer->SetPointId(4, sampleNumberPerCircle+j*2);
	simplexMesh->AddFace(m_NewSimplexCellPointer);		
  }
   m_NewSimplexCellPointer.TakeOwnership(new OutputPolygonType);
   m_NewSimplexCellPointer->SetPointId(0, j);
   m_NewSimplexCellPointer->SetPointId(1, 0);
   m_NewSimplexCellPointer->SetPointId(2, sampleNumberPerCircle);
   m_NewSimplexCellPointer->SetPointId(3, sampleNumberPerCircle+j*2+1);
   m_NewSimplexCellPointer->SetPointId(4, sampleNumberPerCircle+j*2);
   simplexMesh->AddFace(m_NewSimplexCellPointer);
   /** the last circle : cells */
   i=nCenterVertex-2;
	for(j=0;j<strutsNumber-1;j++){
		   m_NewSimplexCellPointer.TakeOwnership(new OutputPolygonType);
		   m_NewSimplexCellPointer->SetPointId(0, i*sampleNumberPerCircle+j*2);
		   m_NewSimplexCellPointer->SetPointId(1, i*sampleNumberPerCircle+j*2+1);
		   m_NewSimplexCellPointer->SetPointId(2, i*sampleNumberPerCircle+j*2+2);
		   m_NewSimplexCellPointer->SetPointId(3, strutsNumber+j+1);
		   m_NewSimplexCellPointer->SetPointId(4, strutsNumber+j);
		   simplexMesh->AddFace(m_NewSimplexCellPointer);		
	   }
	   m_NewSimplexCellPointer.TakeOwnership(new OutputPolygonType);
	   m_NewSimplexCellPointer->SetPointId(0, i*sampleNumberPerCircle+j*2);
	   m_NewSimplexCellPointer->SetPointId(1, i*sampleNumberPerCircle+j*2+1);
	   m_NewSimplexCellPointer->SetPointId(2, i*sampleNumberPerCircle);
	   m_NewSimplexCellPointer->SetPointId(3, strutsNumber);
	   m_NewSimplexCellPointer->SetPointId(4, strutsNumber+j);
	   simplexMesh->AddFace(m_NewSimplexCellPointer);

  /** the middle circles : cells */
  for(i=2;i<nCenterVertex-2;i=i+2){
	   for(j=0;j<sampleNumberPerCircle-2;j=j+2){
		   m_NewSimplexCellPointer.TakeOwnership(new OutputPolygonType);
		   m_NewSimplexCellPointer->SetPointId(0, i*sampleNumberPerCircle+j);
		   m_NewSimplexCellPointer->SetPointId(1, i*sampleNumberPerCircle+j+1);
		   m_NewSimplexCellPointer->SetPointId(2, i*sampleNumberPerCircle+j+2);
		   m_NewSimplexCellPointer->SetPointId(3, (i+1)*sampleNumberPerCircle+j+2);
		   m_NewSimplexCellPointer->SetPointId(4, (i+1)*sampleNumberPerCircle+j+1);
		   m_NewSimplexCellPointer->SetPointId(5, (i+1)*sampleNumberPerCircle+j);
		   simplexMesh->AddFace(m_NewSimplexCellPointer);		
	   }
	   m_NewSimplexCellPointer.TakeOwnership(new OutputPolygonType);
	   m_NewSimplexCellPointer->SetPointId(0, i*sampleNumberPerCircle+j);
	   m_NewSimplexCellPointer->SetPointId(1, i*sampleNumberPerCircle+j+1);
	   m_NewSimplexCellPointer->SetPointId(2, i*sampleNumberPerCircle);
	   m_NewSimplexCellPointer->SetPointId(3, (i+1)*sampleNumberPerCircle);
	   m_NewSimplexCellPointer->SetPointId(4, (i+1)*sampleNumberPerCircle+j+1);
	   m_NewSimplexCellPointer->SetPointId(5, (i+1)*sampleNumberPerCircle+j);
	   simplexMesh->AddFace(m_NewSimplexCellPointer);
   }

  /** connect struts to struts */
   for(i=1;i<nCenterVertex-1;i=i+2){
	   for(j=1;j<sampleNumberPerCircle-2;j=j+2){
		   m_NewSimplexCellPointer.TakeOwnership(new OutputPolygonType);
		   m_NewSimplexCellPointer->SetPointId(0, i*sampleNumberPerCircle+j);
		   m_NewSimplexCellPointer->SetPointId(1, i*sampleNumberPerCircle+j+1);
		   m_NewSimplexCellPointer->SetPointId(2, i*sampleNumberPerCircle+j+2);
		   m_NewSimplexCellPointer->SetPointId(3, (i+1)*sampleNumberPerCircle+j+2);
		   m_NewSimplexCellPointer->SetPointId(4, (i+1)*sampleNumberPerCircle+j+1);
		   m_NewSimplexCellPointer->SetPointId(5, (i+1)*sampleNumberPerCircle+j);
		   simplexMesh->AddFace(m_NewSimplexCellPointer);		
	   }
	    m_NewSimplexCellPointer.TakeOwnership(new OutputPolygonType);
		m_NewSimplexCellPointer->SetPointId(0, i*sampleNumberPerCircle+j);
		m_NewSimplexCellPointer->SetPointId(1, i*sampleNumberPerCircle);
		m_NewSimplexCellPointer->SetPointId(2, i*sampleNumberPerCircle+1);
		m_NewSimplexCellPointer->SetPointId(3, (i+1)*sampleNumberPerCircle+1);
		m_NewSimplexCellPointer->SetPointId(4, (i+1)*sampleNumberPerCircle);
		m_NewSimplexCellPointer->SetPointId(5, (i+1)*sampleNumberPerCircle+j);
		simplexMesh->AddFace(m_NewSimplexCellPointer);	
   }
//  simplexMesh->BuildCellLinks();
}




void vtkMEDStentModelSource::createStruts(){
  strutsList.clear() ;

  Strut theStrut;
	int i,j;
	int sampleNumberPerCircle = 2*strutsNumber;   

	/** the even circles, counted from zero */	
	for(j=2;j<nCenterVertex-2;j=j+4){
		for(i=0;i<sampleNumberPerCircle-2;i=i+2){
			theStrut.startVertex = sampleNumberPerCircle*j+i;
			theStrut.endVertex = sampleNumberPerCircle*(j+1)+i+1;
			strutsList.push_back(theStrut);
			theStrut.startVertex = sampleNumberPerCircle*j+i+2;
			theStrut.endVertex = sampleNumberPerCircle*(j+1)+i+1;
			strutsList.push_back(theStrut);		
		}
		theStrut.startVertex = sampleNumberPerCircle*j+i;
		theStrut.endVertex = sampleNumberPerCircle*(j+1)+i+1;
		strutsList.push_back(theStrut);
		theStrut.startVertex = sampleNumberPerCircle*j;
		theStrut.endVertex = sampleNumberPerCircle*(j+1)+i+1;
		strutsList.push_back(theStrut);
	}
		
	/** the odd circles, counted from zero */
	if(stentConfiguration == OutOfPhase){
		for(j=4;j<nCenterVertex-2;j=j+4){
			for(i=1;i<sampleNumberPerCircle-2;i=i+2){
				theStrut.startVertex = sampleNumberPerCircle*j+i;
				theStrut.endVertex = sampleNumberPerCircle*(j+1)+i-1;
				strutsList.push_back(theStrut);
				theStrut.startVertex = sampleNumberPerCircle*j+i;
				theStrut.endVertex = sampleNumberPerCircle*(j+1)+i+1;
				strutsList.push_back(theStrut);		
			}
			theStrut.startVertex = sampleNumberPerCircle*j+i;
			theStrut.endVertex = sampleNumberPerCircle*(j+1)+i-1;
			strutsList.push_back(theStrut);
			theStrut.startVertex = sampleNumberPerCircle*j+i;
			theStrut.endVertex = sampleNumberPerCircle*(j+1);
			strutsList.push_back(theStrut);
		}
	} else if (stentConfiguration == InPhase){
		for(j=4;j<nCenterVertex-2;j=j+4){
			for(i=0;i<sampleNumberPerCircle-2;i=i+2){
				theStrut.startVertex = sampleNumberPerCircle*j+i;
				theStrut.endVertex = sampleNumberPerCircle*(j+1)+i+1;
				strutsList.push_back(theStrut);
				theStrut.startVertex = sampleNumberPerCircle*j+i+2;
				theStrut.endVertex = sampleNumberPerCircle*(j+1)+i+1;
				strutsList.push_back(theStrut);		
			}
			theStrut.startVertex = sampleNumberPerCircle*j+i;
			theStrut.endVertex = sampleNumberPerCircle*(j+1)+i+1;
			strutsList.push_back(theStrut);
			theStrut.startVertex = sampleNumberPerCircle*j;
			theStrut.endVertex = sampleNumberPerCircle*(j+1)+i+1;
			strutsList.push_back(theStrut);	
		}
	}
}

/**
  *different link types: 12 kinds + different alignment
  *	6 kinds for out of phase:p2p+N; v2v+N; p2v+1;p2v-1;v2p+1;v2p-1
  * 6 kinds for in phase:p2v+N; v2P+N; p2p+1;p2p-1;v2v+1;v2v-1
*/
void vtkMEDStentModelSource::createLinks(){
  linkList.clear() ;

	Strut theLink;
	int i,j;
	int sampleNumberPerCircle = 2*strutsNumber;
	int interval = (strutsNumber / linkNumber)*2;
	int indexFirstLink=0;
	int indexCurrent;
	/**--------OUT OF PHASE---------------*/
	if(stentConfiguration == OutOfPhase){ 
		
		if(linkConnection == peak2peak && linkOrientation == None){ /** out of phase + p2p + N */
			
			indexFirstLink = 1;
			for(j=1;j<nCrownSimplex-2;j++){
				for(i=0;i<linkNumber;i=i++){
					indexCurrent = (indexFirstLink + i*interval)%sampleNumberPerCircle;
					theLink.startVertex = sampleNumberPerCircle * (j*2+1) + indexCurrent;
					theLink.endVertex = sampleNumberPerCircle *(j*2+2) + indexCurrent;
					linkList.push_back(theLink);
				}
				indexFirstLink = (indexFirstLink + (linkAlignment*2-1))%sampleNumberPerCircle;
			}

		}else if(linkConnection == valley2valley && linkOrientation == None){/** out of phase + v2v + N */

			indexFirstLink = 0;

			for(j=1;j<nCrownSimplex-2;j++){
				for(i=0;i<linkNumber;i++){
					indexCurrent = (indexFirstLink + i*interval)%sampleNumberPerCircle;
					theLink.startVertex = sampleNumberPerCircle * (j*2) + indexCurrent;
					theLink.endVertex = sampleNumberPerCircle *(j*2+3) + indexCurrent;
					linkList.push_back(theLink);
				}
				indexFirstLink = (indexFirstLink + (linkAlignment*2-1))%sampleNumberPerCircle;
			}

		
		}else if(linkConnection == peak2valley && linkOrientation != None){ /** out of phase, p2v, +/-1 */
			
			int offset;
			if(linkOrientation == PositiveOne) offset = 1;
			if(linkOrientation == NegativeOne) offset = -1;
			
			indexFirstLink = 1;

			int indexCurrent2;
			for(j=1;j<nCrownSimplex-2;j++){
				for(i=0;i<linkNumber;i++){
					indexCurrent = (indexFirstLink + i*interval)%sampleNumberPerCircle;
					indexCurrent2 = (indexCurrent + offset + sampleNumberPerCircle)%sampleNumberPerCircle;
					theLink.startVertex = sampleNumberPerCircle * (j*2+1) + indexCurrent;
					theLink.endVertex = sampleNumberPerCircle *(j*2+3) + indexCurrent2;
					linkList.push_back(theLink);
				}
				indexFirstLink = (indexFirstLink + (linkAlignment*2-1))%sampleNumberPerCircle;
			}

		}else if(linkConnection == valley2peak && linkOrientation != None){
			int offset;
			if(linkOrientation == PositiveOne) offset = 1;
			if(linkOrientation == NegativeOne) offset = -1;
			
			indexFirstLink = 0;
			int indexCurrent2;
			for(j=1;j<nCrownSimplex-2;j++){
				for(i=0;i<linkNumber;i++){
					indexCurrent = (indexFirstLink + i*interval)%sampleNumberPerCircle;
					indexCurrent2 = (indexCurrent + offset + sampleNumberPerCircle)%sampleNumberPerCircle;
					theLink.startVertex = sampleNumberPerCircle * (j*2) + indexCurrent;
					theLink.endVertex = sampleNumberPerCircle*(j*2+2) + indexCurrent2;
					linkList.push_back(theLink);
				}
				indexFirstLink = (indexFirstLink + (linkAlignment*2-1))%sampleNumberPerCircle;
			}
		
		}else{
			std::cout << "please check link settings"<<std::endl;
		}
	
	}else{ /** -------------IN PHASE----------------- */
		if(linkConnection == peak2valley && linkOrientation == None){ /** p2v */
				
			indexFirstLink = 1;
			for(j=1;j<nCrownSimplex-2;j++){
				for(i=0;i<linkNumber;i++){
					indexCurrent = (indexFirstLink + i*interval)%sampleNumberPerCircle;
					theLink.startVertex = sampleNumberPerCircle*(j*2+1) + indexCurrent;
					theLink.endVertex = sampleNumberPerCircle*(j*2+3) + indexCurrent;
					linkList.push_back(theLink);
				}
				indexFirstLink = (indexFirstLink + (linkAlignment*2))%sampleNumberPerCircle;
			}
	
		}else if(linkConnection == valley2peak && linkOrientation == None){/** v2p */
			
			indexFirstLink = 0;
			for(j=1;j<nCrownSimplex-2;j++){
				for(i=0;i<linkNumber;i++){
					indexCurrent = (indexFirstLink + i*interval)%sampleNumberPerCircle;
					theLink.startVertex = sampleNumberPerCircle*(j*2) + indexCurrent;
					theLink.endVertex = sampleNumberPerCircle*(j*2+2) + indexCurrent;
					linkList.push_back(theLink);
				}
				indexFirstLink = (indexFirstLink + (linkAlignment*2))%sampleNumberPerCircle;
			}
		
		}else if(linkConnection == peak2peak && linkOrientation != None){/** p2p */
			int offset;
			if(linkOrientation == PositiveOne) offset = 1;
			if(linkOrientation == NegativeOne) offset = -1;

			indexFirstLink = 1;
			int indexCurrent2;
			for(j=1;j<nCrownSimplex-2;j++){
				for(i=0;i<linkNumber;i++){
					indexCurrent = (indexFirstLink + i*interval)%sampleNumberPerCircle;
					indexCurrent2 = (indexCurrent + offset + sampleNumberPerCircle)%sampleNumberPerCircle;
					theLink.startVertex = sampleNumberPerCircle * (j*2+1) + indexCurrent;
					theLink.endVertex = sampleNumberPerCircle*(j*2+2) + indexCurrent2;
					linkList.push_back(theLink);
				}
				indexFirstLink = (indexFirstLink + (linkAlignment*2))%sampleNumberPerCircle;
			}
		
		}else if(linkConnection == valley2valley && linkOrientation != None){/** v2v */
			int offset;
			if(linkOrientation == PositiveOne) offset = 1;
			if(linkOrientation == NegativeOne) offset = -1;

			indexFirstLink = 0;
			int indexCurrent2;
	
			for(j=1;j<nCrownSimplex-2;j++){
				for(i=0;i<linkNumber;i++){
					indexCurrent = (indexFirstLink + i*interval)%sampleNumberPerCircle;
					indexCurrent2 = (indexCurrent + offset + sampleNumberPerCircle)%sampleNumberPerCircle;
					theLink.startVertex = sampleNumberPerCircle*(j*2) + indexCurrent;
					theLink.endVertex = sampleNumberPerCircle*(j*2+3) + indexCurrent2;
					linkList.push_back(theLink);
				}
				indexFirstLink = (indexFirstLink + (linkAlignment*2))%sampleNumberPerCircle;
			}
		
		}else{
			std::cout << "please check link settings"<<std::endl;
		}	
	}
}
/**  create  intervals points successively equal to 
crown length and length between crowns*/
void vtkMEDStentModelSource::calculateSamplingPoint(double *preSamplePoint, double* samplePoint, 
											  double distance, double* left, double* right){
	double middle[3];
	middle[0] = (left[0]+right[0])/2;
	middle[1] = (left[1]+right[1])/2;
	middle[2] = (left[2]+right[2])/2;
	double dis;
	dis = sqrt((middle[0]-preSamplePoint[0])*(middle[0]-preSamplePoint[0])
		      +(middle[1]-preSamplePoint[1])*(middle[1]-preSamplePoint[1])
			  +(middle[2]-preSamplePoint[2])*(middle[2]-preSamplePoint[2]));
	if(fabs(dis-distance)<0.05) {
		samplePoint[0] = middle[0];
		samplePoint[1] = middle[1];
		samplePoint[2] = middle[2];
		return;
	}else if(dis>distance){
		calculateSamplingPoint(preSamplePoint, samplePoint,distance,left,middle);
	}else{
		calculateSamplingPoint(preSamplePoint, samplePoint,distance,middle,right);
	}
}
/** to calculate how many crowns could be set on a centerline */
int vtkMEDStentModelSource::computeCrownNumberAfterSetCenterLine(){
	int rtn = 0;
	
	double lineLength = this->centerLineLength;//to do compute lineLength
	double gapLength;
	gapLength = calculateDistanceBetweenCrown();
	//lineLength = lineLength - crownLength ;
	
	rtn = floor( lineLength / (crownLength + gapLength))-2;
	return rtn ;
}
/** calculate the distance between crowns */
double vtkMEDStentModelSource::calculateDistanceBetweenCrown(){

	double distanceBetweenCrown = linkLength;
	double horizontalLength = 0;
	strutAngle = 2*atan(stentDiameter*(vtkMath::DoublePi())/strutsNumber/2/crownLength);
	if(linkOrientation == None){
		if(linkConnection == peak2peak)
			distanceBetweenCrown = linkLength;
		else if (linkConnection == valley2valley)
			distanceBetweenCrown = linkLength-2*crownLength;
		else
			distanceBetweenCrown = linkLength-crownLength;
	}else{ /** link orientation == PositiveOne or NegativeOne */
		horizontalLength = sqrt(linkLength*linkLength
			                 - (crownLength*tan(strutAngle/2))*(crownLength*tan(strutAngle/2)));
		if(linkConnection == peak2peak){			
			distanceBetweenCrown = horizontalLength;
		}
		else if (linkConnection == valley2valley){
			distanceBetweenCrown = horizontalLength-2*crownLength;
		}
		else {
			distanceBetweenCrown = horizontalLength-crownLength;
		}
	}
	return distanceBetweenCrown;
}



//----------------------------------------------------------------------------
// Allocate or reallocate centerline
//----------------------------------------------------------------------------
void vtkMEDStentModelSource::AllocateCentreLine(int n)
{
  if (centerLine != NULL){
    if (n > m_NumberOfPointsAllocatedToCenterLine){
      // delete existing allocation
      delete [] centerLine ;
      centerLine = NULL ;
      m_NumberOfPointsAllocatedToCenterLine = 0 ;
    }
    else{
      // already enough allocated - do nothing
      return ;
    }
  }

  // allocate
  if (n > 0){
    centerLine = new double[n][3];
    m_NumberOfPointsAllocatedToCenterLine = n ;
  }
}
