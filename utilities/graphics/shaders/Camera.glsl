layout(set = 0, binding = 0) uniform Camera {
	int width;
	int height;
	dvec3 worldPosition;
	double fov;
	dvec3 lookDirection;
	double znear;
	dvec3 viewUp;
	double zfar;
	dmat4 viewMatrix;
	dmat4 projectionMatrix;
} camera;
