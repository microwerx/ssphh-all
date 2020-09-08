#include "test-resample.hpp"
#include <fluxions_gte.hpp>
#include <fluxions_gte_image_operations.hpp>
#include <hatchetfish_log.hpp>

namespace Fluxions {
	template <typename T>
	void ResampleRectToCubeMap(TImage<T>& src, TImage<T>& dst, int size, Matrix3f R) {
		dst.resize(size, size, 6);

		const int srcw = src.width();
		const int srch = src.height();
		Vector3f v;
		float inverse = 1.0f / (size - 1.0f);
		for (int face = 0; face < 6; face++) {
			for (int dy = 0; dy < size; dy++) {
				float t = dy * inverse;
				for (int dx = 0; dx < size; dx++) {
					float s = dx * inverse;
					MakeCubeVectorFromFaceST(face, s, t, &v.x, &v.y, &v.z);
					v.normalize();
					// get longitude between 0 and 2PI
					float lon = -std::atan2(-v.z, -v.x) - FX_F32_PI_2;
					while (lon < -FX_F32_PI) lon += FX_F32_TWOPI;
					Vector3f p(v.x, 0.0f, v.z);
					p.normalize();
					float lat = std::acos(p.dot(v));
					if (v.y < 0) lat = -lat;
					// map -PI to PI to 0 to (width - 1)
					// map -PI/2 to PI to 0 to (height - 1)
					int ix = (int)(srcw * (lon * FX_F32_1_2PI + 0.5f));
					int iy = (int)(srch * (lat * FX_F32_1_PI + 0.5f));

					dst.setPixel(dx, dy, face, src.getPixel(ix, iy));
				}
			}
		}
	}
}

void TestResample() {
	using namespace Fluxions;
	Fluxions::Image3f src;
	if (!src.loadPFM("../ssphh/resources/textures/mwpan2_RGB_3600.pfm")) {
		//if (!src.loadPPM("../ssphh/resources/textures/test_environment.ppm")) {
		HFLOGERROR("no src file!");
		exit(0);
	}
	Fluxions::Image3f dst;
	HFLOGINFO("resampling");
	Fluxions::ResampleRectToCubeMap(src, dst, 1024, Matrix3f());
	dst.convertCubeMapToCross(true);
	dst.savePFM("../ssphh/resources/textures/mwpan2_cubemap.pfm");
	//dst.saveCubePPM("../ssphh/resources/textures/test_environment_cube.ppm");
	HFLOGINFO("done resampling");
}
