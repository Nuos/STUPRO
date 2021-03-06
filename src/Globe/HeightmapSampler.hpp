#ifndef SRC_GLOBE_HEIGHTMAPSAMPLER_HPP_
#define SRC_GLOBE_HEIGHTMAPSAMPLER_HPP_

#include <vector>

/**
 * Singleton class that provides fixed-resolution access to earth's heightmap data.
 */
class HeightmapSampler {
public:

	/**
	 * Returns the singleton instance of the heightmap sampler, initializing it if necessary.
	 *
	 * This function blocks until the heightmap data has been loaded.
	 */
	static HeightmapSampler& getInstance();

	/**
	 * Returns the height (in meters) at the specified location, multiplied with the globe's height
	 * factor. Optionally, a constant offset (in meters) can be applied to the output value.
	 */
	float sample(float longitude, float latitude, float offset = 0.f) const;

	/**
	 * Returns the height (in meters) at the specified location, multiplied with the globe's height
	 * factor. Optionally, a constant offset (in meters) can be applied to the output value.
	 *
	 * This function guarantees a return value of zero or greater.
	 */
	float sampleClamped(float longitude, float latitude, float offset = 0.f) const;

private:

	HeightmapSampler();

	void initHeightmap();

	struct Heightmap {
		unsigned int width;
		unsigned int height;
		std::vector<short> samples;
	};

	std::vector<Heightmap> heightmaps;
	unsigned int zoomLevel;
	float heightFactor;
};

#endif /* SRC_GLOBE_HEIGHTMAPSAMPLER_HPP_ */
