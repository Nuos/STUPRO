#include <Filter/AbstractSelectionFilter.hpp>

#include <vtkPolyData.h>
#include <vtkDataObject.h>
#include <vtkAlgorithm.h>
#include <vtkCellArray.h>
#include <vtkExecutive.h>
#include <vtkInformationExecutivePortVectorKey.h>

AbstractSelectionFilter::AbstractSelectionFilter() { }

AbstractSelectionFilter::~AbstractSelectionFilter() { }

void AbstractSelectionFilter::fail(QString message) {
	vtkErrorMacro( << QString("%1 This filter may not work, please proceed with caution.").arg(
	                   message).toStdString());
}

int AbstractSelectionFilter::RequestData(vtkInformation* info,
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector) {
	// Get input and output data from the information vectors
	vtkInformation* inputInformation = inputVector[0]->GetInformationObject(0);
	vtkPointSet* inputData = vtkPointSet::SafeDownCast(inputInformation->Get(
	                             vtkDataObject::DATA_OBJECT()));

	vtkInformation* outputInformation = outputVector->GetInformationObject(0);
	vtkPolyData* output = vtkPolyData::SafeDownCast(outputInformation->Get(
	                          vtkDataObject::DATA_OBJECT()));

	// Create a list of the indices of all points that should be kept by evaluating each one
	QList<int> selectedPoints;

	for (int i = 0; i < inputData->GetNumberOfPoints(); i++) {
		double coordinates[3];
		inputData->GetPoint(i, coordinates);
		if (this->evaluatePoint(i, Coordinate(coordinates[0], coordinates[1]), inputData->GetPointData())) {
			selectedPoints.append(i);
		}
	}

	// Create the content of the output poly data object
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
	vertices->Allocate(vertices->EstimateSize(1, selectedPoints.size()));
	vertices->InsertNextCell(selectedPoints.size());

	// Create all arrays from the input data
	QList<vtkSmartPointer<vtkAbstractArray>> inputArrays;
	QList<vtkSmartPointer<vtkAbstractArray>> outputArrays;

	for (int i = 0; i < inputData->GetPointData()->GetNumberOfArrays(); i++) {
		vtkSmartPointer<vtkAbstractArray> inputArray = inputData->GetPointData()->GetAbstractArray(i);

		if (!inputArray) {
			this->fail("An input array could not be read.");
			return 0;
		}

		vtkSmartPointer<vtkAbstractArray> outputArray = vtkAbstractArray::CreateArray(
		            inputArray->GetDataType());

		outputArray->SetNumberOfComponents(inputArray->GetNumberOfComponents());
		outputArray->SetNumberOfTuples(selectedPoints.size());
		outputArray->SetName(inputArray->GetName());

		inputArrays.append(inputArray);
		outputArrays.append(outputArray);
	}

	// Fill the output poly data object with the coordinates of all selected points
	QList<int>::iterator i;
	int tupleNumber = 0;
	for (i = selectedPoints.begin(); i != selectedPoints.end(); ++i) {
		double coordinates[3];
		inputData->GetPoint(*i, coordinates);
		vertices->InsertCellPoint(points->InsertNextPoint(coordinates[0], coordinates[1], coordinates[2]));

		// Copy over all scalars
		for (int j = 0; j < inputArrays.size(); j++) {
			outputArrays.at(j)->SetTuple(tupleNumber, *i, inputArrays.at(j));
		}

		tupleNumber++;
	}

	// Assign the created point set to the output object
	output->SetPoints(points);
	output->SetVerts(vertices);

	// Add the output arrays to the data set
	QList<vtkSmartPointer<vtkAbstractArray>>::iterator j;
	for (j = outputArrays.begin(); j != outputArrays.end(); ++j) {
		output->GetPointData()->AddArray(*j);
	}

	return 1;
}

int AbstractSelectionFilter::RequestInformation(vtkInformation* request,
        vtkInformationVector** inputVector,
        vtkInformationVector* outputVector) {
	vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
	vtkInformation* outInfo = outputVector->GetInformationObject(0);

	// Create a human-readable string of all supported data types for potentially showing an error message
	QString supportedTypes = "";
	int amountOfSupportedDataTypes = this->getCompatibleDataTypes().size();
	if (amountOfSupportedDataTypes == 1) {
		supportedTypes.append(Data::getDataTypeName(this->getCompatibleDataTypes().value(0)));
	} else if (amountOfSupportedDataTypes > 1) {
		for (int i = 0; i < amountOfSupportedDataTypes - 2; i++) {
			supportedTypes.append(Data::getDataTypeName(this->getCompatibleDataTypes().value(i)));
			if (i < amountOfSupportedDataTypes - 3) {
				supportedTypes.append(", ");
			}
		}
		supportedTypes.append(" and ").append(Data::getDataTypeName(this->getCompatibleDataTypes().value(
		        amountOfSupportedDataTypes - 1)));
	}

	// Check the meta information containing the data's type
	if (inInfo->Has(Data::VTK_DATA_TYPE())) {
		Data::Type dataType = static_cast<Data::Type>(inInfo->Get(Data::VTK_DATA_TYPE()));
		if (!this->getCompatibleDataTypes().contains(dataType)) {
			this->fail(
			    QString("This filter only works with %1 data, but the input contains %2 data.").arg(supportedTypes,
			            Data::getDataTypeName(dataType)));
			return 0;
		}
	} else {
		this->fail("This filter only works with data read by the Kronos reader.");
		return 0;
	}

	// Check the meta information containing the data's state
	if (vtkExecutive::CONSUMERS()->Length(outInfo) == 0) {
		if (inInfo->Has(Data::VTK_DATA_STATE())) {
			Data::State dataState = static_cast<Data::State>(inInfo->Get(Data::VTK_DATA_STATE()));
			if (dataState != Data::RAW) {
				this->fail(
				    QString("This filter only works with raw input data, but the input data has the state %1.").arg(
				        Data::getDataStateName(dataState)));
				return 0;
			}
		} else {
			this->fail("The input data has no data state information attached.");
			return 0;
		}
	}

	outInfo->Set(Data::VTK_DATA_STATE(), Data::RAW);

	return 1;
}

void AbstractSelectionFilter::PrintSelf(ostream& os, vtkIndent indent) {
	this->Superclass::PrintSelf(os, indent);
	os << indent << "Filter for selecting and extracting certain data points, Kronos Project" <<
	   endl;
}

int AbstractSelectionFilter::FillOutputPortInformation(int port, vtkInformation* info) {
	info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");

	return 1;
}

int AbstractSelectionFilter::FillInputPortInformation(int port, vtkInformation* info) {
	if (port == 0) {
		info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
		info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPointSet");
		info->Append(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkUnstructuredGrid");
		info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 0);
	}

	return 1;
}
