#include <gtest/gtest.h>

#include <Filter/TemporalInterpolationFilter.h>
#include <Filter/HeatmapDensityFilter.h>
#include <Reader/DataReader/DataPoints/TemporalDataPoints/PrecipitationDataPoint.hpp>
#include <Reader/vtkKronosReader.h>
#include <Utils/Math/Vector3.hpp>

#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkPointData.h>

#include <qlist.h>

TEST(TestTemporalInterpolationFilter, TestPrecipitationData) {
	// Read some test data
	vtkSmartPointer<vtkKronosReader> kronosReader = vtkSmartPointer<vtkKronosReader>::New();
	kronosReader->SetFileName("res/test-data/temporal-interpolation-test/precipitation-test-data.kJson",
	                          false);

	// Set up the filter and its input
	vtkSmartPointer<TemporalInterpolationFilter> filter = TemporalInterpolationFilter::New();
	filter->SetInputConnection(kronosReader->GetOutputPort());
	filter->GetInputInformation()->Set(Data::VTK_DATA_TYPE(), Data::TEMPERATURE);
	filter->GetInputInformation()->Set(Data::VTK_TIME_RESOLUTION(), 1);

	// Test integral values
	QList<float> precipitationRatesOfFirstPoint = QList<float>() << 4.45 << 7.86 << 3.72 << 4.76;
	QList<int> precipitationTypesOfFirstPoint = QList<int>() << PrecipitationDataPoint::RAIN <<
	        PrecipitationDataPoint::SNOW << PrecipitationDataPoint::SLEET << PrecipitationDataPoint::SNOW;

	QList<float> precipitationRatesOfSecondPoint = QList<float>() << 6.49 << 6.1133332 << 5.7366667 <<
	        5.36;
	QList<int> precipitationTypesOfSecondPoint = QList<int>() << PrecipitationDataPoint::NONE <<
	        PrecipitationDataPoint::NONE << PrecipitationDataPoint::HAIL << PrecipitationDataPoint::HAIL;

	for (int t = 0; t < precipitationRatesOfFirstPoint.size() - 1; t++) {
		filter->GetOutputInformation(0)->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), t);
		filter->Update();

		// Run the filter on the input data
		vtkSmartPointer<vtkPolyData> outputDataSet = vtkSmartPointer<vtkPolyData>::New();
		outputDataSet->ShallowCopy(filter->GetPolyDataOutput());

		ASSERT_TRUE(outputDataSet);

		// Extract the filter's output
		vtkSmartPointer<vtkFloatArray> precipitationRateArray = vtkFloatArray::SafeDownCast(
		            outputDataSet->GetPointData()->GetArray("precipitationRates"));
		ASSERT_TRUE(precipitationRateArray);

		vtkSmartPointer<vtkIntArray> precipitationTypeArray = vtkIntArray::SafeDownCast(
		            outputDataSet->GetPointData()->GetArray("precipitationTypes"));
		ASSERT_TRUE(precipitationTypeArray);

		EXPECT_FLOAT_EQ(precipitationRatesOfFirstPoint[t], precipitationRateArray->GetTuple1(0));
		EXPECT_FLOAT_EQ(precipitationTypesOfFirstPoint[t], precipitationTypeArray->GetTuple1(0));

		EXPECT_FLOAT_EQ(precipitationRatesOfSecondPoint[t], precipitationRateArray->GetTuple1(1));
		EXPECT_FLOAT_EQ(precipitationTypesOfSecondPoint[t], precipitationTypeArray->GetTuple1(1));
	}

	// Test an intermediate value
	filter->GetOutputInformation(0)->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), 0.5);
	filter->Update();

	// Run the filter on the input data
	vtkSmartPointer<vtkPolyData> outputDataSet = vtkSmartPointer<vtkPolyData>::New();
	outputDataSet->ShallowCopy(filter->GetPolyDataOutput());

	ASSERT_TRUE(outputDataSet);

	// Extract the filter's output
	vtkSmartPointer<vtkFloatArray> precipitationRateArray = vtkFloatArray::SafeDownCast(
	            outputDataSet->GetPointData()->GetArray("precipitationRates"));
	ASSERT_TRUE(precipitationRateArray);

	vtkSmartPointer<vtkIntArray> precipitationTypeArray = vtkIntArray::SafeDownCast(
	            outputDataSet->GetPointData()->GetArray("precipitationTypes"));
	ASSERT_TRUE(precipitationTypeArray);

	EXPECT_FLOAT_EQ(6.1549997, precipitationRateArray->GetTuple1(0));
	EXPECT_FLOAT_EQ(PrecipitationDataPoint::SNOW, precipitationTypeArray->GetTuple1(0));

	EXPECT_FLOAT_EQ(6.3016663, precipitationRateArray->GetTuple1(1));
	EXPECT_FLOAT_EQ(PrecipitationDataPoint::NONE, precipitationTypeArray->GetTuple1(1));
}

TEST(TestTemporalInterpolationFilter, TestTemperatureData) {
	// Read some test data
	vtkSmartPointer<vtkKronosReader> kronosReader = vtkSmartPointer<vtkKronosReader>::New();
	kronosReader->SetFileName("res/test-data/temporal-interpolation-test/temperature-test-data.kJson",
	                          false);

	// Set up the filter and its input
	vtkSmartPointer<TemporalInterpolationFilter> filter = TemporalInterpolationFilter::New();
	filter->SetInputConnection(kronosReader->GetOutputPort());
	filter->GetInputInformation()->Set(Data::VTK_DATA_TYPE(), Data::TEMPERATURE);
	filter->GetInputInformation()->Set(Data::VTK_TIME_RESOLUTION(), 1);

	// Test integral values
	QList<float> temperaturesOfFirstPoint = QList<float>() << 20.9 << 23.38 << 4.17 << 9.12;
	QList<float> temperaturesOfSecondPoint = QList<float>() << 18.8 << 18.8 << 18.8 << 18.8;

	for (int t = 0; t < temperaturesOfFirstPoint.size() - 1; t++) {
		filter->GetOutputInformation(0)->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), t);
		filter->Update();

		// Run the filter on the input data
		vtkSmartPointer<vtkPolyData> outputDataSet = vtkSmartPointer<vtkPolyData>::New();
		outputDataSet->ShallowCopy(filter->GetPolyDataOutput());

		ASSERT_TRUE(outputDataSet);

		// Extract the filter's output
		vtkSmartPointer<vtkFloatArray> temperatureArray = vtkFloatArray::SafeDownCast(
		            outputDataSet->GetPointData()->GetArray("temperatures"));
		ASSERT_TRUE(temperatureArray);

		EXPECT_FLOAT_EQ(temperaturesOfFirstPoint[t], temperatureArray->GetTuple1(0));
		EXPECT_FLOAT_EQ(temperaturesOfSecondPoint[t], temperatureArray->GetTuple1(1));
	}

	// Test an intermediate value
	filter->GetOutputInformation(0)->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), 0.5);
	filter->Update();

	// Run the filter on the input data
	vtkSmartPointer<vtkPolyData> outputDataSet = vtkSmartPointer<vtkPolyData>::New();
	outputDataSet->ShallowCopy(filter->GetPolyDataOutput());

	ASSERT_TRUE(outputDataSet);

	// Extract the filter's output
	vtkSmartPointer<vtkFloatArray> temperatureArray = vtkFloatArray::SafeDownCast(
	            outputDataSet->GetPointData()->GetArray("temperatures"));
	ASSERT_TRUE(temperatureArray);

	EXPECT_FLOAT_EQ(22.14, temperatureArray->GetTuple1(0));
	EXPECT_FLOAT_EQ(18.8, temperatureArray->GetTuple1(1));
}

TEST(TestTemporalInterpolationFilter, TestWindData) {
	// Read some test data
	vtkSmartPointer<vtkKronosReader> kronosReader = vtkSmartPointer<vtkKronosReader>::New();
	kronosReader->SetFileName("res/test-data/temporal-interpolation-test/wind-test-data.kJson", false);

	// Set up the filter and its input
	vtkSmartPointer<TemporalInterpolationFilter> filter = TemporalInterpolationFilter::New();
	filter->SetInputConnection(kronosReader->GetOutputPort());
	filter->GetInputInformation()->Set(Data::VTK_DATA_TYPE(), Data::TEMPERATURE);
	filter->GetInputInformation()->Set(Data::VTK_TIME_RESOLUTION(), 1);

	// Test integral values
	QList<float> windSpeedsOfFirstPoint = QList<float>() << 0.1 << 1.57 << 0.06 << 1.59;
	QList<int> windBearingsOfFirstPoint = QList<int>() << 3 << 123 << 207 << 282;

	QList<float> windSpeedsOfSecondPoint = QList<float>() << 0.83 << 0.83 << 0.83 << 0.83;
	QList<int> windBearingsOfSecondPoint = QList<int>() << 323 << 323 << 323 << 323;

	for (int t = 0; t < windSpeedsOfFirstPoint.size() - 1; t++) {
		filter->GetOutputInformation(0)->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), t);
		filter->Update();

		// Run the filter on the input data
		vtkSmartPointer<vtkPolyData> outputDataSet = vtkSmartPointer<vtkPolyData>::New();
		outputDataSet->ShallowCopy(filter->GetPolyDataOutput());

		ASSERT_TRUE(outputDataSet);

		// Extract the filter's output
		vtkSmartPointer<vtkFloatArray> windSpeedArray = vtkFloatArray::SafeDownCast(
		            outputDataSet->GetPointData()->GetArray("speeds"));
		ASSERT_TRUE(windSpeedArray);

		vtkSmartPointer<vtkFloatArray> windBearingArray = vtkFloatArray::SafeDownCast(
		            outputDataSet->GetPointData()->GetArray("directions"));
		ASSERT_TRUE(windBearingArray);

		EXPECT_FLOAT_EQ(windSpeedsOfFirstPoint[t], windSpeedArray->GetTuple1(0));
		EXPECT_FLOAT_EQ(windBearingsOfFirstPoint[t], windBearingArray->GetTuple1(0));

		EXPECT_FLOAT_EQ(windSpeedsOfSecondPoint[t], windSpeedArray->GetTuple1(1));
		EXPECT_FLOAT_EQ(windBearingsOfSecondPoint[t], windBearingArray->GetTuple1(1));
	}

	// Test an intermediate value
	filter->GetOutputInformation(0)->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), 0.5);
	filter->Update();

	// Run the filter on the input data
	vtkSmartPointer<vtkPolyData> outputDataSet = vtkSmartPointer<vtkPolyData>::New();
	outputDataSet->ShallowCopy(filter->GetPolyDataOutput());

	ASSERT_TRUE(outputDataSet);

	// Extract the filter's output
	vtkSmartPointer<vtkFloatArray> windSpeedArray = vtkFloatArray::SafeDownCast(
	            outputDataSet->GetPointData()->GetArray("speeds"));
	ASSERT_TRUE(windSpeedArray);

	vtkSmartPointer<vtkFloatArray> windBearingArray = vtkFloatArray::SafeDownCast(
	            outputDataSet->GetPointData()->GetArray("directions"));
	ASSERT_TRUE(windBearingArray);
}

TEST(TestTemporalInterpolationFilter, TestCloudCoverageData) {
	// Read some test data
	vtkSmartPointer<vtkKronosReader> kronosReader = vtkSmartPointer<vtkKronosReader>::New();
	kronosReader->SetFileName("res/test-data/temporal-interpolation-test/cloud-coverage-test-data.kJson",
	                          false);

	// Set up the filter and its input
	vtkSmartPointer<TemporalInterpolationFilter> filter = TemporalInterpolationFilter::New();
	filter->SetInputConnection(kronosReader->GetOutputPort());
	filter->GetInputInformation()->Set(Data::VTK_DATA_TYPE(), Data::TEMPERATURE);
	filter->GetInputInformation()->Set(Data::VTK_TIME_RESOLUTION(), 1);

	// Test integral values
	QList<float> coverageValuesOfFirstPoint = QList<float>() << 0.43 << 0.86 << 0.89 << 0.35;
	QList<float> coverageValuesOfSecondPoint = QList<float>() << 0.63 << 0.63 << 0.63 << 0.63;

	for (int t = 0; t < coverageValuesOfFirstPoint.size() - 1; t++) {
		filter->GetOutputInformation(0)->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), t);
		filter->Update();

		// Run the filter on the input data
		vtkSmartPointer<vtkPolyData> outputDataSet = vtkSmartPointer<vtkPolyData>::New();
		outputDataSet->ShallowCopy(filter->GetPolyDataOutput());

		ASSERT_TRUE(outputDataSet);

		// Extract the filter's output
		vtkSmartPointer<vtkFloatArray> cloudCoverageArray = vtkFloatArray::SafeDownCast(
		            outputDataSet->GetPointData()->GetArray("cloudCovers"));
		ASSERT_TRUE(cloudCoverageArray);

		EXPECT_FLOAT_EQ(coverageValuesOfFirstPoint[t], cloudCoverageArray->GetTuple1(0));
		EXPECT_FLOAT_EQ(coverageValuesOfSecondPoint[t], cloudCoverageArray->GetTuple1(1));
	}

	// Test an intermediate value
	filter->GetOutputInformation(0)->Set(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), 0.5);
	filter->Update();

	// Run the filter on the input data
	vtkSmartPointer<vtkPolyData> outputDataSet = vtkSmartPointer<vtkPolyData>::New();
	outputDataSet->ShallowCopy(filter->GetPolyDataOutput());

	ASSERT_TRUE(outputDataSet);

	// Extract the filter's output
	vtkSmartPointer<vtkFloatArray> cloudCoverageArray = vtkFloatArray::SafeDownCast(
	            outputDataSet->GetPointData()->GetArray("cloudCovers"));
	ASSERT_TRUE(cloudCoverageArray);

	EXPECT_FLOAT_EQ(0.645, cloudCoverageArray->GetTuple1(0));
	EXPECT_FLOAT_EQ(0.63, cloudCoverageArray->GetTuple1(1));
}

TEST(TestTemporalInterpolationFilter, TestTwitterDensityData) {
	// Read some test data
	vtkSmartPointer<vtkKronosReader> kronosReader = vtkSmartPointer<vtkKronosReader>::New();
	kronosReader->SetFileName("res/test-data/temporal-interpolation-test/twitter-test-data.kJson",
	                          false);

	// Set up the density heatmap filter and its input
	vtkSmartPointer<HeatmapDensityFilter> densityFilter = HeatmapDensityFilter::New();
	densityFilter->SetInputConnection(kronosReader->GetOutputPort());
	densityFilter->GetInputInformation()->Set(Data::VTK_DATA_TYPE(), Data::TWEETS);
	densityFilter->GetInputInformation()->Set(Data::VTK_TIME_RESOLUTION(), 1);
	densityFilter->setHorizontalHeatmapResolution(1);
	densityFilter->setVerticalHeatmapResolution(1);
	densityFilter->Update();

	// Set up the temporal interpolation filter and its input
	vtkSmartPointer<TemporalInterpolationFilter> interpolationFilter =
	    TemporalInterpolationFilter::New();
	interpolationFilter->SetInputConnection(densityFilter->GetOutputPort());

	// Test an intermediate value
	interpolationFilter->GetOutputInformation(0)->Set(
	    vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP(), 0.5);
	interpolationFilter->Update();

	// Run the filter on the input data
	vtkSmartPointer<vtkPolyData> outputDataSet = vtkSmartPointer<vtkPolyData>::New();
	outputDataSet->ShallowCopy(interpolationFilter->GetPolyDataOutput());

	ASSERT_TRUE(outputDataSet);

	// Extract the filter's output
	vtkSmartPointer<vtkFloatArray> densityArray = vtkFloatArray::SafeDownCast(
	            outputDataSet->GetPointData()->GetArray("density"));
	ASSERT_TRUE(densityArray);

	EXPECT_FLOAT_EQ(3.5, densityArray->GetTuple1(0));
	EXPECT_FLOAT_EQ(3, densityArray->GetTuple1(1));
	EXPECT_FLOAT_EQ(3.5, densityArray->GetTuple1(2));
	EXPECT_FLOAT_EQ(3, densityArray->GetTuple1(3));
}
