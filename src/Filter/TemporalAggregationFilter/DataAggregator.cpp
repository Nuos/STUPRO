#include <Filter/TemporalAggregationFilter/DataAggregator.hpp>

#include <Filter/TemporalAggregationFilter/PrecipitationAggregationValue.hpp>
#include <Filter/TemporalAggregationFilter/TemperatureAggregationValue.hpp>
#include <Filter/TemporalAggregationFilter/WindAggregationValue.hpp>
#include <Filter/TemporalAggregationFilter/CloudCoverageAggregationValue.hpp>
#include <Utils/Misc/Macros.hpp>

#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>

DataAggregator::DataAggregator() : lastTimeIndex(0) { }

DataAggregator::~DataAggregator() {
	this->clearAggregationData();
}

void DataAggregator::setDataSetAttributes(Data::Type dataType, int timeResolution) {
	this->dataType = dataType;
	this->timeResolution = timeResolution;
}

void DataAggregator::clearAggregationData() {
	qDeleteAll(this->aggregatedData);
	this->aggregatedData.clear();
}

void DataAggregator::addPointData(int pointIndex, PointCoordinates coordinates, int currentTimeStep,
                                  vtkSmartPointer<vtkPointData> pointData) {
	this->lastTimeIndex = currentTimeStep;

	switch (this->dataType) {
	case Data::PRECIPITATION: {
		vtkSmartPointer<vtkDataArray> abstractPrecipitationRateArray =
		    pointData->GetArray("precipitationRates");
		vtkSmartPointer<vtkFloatArray> precipitationRateArray = vtkFloatArray::SafeDownCast(
		            abstractPrecipitationRateArray);

		if (!precipitationRateArray) {
			return;
		}

		double currentPrecipitationRate = precipitationRateArray->GetValue(pointIndex);

		if (this->aggregatedData.contains(coordinates)) {
			// The point has already been looked at before
			PrecipitationAggregationValue* currentValue = static_cast<PrecipitationAggregationValue*>
			        (this->aggregatedData.value(coordinates));

			// Accumulate the precipitation rates over time, therefore getting the total amount of precipitation.
			// To do this, two assumptions are made:
			// 1. The precipitation amount at the beginning of the accumulation is zero.
			// 2. A data point constantly emits precipitation with its precipitation rate until new precipitation rate data is available.
			currentValue->setAccumulatedPrecipitation(currentValue->getAccumulatedPrecipitation() + ((1.0 *
			        (currentTimeStep - currentValue->getTimeIndex()) * this->timeResolution) *
			        currentValue->getLastPrecipitationRate()));
			currentValue->setTimeIndex(currentTimeStep);
			currentValue->setLastPrecipitationRate(currentPrecipitationRate);
		} else {
			// This is the first time a point with these coordinates shows up
			PrecipitationAggregationValue* newValue = new PrecipitationAggregationValue();
			newValue->setLastPrecipitationRate(currentPrecipitationRate);
			newValue->setTimeIndex(currentTimeStep);
			this->aggregatedData.insert(coordinates, newValue);
		}
		break;
	}
	case Data::TEMPERATURE: {
		vtkSmartPointer<vtkDataArray> abstractTemperatureArray = pointData->GetArray("temperatures");
		vtkSmartPointer<vtkFloatArray> temperatureArray = vtkFloatArray::SafeDownCast(
		            abstractTemperatureArray);

		if (!temperatureArray) {
			return;
		}

		double currentTemperature = temperatureArray->GetValue(pointIndex);

		if (this->aggregatedData.contains(coordinates)) {
			// The point has already been looked at before
			TemperatureAggregationValue* currentValue = static_cast<TemperatureAggregationValue*>
			        (this->aggregatedData.value(coordinates));

			// Calculate the arithmetric mean with the cumulative moving average method.
			// This is a bit more costly than the naive way to calculate the average but prevents huge numbers from showing up while summing up all values.
			currentValue->setAverageTemperature((currentTemperature + (currentValue->getTimeIndex() *
			                                     currentValue->getAverageTemperature())) / (currentValue->getTimeIndex() * 1.0 + 1));
			currentValue->setTimeIndex(currentValue->getTimeIndex() + 1);
		} else {
			// This is the first time a point with these coordinates shows up
			TemperatureAggregationValue* newValue = new TemperatureAggregationValue();
			newValue->setAverageTemperature(currentTemperature);
			newValue->setTimeIndex(1);
			this->aggregatedData.insert(coordinates, newValue);
		}
		break;
	}
	case Data::WIND: {
		vtkSmartPointer<vtkDataArray> abstractVelocitiesArray = pointData->GetArray("speeds");
		vtkSmartPointer<vtkFloatArray> velocitiesArray = vtkFloatArray::SafeDownCast(
		            abstractVelocitiesArray);
		vtkSmartPointer<vtkDataArray> abstractBearingsArray = pointData->GetArray("directions");
		vtkSmartPointer<vtkFloatArray> bearingsArray = vtkFloatArray::SafeDownCast(
		            abstractBearingsArray);

		if (!velocitiesArray || !bearingsArray) {
			return;
		}

		double currentVelocity = velocitiesArray->GetValue(pointIndex);
		double currentBearing = bearingsArray->GetValue(pointIndex);

		if (this->aggregatedData.contains(coordinates)) {
			// The point has already been looked at before
			WindAggregationValue* currentValue = static_cast<WindAggregationValue*>(this->aggregatedData.value(
			        coordinates));

			// Calculate the arithmetric mean with the cumulative moving average method.
			// This is a bit more costly than the naive way to calculate the average but prevents huge numbers from showing up while summing up all values.
			currentValue->setAverageVelocity((currentVelocity + (currentValue->getTimeIndex() *
			                                  currentValue->getAverageVelocity())) / (currentValue->getTimeIndex() * 1.0 + 1));
			currentValue->setAverageBearing((currentBearing + (currentValue->getTimeIndex() *
			                                 currentValue->getAverageBearing())) / (currentValue->getTimeIndex() * 1.0 + 1));
			currentValue->setTimeIndex(currentValue->getTimeIndex() + 1);
		} else {
			// This is the first time a point with these coordinates shows up
			WindAggregationValue* newValue = new WindAggregationValue();
			newValue->setAverageVelocity(currentVelocity);
			newValue->setAverageBearing(currentBearing);
			newValue->setTimeIndex(1);
			this->aggregatedData.insert(coordinates, newValue);
		}
		break;
	}
	case Data::CLOUD_COVERAGE: {
		vtkSmartPointer<vtkDataArray> abstractCoverageArray = pointData->GetArray("cloudCovers");
		vtkSmartPointer<vtkFloatArray> coverageArray = vtkFloatArray::SafeDownCast(
		            abstractCoverageArray);

		if (!coverageArray) {
			return;
		}

		double currentCoverage = coverageArray->GetValue(pointIndex);

		if (this->aggregatedData.contains(coordinates)) {
			// The point has already been looked at before
			CloudCoverageAggregationValue* currentValue = static_cast<CloudCoverageAggregationValue*>
			        (this->aggregatedData.value(coordinates));

			// Calculate the arithmetric mean with the cumulative moving average method.
			// This is a bit more costly than the naive way to calculate the average but prevents huge numbers from showing up while summing up all values.
			currentValue->setAverageCloudCoverage((currentCoverage + (currentValue->getTimeIndex() *
			                                       currentValue->getAverageCloudCoverage())) / (currentValue->getTimeIndex() * 1.0 + 1));
			currentValue->setTimeIndex(currentValue->getTimeIndex() + 1);
		} else {
			// This is the first time a point with these coordinates shows up
			CloudCoverageAggregationValue* newValue = new CloudCoverageAggregationValue();
			newValue->setAverageCloudCoverage(currentCoverage);
			newValue->setTimeIndex(1);
			this->aggregatedData.insert(coordinates, newValue);
		}
		break;
	}
	default:
		break;
	}
}

vtkSmartPointer<vtkPolyData> DataAggregator::getPolyData() {
	this->finishAggregation();

	vtkSmartPointer<vtkPolyData> dataSet = vtkSmartPointer<vtkPolyData>::New();

	// Create the content of the output poly data object
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkCellArray> vertices = vtkSmartPointer<vtkCellArray>::New();
	vertices->Allocate(vertices->EstimateSize(1, this->aggregatedData.size()));
	vertices->InsertNextCell(this->aggregatedData.size());

	switch (this->dataType) {
	case Data::PRECIPITATION: {
		// Create the array that will hold the aggregated precipitation amounts
		vtkSmartPointer<vtkFloatArray> aggregatedPrecipitationAmounts =
		    vtkSmartPointer<vtkFloatArray>::New();
		aggregatedPrecipitationAmounts->SetNumberOfComponents(1);
		aggregatedPrecipitationAmounts->SetNumberOfTuples(this->aggregatedData.size());
		aggregatedPrecipitationAmounts->SetName("Accumulated Precipitation Amounts");

		// Iterate over all points in the aggregated data
		QMap<PointCoordinates, AggregationValue*>::iterator i;
		int tupleNumber = 0;
		for (i = this->aggregatedData.begin(); i != this->aggregatedData.end(); ++i) {
			PrecipitationAggregationValue* currentValue = static_cast<PrecipitationAggregationValue*>
			        (i.value());

			// Add the point's coordinates
			vertices->InsertCellPoint(points->InsertNextPoint(i.key().getX(), i.key().getY(), i.key().getZ()));

			// Add the point's accumulated precipitation amounts
			double aggregatedPrecipitationAmount[1] = {
				(double) currentValue->getAccumulatedPrecipitation()
			};
			aggregatedPrecipitationAmounts->SetTuple(tupleNumber, aggregatedPrecipitationAmount);

			tupleNumber++;
		}

		dataSet->GetPointData()->AddArray(aggregatedPrecipitationAmounts);
		break;
	}
	case Data::TEMPERATURE: {
		// Create the array that will hold the aggregated temperatures
		vtkSmartPointer<vtkFloatArray> aggregatedTemperatures =
		    vtkSmartPointer<vtkFloatArray>::New();
		aggregatedTemperatures->SetNumberOfComponents(1);
		aggregatedTemperatures->SetNumberOfTuples(this->aggregatedData.size());
		aggregatedTemperatures->SetName("Average Temperatures");

		// Iterate over all points in the aggregated data
		QMap<PointCoordinates, AggregationValue*>::iterator i;
		int tupleNumber = 0;
		for (i = this->aggregatedData.begin(); i != this->aggregatedData.end(); ++i) {
			TemperatureAggregationValue* currentValue = static_cast<TemperatureAggregationValue*>(i.value());

			// Add the point's coordinates
			vertices->InsertCellPoint(points->InsertNextPoint(i.key().getX(), i.key().getY(), i.key().getZ()));

			// Add the point's average temperatures
			double aggregatedTemperature[1] = {
				(double) currentValue->getAverageTemperature()
			};
			aggregatedTemperatures->SetTuple(tupleNumber, aggregatedTemperature);

			tupleNumber++;
		}

		dataSet->GetPointData()->AddArray(aggregatedTemperatures);
		break;
	}
	case Data::WIND: {
		// Create the array that will hold the aggregated wind speeds
		vtkSmartPointer<vtkFloatArray> aggregatedSpeeds = vtkSmartPointer<vtkFloatArray>::New();
		aggregatedSpeeds->SetNumberOfComponents(1);
		aggregatedSpeeds->SetNumberOfTuples(this->aggregatedData.size());
		aggregatedSpeeds->SetName("Average Wind Speeds");

		// Create the array that will hold the aggregated wind bearings
		vtkSmartPointer<vtkFloatArray> aggregatedDirections =
		    vtkSmartPointer<vtkFloatArray>::New();
		aggregatedDirections->SetNumberOfComponents(1);
		aggregatedDirections->SetNumberOfTuples(this->aggregatedData.size());
		aggregatedDirections->SetName("Average Wind Directions");

		// Iterate over all points in the aggregated data
		QMap<PointCoordinates, AggregationValue*>::iterator i;
		int tupleNumber = 0;
		for (i = this->aggregatedData.begin(); i != this->aggregatedData.end(); ++i) {
			WindAggregationValue* currentValue = static_cast<WindAggregationValue*>(i.value());

			// Add the point's coordinates
			vertices->InsertCellPoint(points->InsertNextPoint(i.key().getX(), i.key().getY(), i.key().getZ()));

			// Add the point's average speed
			double aggregatedSpeed[1] = {
				(double) currentValue->getAverageVelocity()
			};
			aggregatedSpeeds->SetTuple(tupleNumber, aggregatedSpeed);

			// Add the point's average direction
			double aggregatedBearing[1] = {
				(double) currentValue->getAverageBearing()
			};
			aggregatedDirections->SetTuple(tupleNumber, aggregatedBearing);

			tupleNumber++;
		}

		dataSet->GetPointData()->AddArray(aggregatedSpeeds);
		dataSet->GetPointData()->AddArray(aggregatedDirections);
		break;
	}
	case Data::CLOUD_COVERAGE: {
		// Create the array that will hold the aggregated cloud coverage values
		vtkSmartPointer<vtkFloatArray> aggregatedCoverageValues =
		    vtkSmartPointer<vtkFloatArray>::New();
		aggregatedCoverageValues->SetNumberOfComponents(1);
		aggregatedCoverageValues->SetNumberOfTuples(this->aggregatedData.size());
		aggregatedCoverageValues->SetName("Average Cloud Coverage Values");

		// Iterate over all points in the aggregated data
		QMap<PointCoordinates, AggregationValue*>::iterator i;
		int tupleNumber = 0;
		for (i = this->aggregatedData.begin(); i != this->aggregatedData.end(); ++i) {
			CloudCoverageAggregationValue* currentValue = static_cast<CloudCoverageAggregationValue*>
			        (i.value());

			// Add the point's coordinates
			vertices->InsertCellPoint(points->InsertNextPoint(i.key().getX(), i.key().getY(), i.key().getZ()));

			// Add the point's average cloud coverage
			double aggregatedCoverageValue[1] = {
				(double) currentValue->getAverageCloudCoverage()
			};
			aggregatedCoverageValues->SetTuple(tupleNumber, aggregatedCoverageValue);

			tupleNumber++;
		}

		dataSet->GetPointData()->AddArray(aggregatedCoverageValues);
		break;
	}
	default:
		break;
	}

	// Assign the created point set to the output object
	dataSet->SetPoints(points);
	dataSet->SetVerts(vertices);

	return dataSet;
}

void DataAggregator::finishAggregation() {
	if (this->dataType == Data::PRECIPITATION) {
		// The accumulation of precipitation amount must be finished for data points that had no new data available in the time step last accumulated
		QMap<PointCoordinates, AggregationValue*>::iterator i;
		for (i = this->aggregatedData.begin(); i != this->aggregatedData.end(); ++i) {
			PrecipitationAggregationValue* currentValue = static_cast<PrecipitationAggregationValue*>
			        (i.value());

			currentValue->setAccumulatedPrecipitation(currentValue->getAccumulatedPrecipitation() + ((1.0 *
			        (this->lastTimeIndex - currentValue->getTimeIndex() + 1) * this->timeResolution) *
			        currentValue->getLastPrecipitationRate()));
			currentValue->setTimeIndex(this->lastTimeIndex);
		}
	}
}