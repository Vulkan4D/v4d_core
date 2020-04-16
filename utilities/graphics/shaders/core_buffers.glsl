// layout(set = 0, binding = 1) readonly buffer ObjectBuffer {dmat4 objectInstances[];};
layout(set = 0, binding = 2) readonly buffer LightBuffer {float lightSources[];};
layout(set = 0, binding = 3) readonly buffer ActiveLights {uint activeLights; uint lightIndices[];};
layout(set = 0, binding = 4) readonly buffer GeometryBuffer {float geometries[];};
layout(set = 0, binding = 5) readonly buffer IndexBuffer {uint indices[];};
layout(set = 0, binding = 6) readonly buffer VertexBuffer {vec4 vertices[];};

struct GeometryInstance {
	uint indexOffset;
	uint vertexOffset;
	uint objectIndex;
	uint material;
	
	mat4 modelTransform;
	mat4 modelViewTransform;
	mat3 normalViewTransform;
	
	vec3 custom3f;
	mat4 custom4x4f;
	
	//
	vec3 viewPosition;
};

GeometryInstance GetGeometryInstance(uint index) {
	GeometryInstance geometry;
	uint i = index*64;
	
	geometry.indexOffset = floatBitsToUint(geometries[i++]);
	geometry.vertexOffset = floatBitsToUint(geometries[i++]);
	geometry.objectIndex = floatBitsToUint(geometries[i++]);
	geometry.material = floatBitsToUint(geometries[i++]);
	
	geometry.modelTransform = mat4(
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++]),
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++]),
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++]),
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++])
	);
	geometry.modelViewTransform = mat4(
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++]),
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++]),
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++]),
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++])
	);
	geometry.normalViewTransform = mat3(
		vec3(geometries[i++], geometries[i++], geometries[i++]),
		vec3(geometries[i++], geometries[i++], geometries[i++]),
		vec3(geometries[i++], geometries[i++], geometries[i++])
	);
	
	geometry.custom3f = vec3(geometries[i++], geometries[i++], geometries[i++]);
	
	geometry.custom4x4f = mat4(
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++]),
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++]),
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++]),
		vec4(geometries[i++], geometries[i++], geometries[i++], geometries[i++])
	);
	
	//
	geometry.viewPosition = geometry.modelViewTransform[3].xyz;
	
	return geometry;
}

struct Vertex {
	vec3 pos;
	vec4 color;
	vec3 normal;
	vec2 uv;
};

struct LightSource {
	vec3 position;
	float intensity;
	vec3 color;
	uint type;
	uint attributes;
	float radius;
	float custom1;
};

LightSource GetLight (uint i) {
	LightSource light;
	light.position = vec3(lightSources[i*8 + 0], lightSources[i*8 + 1], lightSources[i*8 + 2]);
	light.intensity = lightSources[i*8 + 3];
	vec4 c = UnpackColorFromFloat(lightSources[i*8 + 4]);
	light.color = c.rgb;
	light.type = floatBitsToUint(lightSources[i*8 + 4]) & 0x000000ff;
	light.attributes = floatBitsToUint(lightSources[i*8 + 5]);
	light.radius = lightSources[i*8 + 6];
	light.custom1 = lightSources[i*8 + 7];
	return light;
};

#ifdef RAY_TRACING

	Vertex GetVertex(uint index) {
		Vertex v;
		v.pos = vertices[index*2].xyz;
		v.color = UnpackColorFromFloat(vertices[index*2].w);
		v.normal = vertices[index*2+1].xyz;
		v.uv = UnpackUVfromFloat(vertices[index*2+1].w);
		return v;
	}

	struct ProceduralGeometry {
		GeometryInstance geometryInstance;
		
		uint vertexOffset;
		uint objectIndex;
		uint material;
		
		vec3 aabbMin;
		vec3 aabbMax;
		vec4 color;
		float custom1;
	};

	ProceduralGeometry GetProceduralGeometry(uint geometryIndex) {
		ProceduralGeometry geom;
		
		geom.geometryInstance = GetGeometryInstance(geometryIndex);
		
		geom.vertexOffset = geom.geometryInstance.vertexOffset;
		geom.objectIndex = geom.geometryInstance.objectIndex;
		geom.material = geom.geometryInstance.material;
		
		geom.aabbMin = vertices[geom.vertexOffset*2].xyz;
		geom.aabbMax = vec3(vertices[geom.vertexOffset*2].w, vertices[geom.vertexOffset*2+1].xy);
		geom.color = UnpackColorFromFloat(vertices[geom.vertexOffset*2+1].z);
		geom.custom1 = vertices[geom.vertexOffset*2+1].w;
		
		return geom;
	}
	
#else

	#ifdef VISIBILITY_VERTEX_SHADER
		layout(location = 0) in vec3 in_pos;
		layout(location = 1) in uint _in_color;
		layout(location = 2) in vec3 in_normal;
		layout(location = 3) in uint _in_uv;
		Vertex GetVertex() {
			Vertex v;
			v.pos = in_pos.xyz;
			v.color = UnpackColorFromUint(_in_color);
			v.normal = in_normal.xyz;
			v.uv = UnpackUVfromUint(_in_uv);
			return v;
		}
	#endif
	
#endif
