//
//  SpericalToCartesianFilter.hpp
//  kronos
//
//  Created by Larissa Laich on 04.12.15.
//
//

#ifndef SpericalToCartesianFilter_hpp
#define SpericalToCartesianFilter_hpp

#include "vtkDataSetAlgorithm.h"
#include <stdio.h>

class SpericalToCartesianFilter : public vtkDataSetAlgorithm{
public:
    vtkTypeMacro(SpericalToCartesianFilter, vtkDataSetAlgorithm);
    static SpericalToCartesianFilter *New();
    int ProcessRequest(vtkInformation *request, vtkInformationVector **inputVector, vtkInformationVector *outputVector) override;
    void PrintSelf(ostream& os, vtkIndent indent) override;
protected:
    SpericalToCartesianFilter(){
        
    };
    ~SpericalToCartesianFilter(){};
    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

private:
    SpericalToCartesianFilter(const SpericalToCartesianFilter&);  // Not implemented.
    void operator=(const SpericalToCartesianFilter&);  // Not implemented.
    
};


#endif /* SpericalToCartesianFilter_hpp */
