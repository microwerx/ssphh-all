// ssphh-tests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <type_traits>
#include <iterator>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <fluxions_gte.hpp>
#include <fluxions_gte_catmull_rom.hpp>
#include "test-resample.hpp"
#include <hatchetfish_log.hpp>

using namespace std;

#pragma comment(lib, "fluxions.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "xinput.lib")

void printControlPoints(ostream& fout,
						const std::vector<Fluxions::Vector3f>& points,
						const std::string& style) {
	for (auto& p : points) {
		fout << "<circle ";
		fout << "cx=\"" << (p.x * 100) << "\" ";
		fout << "cy=\"" << (p.y * 100) << "\" ";
		fout << "r=\"5\" ";
		fout << "style=\"" << style << "\" />\n";
	}
}

void printControlPoints(ostream& fout,
						const std::vector<Fluxions::Vector3f>& points,
						const std::vector<float>& alpha,
						const std::string& style) {
	for (unsigned i = 0; i < points.size(); i++) {

		fout << "<circle ";
		fout << "cx=\"" << (points[i].x * 100) << "\" ";
		fout << "cy=\"" << (points[i].y * 100) << "\" ";
		fout << "r=\"" << (alpha[i] + 2) << "\" ";
		fout << "style=\"" << style << "\" />\n";
	}
}

void printPolyline(ostream& fout,
				   const std::vector<Fluxions::Vector3f>& curve,
				   const std::string& style) {
	fout << "<polyline points=\"";
	for (auto& p : curve) {
		fout << (p.x * 100) << "," << (p.y * 100) << "\n";
	}
	fout << "\" style=\"" << style << "\" />\n";
}

void printLine(ostream& fout,
			   float x1, float y1, float x2, float y2,
			   const std::string& style) {
	fout << "<line ";
	fout << "x1=\"" << x1 * 100 << "\" ";
	fout << "y1=\"" << y1 * 100 << "\" ";
	fout << "x2=\"" << x2 * 100 << "\" ";
	fout << "y2=\"" << y2 * 100 << "\" ";
	fout << "style=\"" << style << "\" />\n";
}

void printQuaternion(ostream& fout,
					 const Fluxions::Vector3f& p,
					 const Fluxions::Quaternionf& q,
					 const std::string& style) {
	Fluxions::Matrix3f M = q.toMatrix3();
	float x1 = p.x;
	float y1 = p.y;
	float x2 = x1 + M.m11 * 0.25f;
	float y2 = y1 + M.m21 * 0.25f;
	printLine(fout, x1, y1, x2, y2, style);
}

void printQuaternions(ostream& fout,
					  const std::vector<Fluxions::Vector3f>& controlPoints,
					  const std::vector<Fluxions::Quaternionf>& controlQuaternions,
					  const std::string& style) {
	//const std::string style{ "stroke: black; stroke-width: 1; marker-end: url(#arrow);" };
	if (controlPoints.size() != controlQuaternions.size()) return;
	for (unsigned i = 0; i < controlPoints.size(); i++) {
		printQuaternion(fout, controlPoints[i], controlQuaternions[i], style);
	}
}

class CameraAnimation {
public:
	static constexpr unsigned w = 4;
	static constexpr unsigned h = 4;
	static constexpr unsigned numControlPoints = w * h;
	static constexpr unsigned numPoints = 6;

	using Quaternionf = Fluxions::Quaternionf;
	using Vector3f = Fluxions::Vector3f;

	std::vector<Quaternionf> controlQuaternions;
	std::vector<Vector3f> controlPoints;
	std::vector<float> controlAlpha;
	std::vector<float> controlTime;
	std::vector<Quaternionf> curveQuaternions;

	void create() {
		using Fluxions::randomSampler;
		for (int i = 0; i < numControlPoints; i++) {
			float x = (float)(i % w);
			float y = (float)(i / w);
			float s = 0.25f;
			Vector3f p(1 + x + 0.5f, 1 + y + 0.5f, 0.0f);
			controlPoints.push_back(p + Vector3f(randomSampler(-s, s),
												 randomSampler(-s, s),
												 randomSampler(-s, s)));
			controlAlpha.push_back(1.0f);
			controlQuaternions.push_back(Quaternionf::makeFromAngleAxis(randomSampler(0.0, 360.0), 0, 0, 1));
		}
	}

	Vector3f pcatmullrom(float t) const {
		return CatmullRomSplinePoint(t, controlPoints, controlAlpha);
	}

	Quaternionf q(float t) const {
		int q1 = int(t) % numControlPoints;
		int q2 = (q1 + 1) % numControlPoints;
		t = t - int(t);
		return Fluxions::slerp(controlQuaternions[q1], controlQuaternions[q2], t);
	}

	Fluxions::Quaternionf qsquad(float t) const {
		int iq1 = int(t) % numControlPoints;
		int iq0 = (iq1 - 1) % numControlPoints;
		int iq2 = (iq1 + 1) % numControlPoints;
		int iq3 = (iq1 + 2) % numControlPoints;
		t = t - int(t);
		Fluxions::Quaternionf q0 = controlQuaternions[iq0];
		Fluxions::Quaternionf q1 = controlQuaternions[iq1];
		Fluxions::Quaternionf q2 = controlQuaternions[iq2];
		Fluxions::Quaternionf q3 = controlQuaternions[iq3];
		return Fluxions::squad(q0, q1, q2, q3, t);
	}
};

void TestCatmullRom() {
	constexpr unsigned w = 4;
	constexpr unsigned h = 4;
	constexpr unsigned numControlPoints = w * h;
	constexpr unsigned numPoints = 6;

	using namespace Fluxions;

	std::vector<Quaternionf> controlQuaternions;
	std::vector<Vector3f> controlPoints;
	std::vector<float> controlAlpha;
	std::vector<float> curveTime;
	std::vector<Quaternionf> curveQuaternions;

	for (int i = 0; i < numControlPoints; i++) {
		float x = (float)(i % w);
		float y = (float)(i / w);
		float s = 0.25f;
		Vector3f p(1 + x + 0.5f, 1 + y + 0.5f, 0.0f);
		controlPoints.push_back(p + Vector3f(randomSampler(-s, s),
											 randomSampler(-s, s),
											 randomSampler(-s, s)));
		//controlPoints.push_back(Vector3f(randomSampler(0, w),
		//								 randomSampler(0, h),
		//								 randomSampler(0, 1)));
		controlAlpha.push_back(1.0f);
		controlQuaternions.push_back(Quaternionf::makeFromAngleAxis(randomSampler(0.0, 360.0), 0, 0, 1));
	}
	int first = 0;
	int last = numControlPoints * numPoints;
	for (int j = first; j < last + 1; j++) {
		curveTime.push_back((float)j / (float)numPoints);
	}

	curveQuaternions.resize(curveTime.size());
	const unsigned ccount = (unsigned)controlQuaternions.size();
	const unsigned pcount = (unsigned)curveQuaternions.size();
	for (int j = 0; j < curveTime.size(); j++) {
		int q1 = int(curveTime[j % pcount]) % ccount;
		int q0 = (q1 - 1) % ccount;
		int q2 = (q1 + 1) % ccount;
		int q3 = (q1 + 2) % ccount;
		float t = curveTime[j] - int(curveTime[j]);
		//curveQuaternions[j] = Fluxions::slerp(controlQuaternions[q1], controlQuaternions[q2], t);
		curveQuaternions[j] = Fluxions::squad(controlQuaternions[q0],
											  controlQuaternions[q1],
											  controlQuaternions[q2],
											  controlQuaternions[q3],
											  t);
	}

	std::vector<Vector3f> curve;
	ofstream fout("curve.svg");
	fout << "<svg xmlns=\"http://www.w3.org/2000/svg\" ";
	fout << "width=\"" << (w + 2) * 100 << "\" ";
	fout << "height=\"" << (h + 2) * 100 << "\">" << endl;

	fout << "<defs>\n";
	fout << "<marker id=\"arrow\" markerWidth=\"10\" markerHeight=\"10\" refX=\"0\" refY=\"3\" orient=\"auto\" markerUnits=\"strokeWidth\">\n";
	fout << "<path d=\"M0,0 L0,6 L9,3 z\" fill=\"#f00\" />\n";
	fout << "</marker>\n";
	fout << "</defs>\n";

	printControlPoints(fout, controlPoints, controlAlpha,
					   "fill:red; stroke: black; stroke-width: 2;");

	CatmullRomClosedSpline(controlPoints, curve, numPoints, 1.0f);
	printPolyline(fout, curve, "fill: none; stroke: green; stroke-width: 1;");

	CatmullRomClosedSpline(controlPoints, curve, numPoints, -1.0f);
	printPolyline(fout, curve, "fill: none; stroke: red; stroke-width: 1;");

	CatmullRomSplineUniform(controlPoints, controlAlpha, curveTime, curve);
	printControlPoints(fout, curve,
					   "fill:green; stroke:black; stroke-width: 2;");
	printPolyline(fout, curve, "fill: none; stroke: blue; stroke-width: 1;");

	printQuaternions(fout, controlPoints, controlQuaternions, "stroke: blue; stroke-width: 2; marker-end: url(#arrow);");
	printQuaternions(fout, curve, curveQuaternions, "stroke: black; stroke-width: 1; marker-end: url(#arrow);");

	fout << "</svg>" << endl;
	fout.close();
}

using Fluxions::Quaternionf;
using std::ostream;
using std::endl;

ostream& operator<<(ostream& os, Quaternionf q) {
	os << q.a << ", " << q.b << ", " << q.c << ", " << q.d;
	return os;
}

namespace Fluxions
{

}

void TestQuaternions() {
	Quaternionf q(1, 2, 3, 4);
	cout << setprecision(6);
	cout << setfill(' ');
	cout << fixed << showpos;
	cout << "q            " << q << endl;
	cout << "q'           " << q.normalized() << endl;
	cout << "log(q)       " << q.log() << endl;
	cout << "exp(q)       " << q.exp() << endl;
	cout << "exp(log(q)): " << q.log().exp() << endl;
	cout << "q^-1         " << q.inverse() << endl;
	cout << "q*           " << q.conjugate() << endl;
	cout << "q^p          " << q.pow(-1.0f) << endl;

	Quaternionf q0 = Quaternionf::makeFromAngleAxis(15.0, 0, 0, 1);
	Quaternionf q1 = Quaternionf::makeFromAngleAxis(35.0, 1, 0, 0);
	Quaternionf q2 = Quaternionf::makeFromAngleAxis(75.0, 0, 1, 0);
	Quaternionf q3 = Quaternionf::makeFromAngleAxis(95.0, 0, 0, 1);
	Quaternionf a = Fluxions::squad_si(q0, q1, q2);
	Quaternionf b = Fluxions::squad_si(q1, q2, q3);
	cout << "q0           " << q0 << endl;
	cout << "q1           " << q1 << endl;
	cout << "a            " << a << endl;
	cout << "b            " << b << endl;
	cout << "q2           " << q2 << endl;
	cout << "q3           " << q3 << endl;

	//Quaternion kQ0inv = rkQ0.UnitInverse();
	//Quaternion kQ1inv = rkQ1.UnitInverse();
	//Quaternion rkP0 = kQ0inv * rkQ1;
	//Quaternion rkP1 = kQ1inv * rkQ2;
	//Quaternion kArg = 0.25 * (rkP0.Log() - rkP1.Log());
	//Quaternion kMinusArg = -kArg;

	//rkA = rkQ1 * kArg.Exp();
	//rkB = rkQ1 * kMinusArg.Exp();

	Quaternionf q0inv = q0.inverse();
	Quaternionf q1inv = q1.inverse();
	Quaternionf p0 = q0inv * q1;
	Quaternionf p1 = q1inv * q2;
	Quaternionf arg = (p0.log() - p1.log()).scale(0.25f);
	Quaternionf narg = -arg;
	a = q1 * arg.exp();
	b = q1 * narg.exp();
	a = Fluxions::squad_a(q0, q1, q2);
	b = Fluxions::squad_b(q0, q1, q2);
	cout << "a            " << a << endl;
	cout << "b            " << b << endl;

	float y = Fluxions::randomSampler(-180, 180);
	float p = Fluxions::randomSampler(-90, 90);
	float r = Fluxions::randomSampler(-180, 180);
	q0 = Quaternionf::makeFromAngles(y, p, r);
	cout << "y,p,r        " << y << ", " << p << ", " << r << endl;
	cout << "q            " << q0 << endl;
	y = q0.yawInDegrees();
	p = q0.pitchInDegrees();
	r = q0.rollInDegrees();
	q0 = Quaternionf::makeFromAngles(y, p, r);
	cout << "y,p,r        " << y << ", " << p << ", " << r << endl;
	y = q0.yawInDegrees();
	p = q0.pitchInDegrees();
	r = q0.rollInDegrees();
	q0 = Quaternionf::makeFromAngles(y, p, r);
	cout << "y,p,r        " << y << ", " << p << ", " << r << endl;
	y = q0.yawInDegrees();
	p = q0.pitchInDegrees();
	r = q0.rollInDegrees();
	q0 = Quaternionf::makeFromAngles(y, p, r);
	cout << "y,p,r        " << y << ", " << p << ", " << r << endl;
}


void TestQuaternionToAngles(Quaternionf q, float& phi, float& theta, float& psi) {
	phi = FX_F32_RADIANS_TO_DEGREES * atan2f(2.0f * (q.a * q.b + q.c * q.d), 1.0f - 2.0f * (q.b * q.b + q.c * q.c));
	theta = FX_F32_RADIANS_TO_DEGREES * asinf(2.0f * (q.a * q.c - q.d * q.b));
	psi = FX_F32_RADIANS_TO_DEGREES * atan2f(2.0f * (q.a * q.d + q.b * q.c), 1.0f - 2.0f * (q.c * q.c + q.d * q.d));

	HFLOGINFO("phi: % 3.4f  theta: % 3.4f psi: % 3.4f", phi, theta, psi);
}


Quaternionf TestAnglesToQuaternion(float phi, float theta, float psi) {
	Quaternionf q = Quaternionf::makeFromAngles(phi, theta, psi);
	float cos_phi = cosf(phi * 0.5f * FX_F32_DEGREES_TO_RADIANS);
	float cos_theta = cosf(theta * 0.5f * FX_F32_DEGREES_TO_RADIANS);
	float cos_psi = cosf(psi * 0.5f * FX_F32_DEGREES_TO_RADIANS);
	float sin_phi = sinf(phi * 0.5f * FX_F32_DEGREES_TO_RADIANS);
	float sin_theta = sinf(theta * 0.5f * FX_F32_DEGREES_TO_RADIANS);
	float sin_psi = sinf(psi * 0.5f * FX_F32_DEGREES_TO_RADIANS);
	q.a = cos_phi * cos_theta * cos_psi + sin_phi * sin_theta * sin_psi;
	q.b = sin_phi * cos_theta * cos_psi - cos_phi * sin_theta * sin_psi;
	q.c = cos_phi * sin_theta * cos_psi + sin_phi * cos_theta * sin_psi;
	q.d = cos_phi * cos_theta * sin_psi - sin_phi * sin_theta * cos_psi;
	HFLOGINFO("    q: % 3.4f % 3.4f % 3.4f % 3.4f ==> % 3.4f", q.a, q.b, q.c, q.d, q.length());
	return q;
}


void TestQuaternionsBook() {
	float angle = -45.0f / 2.0f * FX_F32_DEGREES_TO_RADIANS;
	for (float angle : {-720.0f, -400.0f, -360.0f, -270.0f, -180.0f, -90.0f, -45.0f, -15.0f, 0.0f }) {
		HFLOGINFO("\nAngle = %3.1f degrees", angle);
		angle = angle / 2.0f * FX_F32_DEGREES_TO_RADIANS;
		Quaternionf q1{ cosf(angle), sinf(angle), 0.0f, 0.0f };
		Quaternionf q2{ cosf(angle), 0.0f, sinf(angle), 0.0f };
		Quaternionf q3{ cosf(angle), 0.0f, 0.0f, sinf(angle) };
		float phi{};
		float theta{};
		float psi{};
		HFLOGINFO("    q: % 3.4f % 3.4f % 3.4f % 3.4f", q1.a, q1.b, q1.c, q1.d);
		TestQuaternionToAngles(q1, phi, theta, psi);
		TestAnglesToQuaternion(phi, theta, psi);
		TestQuaternionToAngles(q2, phi, theta, psi);
		TestAnglesToQuaternion(phi, theta, psi);
		TestQuaternionToAngles(q3, phi, theta, psi);
		TestAnglesToQuaternion(phi, theta, psi);
	}

	for (int i : {0, 1, 2, 3, 4, 5, 6}) {
		float phi = Fluxions::randomSampler(-180, 180); // yaw
		float theta = Fluxions::randomSampler(-90, 90); // pitch
		float psi = Fluxions::randomSampler(-180, 180); // roll
		HFLOGINFO("begin test %d", i);
		HFLOGINFO("   phi: % 3.4f  theta: % 3.4f psi: % 3.4f", phi, theta, psi);
		Quaternionf q = TestAnglesToQuaternion(phi, theta, psi);
		TestQuaternionToAngles(q, phi, theta, psi);
		q = TestAnglesToQuaternion(phi, theta, psi);
		HFLOGINFO("finish test %d", i);
	}
}


int main(int argc, char** argv) {
	//TestResample();
	TestQuaternionsBook();
	return 0;

	auto float_category = std::iterator_traits<float*>::iterator_category();

	//cout << _Is_random_iter_v<Fluxions::Vector3f>;
	cout << "TCommonIterator<float>"
		<< "----------------------" << endl;
	cout << "input_iterator_tag:         " << (bool)std::is_convertible_v<Fluxions::TCommonIterator<float>::iterator_category, input_iterator_tag> << endl;
	cout << "output_iterator_tag:        " << (bool)std::is_convertible_v<Fluxions::TCommonIterator<float>::iterator_category, output_iterator_tag> << endl;
	cout << "forward_iterator_tag:       " << (bool)std::is_convertible_v<Fluxions::TCommonIterator<float>::iterator_category, forward_iterator_tag> << endl;
	cout << "bidirectional_iterator_tag: " << (bool)std::is_convertible_v<Fluxions::TCommonIterator<float>::iterator_category, bidirectional_iterator_tag> << endl;
	cout << "random_access_iterator_tag: " << (bool)std::is_convertible_v<Fluxions::TCommonIterator<float>::iterator_category, random_access_iterator_tag> << endl;
	//cout << "contiguous_iterator_tag:    " << (bool)std::is_convertible_v<Fluxions::TCommonIterator<float>, contiguous_iterator_tag> << endl;
	cout << "input_iterator_tag:         " << (bool)std::is_convertible_v<std::iterator_traits<float*>::iterator_category, input_iterator_tag> << endl;
	cout << "output_iterator_tag:        " << (bool)std::is_convertible_v<std::iterator_traits<float*>::iterator_category, output_iterator_tag> << endl;
	cout << "forward_iterator_tag:       " << (bool)std::is_convertible_v<std::iterator_traits<float*>::iterator_category, forward_iterator_tag> << endl;
	cout << "bidirectional_iterator_tag: " << (bool)std::is_convertible_v<std::iterator_traits<float*>::iterator_category, bidirectional_iterator_tag> << endl;
	cout << "random_access_iterator_tag: " << (bool)std::is_convertible_v<std::iterator_traits<float*>::iterator_category, random_access_iterator_tag> << endl;
	//cout << "contiguous_iterator_tag:    " << (bool)std::is_convertible_v<Fluxions::TCommonIterator<float>, contiguous_iterator_tag> << endl;
	cout << "---------------------------------------" << endl;
	Fluxions::TestFluxionsGTE();
	TestCatmullRom();
	return 0;
}
