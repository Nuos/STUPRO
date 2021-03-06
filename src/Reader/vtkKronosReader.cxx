#include <Reader/vtkKronosReader.h>

#include <Reader/DataReader/JsonReaderFactory.hpp>
#include <Reader/DataReader/JsonReader.hpp>
#include <Reader/DataReader/Data.hpp>
#include <Utils/Config/Configuration.hpp>

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkCommand.h>
#include <vtkObjectFactory.h>
#include <vtkExecutive.h>

#include <math.h>
#include <algorithm>
#include <vector>

vtkStandardNewMacro(vtkKronosReader);

vtkKronosReader::vtkKronosReader() : error(false), zoomLevel(0), cachingInThread(false) {
	// Initialize values that are read from the program configuration
	try {
		this->globeRadius = Configuration::getInstance().getDouble("globe.radius");
		this->highestAltitude = Configuration::getInstance()
		                        .getDouble("dataReader.highestAltitude");
		this->maximumPriority = Configuration::getInstance()
		                        .getInteger("dataReader.maximumPriority");
	} catch (ConfigurationException& e) {
		this->fail(QString("Configuration error while initializing the reader: %1").arg(e.what()));
	}

	this->SetNumberOfInputPorts(0);
	this->SetNumberOfOutputPorts(1);
}

vtkKronosReader::~vtkKronosReader() {
	this->abortCaching();
	this->jsonReader.reset();
}

void vtkKronosReader::abortCaching() {
	if (this->jsonReader && this->cachingInThread) {
		this->jsonReader->abortCaching();
		this->cacheThread.join();
	}
}

void vtkKronosReader::SetFileName(std::string name, bool startCaching) {
	this->fileName = QString::fromStdString(name);

	try {
		this->jsonReader = JsonReaderFactory::createReader(this->fileName);
	} catch (const ReaderException& e) {
		this->fail(e.what());
		return;
	}

	if (startCaching) {
		this->cacheThread = std::thread(&JsonReader::cacheAllData, this->jsonReader.get());
		this->cachingInThread = true;
	}
}

void vtkKronosReader::SetFileName(std::string name) {
	this->SetFileName(name, true);
}

void vtkKronosReader::SetCameraPosition(double x, double y, double z) {
	this->cameraPosition = Vector3d(x, y, z);
	int oldZoomLevel = this->zoomLevel;
	this->recalculateZoomLevel();

	if (oldZoomLevel != this->zoomLevel) {
		this->Modified();
	}
}

void vtkKronosReader::recalculateZoomLevel() {
	double stepSize = (this->highestAltitude - this->globeRadius) / this->maximumPriority;
	// int zoom = int(5 * pow((this->cameraPosition.length() - this->globeRadius) / stepSize, 0.35));
	int zoom = std::round(-3.2676 * std::log(0.0015571 * (this->cameraPosition.length() -
	                      this->globeRadius)));

	// Cap the zoom level to a value between zero and the maximum priority
	zoom = std::min(zoom, this->maximumPriority);
	zoom = std::max(zoom, 0);

	this->zoomLevel = zoom;
}

void vtkKronosReader::fail(QString message) {
	vtkErrorMacro( << message.toStdString());
	this->error = true;
}

int vtkKronosReader::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector,
                                        vtkInformationVector* outputVector) {
	if (this->error) {
		return 1;
	}

	vtkInformation* outInfo = outputVector->GetInformationObject(0);

	// Add information to the output vector if the data contains time information
	if (this->jsonReader->hasTemporalData()) {
		int amountOfTimeSteps = this->jsonReader->getAmountOfTimeSteps();
		std::vector<double> timeSteps;
		for (int i = 0; i < amountOfTimeSteps; i++) {
			timeSteps.push_back(i);
		}
		outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), &timeSteps[0], amountOfTimeSteps);

		double timeRange[2];
		timeRange[0] = 0.0;
		timeRange[1] = amountOfTimeSteps;
		outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), timeRange, 2);
	}

	// Append the data type as an entry to the output information
	outInfo->Set(Data::VTK_DATA_TYPE(), this->jsonReader->getDataType());
	request->Append(vtkExecutive::KEYS_TO_COPY(), Data::VTK_DATA_TYPE());

	// Initialise the data state as an entry to the output information
	outInfo->Set(Data::VTK_DATA_STATE(), Data::RAW);
	request->Append(vtkExecutive::KEYS_TO_COPY(), Data::VTK_DATA_STATE());

	// Initialize the transformation info
	outInfo->Set(Data::VTK_DATA_TRANSFORMATION(), Data::UNTRANSFORMED);
	request->Append(vtkExecutive::KEYS_TO_COPY(), Data::VTK_DATA_TRANSFORMATION());

	// If applicable, append the time resolution as an entry to the output information
	if (this->jsonReader->hasTemporalData()) {
		outInfo->Set(Data::VTK_TIME_RESOLUTION(), this->jsonReader->getTimeResolution());
		request->Append(vtkExecutive::KEYS_TO_COPY(), Data::VTK_TIME_RESOLUTION());
	}

	return 1;
}

int vtkKronosReader::RequestData(vtkInformation*, vtkInformationVector**,
                                 vtkInformationVector* outputVector) {
	if (this->error) {
		return 1;
	}

	vtkInformation* outInfo = outputVector->GetInformationObject(0);
	vtkSmartPointer<vtkPolyData> output = vtkPolyData::SafeDownCast(
	        outInfo->Get(vtkDataObject::DATA_OBJECT())
	                                      );

	if (this->jsonReader != nullptr) {
		vtkSmartPointer<vtkPolyData> polyData;

		if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()) &&
		        this->jsonReader->hasTemporalData()) {
			// The data request has time information attached that should be extracted and used

			double requestedTimeValue = outInfo->Get(
			                                vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()
			                            );

			polyData = this->jsonReader->getVtkDataSet(
			               this->zoomLevel,
			               (int) requestedTimeValue
			           );

			polyData->GetInformation()->Set(vtkDataObject::DATA_TIME_STEP(), requestedTimeValue);
		} else {
			// There is no time information or the data is not time-sensitive

			polyData = this->jsonReader->getVtkDataSet(
			               this->zoomLevel
			           );
		}

		output->DeepCopy(polyData);
	}

	return 1;
}
