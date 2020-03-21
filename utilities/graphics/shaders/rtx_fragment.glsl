struct Fragment {
	uint indexOffset;
	uint vertexOffset;
	uint objectIndex;
	uint material;
	ObjectInstance objectInstance;
	Vertex v0;
	Vertex v1;
	Vertex v2;
	vec3 barycentricCoords;
	vec3 hitPoint;
	// Interpolated values
	vec3 pos;
	vec3 normal;
	vec2 uv;
	vec4 color;
};

Fragment GetHitFragment(bool interpolateVertexData) {
	Fragment f;
	
	f.indexOffset = geometries[gl_InstanceCustomIndexEXT].x;
	f.vertexOffset = geometries[gl_InstanceCustomIndexEXT].y;
	f.objectIndex = geometries[gl_InstanceCustomIndexEXT].z;
	f.material = geometries[gl_InstanceCustomIndexEXT].w;
	
	f.objectInstance = GetObjectInstance(f.objectIndex);
	
	f.v0 = GetVertex(indices[f.indexOffset + (3 * gl_PrimitiveID) + 0] + f.vertexOffset);
	f.v1 = GetVertex(indices[f.indexOffset + (3 * gl_PrimitiveID) + 1] + f.vertexOffset);
	f.v2 = GetVertex(indices[f.indexOffset + (3 * gl_PrimitiveID) + 2] + f.vertexOffset);
	
	f.barycentricCoords = vec3(1.0f - hitAttribs.x - hitAttribs.y, hitAttribs.x, hitAttribs.y);
	f.hitPoint = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
	
	if (interpolateVertexData) {
		f.pos = (f.v0.pos * f.barycentricCoords.x + f.v1.pos * f.barycentricCoords.y + f.v2.pos * f.barycentricCoords.z);
		f.normal = normalize(f.v0.normal * f.barycentricCoords.x + f.v1.normal * f.barycentricCoords.y + f.v2.normal * f.barycentricCoords.z);
		// f.normal = normalize(transpose(inverse(mat3(f.objectInstance.modelViewMatrix))) * f.normal);
		f.normal = normalize(f.objectInstance.normalMatrix * f.normal);
		f.uv = (f.v0.uv * f.barycentricCoords.x + f.v1.uv * f.barycentricCoords.y + f.v2.uv * f.barycentricCoords.z);
		f.color = (f.v0.color * f.barycentricCoords.x + f.v1.color * f.barycentricCoords.y + f.v2.color * f.barycentricCoords.z);
	}
	
	return f;
}
