#ifndef KRONOS_GEOGRAPHIC_FUNCTIONS
#define KRONOS_GEOGRAPHIC_FUNCTIONS

#include <Utils/Math/Vector3.hpp>
#include <Utils/Misc/Macros.hpp>
#include <Utils/Config/Configuration.hpp>
#include <Globe/Coordinate.hpp>
#include <Utils/Math/Functions.hpp>

#include <math.h>

/**
 * @brief getGlobeRadius get the radius of the globe
 * @return the globe radius
 */
inline double getGlobeRadius() {
	static double globeRadius = Configuration::getInstance().getDouble("globe.radius");
	return globeRadius;
}

/**
 * Convert the spherical (gps) to the cartesian representation
 * @param gps the gps position to convert
 * @return the cartesian position of gps
 */
template<typename T> Cartesian<T> sphericalToCartesian(const Spherical<T>& gps) {
	//Radian of longitude
	const T lonInRadian = gps.x * KRONOS_PI / 180;
	//Radian of latitude
	const T latInRadian = gps.y * KRONOS_PI / 180;

	Cartesian<T> retVal;
	//WATCH OUT: order of theses assignement, because of ParaView coordinate systems
	retVal.z = (getGlobeRadius() + gps.z) * cos(latInRadian) * cos(lonInRadian);
	retVal.x = (getGlobeRadius() + gps.z) * cos(latInRadian) * sin(lonInRadian);
	retVal.y = (getGlobeRadius() + gps.z) * sin(latInRadian);
	return retVal;
}

template<typename T> void sphericalToCartesianJacobian(const Spherical<T>& gps, T jacobian[3][3]) {
	const T lonInRadian = gps.x * KRONOS_PI / 180;
	const T latInRadian = gps.y * KRONOS_PI / 180;

	//calculate first row of jacobian
	jacobian[0][0] = (getGlobeRadius() + gps.z) * cos(latInRadian) * cos(lonInRadian) * KRONOS_PI / 180;
	jacobian[0][1] = (getGlobeRadius() + gps.z) * sin(lonInRadian) * (-1) * sin(
	                     latInRadian) * KRONOS_PI / 180;
	jacobian[0][2] = 1 * cos(latInRadian) * sin(lonInRadian);

	//calculate second row of jacobian
	jacobian[1][0] = 0;
	jacobian[1][1] = (getGlobeRadius() + gps.z) * cos(latInRadian) * KRONOS_PI / 180;
	jacobian[1][2] = sin(latInRadian);

	//calculate third row of jacobian
	jacobian[2][0] = (getGlobeRadius() + gps.z) * cos(latInRadian) * (-1) * sin(
	                     lonInRadian) * KRONOS_PI / 180;
	jacobian[2][1] = (getGlobeRadius() + gps.z) * (-1) * sin(latInRadian) * KRONOS_PI / 180 * cos(
	                     lonInRadian);
	jacobian[2][2] = 1 * cos(latInRadian) * cos(lonInRadian);

}

/**
 * Convert the spherical (gps) to the cartesian representation, with flat map projection
 * @param gps the gps position to convert
 * @return the cartesian position of gps
 */
template<typename T> Cartesian<T> sphericalToCartesianFlat(const Spherical<T>& gps) {

	Cartesian<T> retVal;

	// Scale to 2D map size
	retVal.x = (gps.x / 90.0) * getGlobeRadius();
	retVal.y = (gps.y / 90.0) * getGlobeRadius();
	retVal.z = gps.z;

	return retVal;
}

template<typename T> void sphericalToCartesianFlatJacobian(const Spherical<T>& gps,
        T jacobian[3][3]) {

	T factor = (1.0 / 90.0) * getGlobeRadius();

	//calculate first row of jacobian
	jacobian[0][0] = factor;
	jacobian[0][1] = 0;
	jacobian[0][2] = 0;

	//calculate second row of jacobian
	jacobian[1][0] = 0;
	jacobian[1][1] = factor;
	jacobian[1][2] = 0;

	//calculate third row of jacobian
	jacobian[2][0] = 0;
	jacobian[2][1] = 0;
	jacobian[2][2] = 1;
}



/**
 * Convert the cartesian to the spherical (gps) representation
 * @param point the cartesian position
 * @return the gps position of point
 */
template<typename T> Spherical<T> cartesianToSpherical(const Cartesian<T>& point) {
	Spherical<T> retVal;
	retVal.z = point.length() - getGlobeRadius();
	retVal.x = atan2(point.x, point.z) * 180 / KRONOS_PI;
	retVal.y = asin(point.y / point.length()) * 180 / KRONOS_PI;
	return retVal;
}

/**
 * Get the (cartesian) abs of a spherical coordinate
 * @param gps the position to get the abs from
 * @return the abs of gps
 */
template<typename T> T abs(const Vector3<T>& gps) {
	return gps.z + getGlobeRadius();
}

/**
 * Get the (cartesian) distance of two given points (spherical)
 * @param point1 the first point
 * @param point1 the second point
 * @return the distance of the two points
 */
template<typename T> float distance(const Spherical<T>& point1, const Spherical<T>& point2) {
	return (sphericalToCartesian(point1) - sphericalToCartesian(point2)).length();
}

/**
 * Scale a spherical coordinate to a given (cartesian) length
 * @param gps the position to scale
 * @return the scaled position
 */
template<typename T> Spherical<T> scaleTo(const Spherical<T>& gps, const T targetLength) {
	return Vector3<T>(gps.x, gps.y, targetLength);
}

/**
 * Get the point right between two other points (in spherical sense, so the result has the middle abs of gps1 and gps2)
 */
template<typename T> Spherical<T> calculateCenter(const Spherical<T>& gps1,
        const Spherical<T>& gps2) {
	Cartesian<T> cartesian1 = sphericalToCartesian(gps1) / 2;
	Cartesian<T> cartesian2 = sphericalToCartesian(gps2) / 2;
	Spherical<T> newPoint = cartesianToSpherical(cartesian1 + cartesian2);
	return scaleTo(newPoint, (gps1.z + gps2.z) / 2);
}

/**
 * Calculate the distance between two coordinates in kilometres using the Haversine formula.
 * @param startPoint The first point
 * @param endPoint The second point
 * @return The distance between the two points in kilometres
 */
double calculateDistance(Coordinate startPoint, Coordinate endPoint);

#endif
