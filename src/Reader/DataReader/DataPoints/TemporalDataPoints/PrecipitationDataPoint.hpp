#ifndef KRONOS_PRECIPITATIONDATAPOINT_HPP
#define KRONOS_PRECIPITATIONDATAPOINT_HPP

#include <Reader/DataReader/DataPoints/TemporalDataPoints/TemporalDataPoint.hpp>

/**
 * Holds a data point for the precipitation rate at a specific location
 */
class PrecipitationDataPoint : public TemporalDataPoint {

public:
	/**
	 * Enum that denotes a precipitation type.
	 * Use caution when changing this enum or the integer values assigned to each entry. These will
	 * be exported to an integer array for the data points to be passed through the ParaView
	 * pipeline. Therefore, changing values may negatively impact the semantics of components that
	 * take these integer values for granted.
	 * Unfortunately, these values need to start with the value one since VTK's selection extraction
	 * seems not to behave well with values of zero.
	 */
	enum PrecipitationType {
		NONE = 1, RAIN = 2, SNOW = 3, SLEET = 4, HAIL = 5
	};

	/**
	 * Create a new PrecipitationDataPoint.
	 * @param coordinate The point's coordinates
	 * @param priority The point's zoom level priority
	 * @param timestamp The point's timestamp
	 * @param precipitationRate The point's precipitation rate
	 * @param precipitationType The point's precipitation type
	 */
	PrecipitationDataPoint(Coordinate coordinate, int priority, int timestamp,
	                       float precipitationRate,
	                       PrecipitationDataPoint::PrecipitationType precipitationType);

	/**
	* Get the precipitation rate of this data point.
	* @return The precipitation rate of this data point
	*/
	float getPrecipitationRate() const;

	/**
	 * Get the precipitation type of this data point.
	 * @return The precipitation type of this data point
	 */
	PrecipitationDataPoint::PrecipitationType getPrecipitationType() const;

private:
	float precipitationRate;
	PrecipitationDataPoint::PrecipitationType precipitationType;
};

#endif