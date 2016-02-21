#include "SciVisFilter.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include <vtkThreshold.h>
#include "vtkPoints.h"
#include "vtkDataSetAlgorithm.h"
#include <vtkInformation.h>
#include "vtkInformationVector.h"
#include <vtkIndent.h>
#include <vtkDataSet.h>
#include <vtkUnstructuredGridAlgorithm.h>
#include <iostream>

vtkStandardNewMacro(SciVisFilter);
SciVisFilter::SciVisFilter() {}

SciVisFilter::~SciVisFilter() {}


void SciVisFilter::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent) ;
}

int	SciVisFilter::RequestData (vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector) {

//vtkInformation *inInfo = inputVector[4]->GetInformationObject(0);
//vtkInformation *outInfo = outputVector->GetInformationObject(0);
//std::cout << inInfo;
//outInfo=inInfo;

    return 1;
}

//int SciVisFilter::FillInputPortInformation (int port, vtkInformation *info){
//this->Superclass::FillInputPortInformation(port,info);
//return 1;
//}

//int SciVisFilter::FillOutputPortInformation(int port, vtkInformation *info){
//  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPointSet");
//  return 1;
//}


void SciVisFilter::chooseDateMIPAS(double MIPASTime) {
  this->MIPASTime = MIPASTime;
}

void SciVisFilter::chooseDateAIRS(double AIRSTime){
  this->AIRSTime = AIRSTime;
}

void SciVisFilter::chooseDateCLaMS(double CLaMSTime){
  this->CLaMSTime = CLaMSTime;
}
