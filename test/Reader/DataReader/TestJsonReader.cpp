#include <gtest/gtest.h>

#include <string>

#include <Reader/DataReader/JsonReaderFactory.hpp>
#include <Reader/DataReader/JsonReader.hpp>
#include <Reader/DataReader/DataType.hpp>
#include <Reader/DataReader/DataPoints/NonTemporalDataPoints/CityDataPoint.hpp>

#include <vtkSmartPointer.h>

TEST(TestJsonReader, ReadCityData) {
	JsonReader cityReader = JsonReaderFactory::createReader("res/test-data/cities.json");
	EXPECT_EQ(
		cityReader.pointDataSet.getDataPoints().size(),
		4
	);
	
	const CityDataPoint& firstDataPoint = static_cast<const CityDataPoint&>(
		cityReader.pointDataSet.getDataPoints().at(1)
	);
	
	
	EXPECT_EQ(
		firstDataPoint.getName().toStdString(),
		"Los Angeles"
	);
}

/*TEST(TestJsonReader, TestShitty) {
    JsonReader jsonReader = JsonReaderFactory::createReader("res/test-data/cities.json");
    vtkSmartPointer<vtkPolyData> polyData = jsonReader.getVtkDataSet(9);
    EXPECT_EQ(false, true);
}*/