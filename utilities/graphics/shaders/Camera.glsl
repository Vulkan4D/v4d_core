layout(set = 0, binding = 0) uniform Camera {
	int width;
	int height;
	bool txaa;
	bool debug;
	vec4 luminance;
	dvec3 worldPosition;
	double fov;
	dvec3 lookDirection;
	double znear;
	dvec3 viewUp;
	double zfar;
	dmat4 viewMatrix;
	dmat4 projectionMatrix;
	dmat4 historyViewMatrix;
	mat4 reprojectionMatrix;
	vec2 txaaOffset;
	vec2 historyTxaaOffset;
} camera;

double GetTrueDistanceFromDepthBuffer(double depth) {
	return 2.0 * (camera.zfar * camera.znear) / (camera.znear + camera.zfar - (depth * 2.0 - 1.0) * (camera.znear - camera.zfar));
}
