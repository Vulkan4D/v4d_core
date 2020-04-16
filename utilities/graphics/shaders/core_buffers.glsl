layout(set = 0, binding = 1) readonly buffer ObjectBuffer {mat4 objectInstances[];};
layout(set = 0, binding = 2) readonly buffer LightBuffer {float lightSources[];};
layout(set = 0, binding = 3) readonly buffer ActiveLights {uint activeLights; uint lightIndices[];};
layout(set = 0, binding = 4) readonly buffer GeometryBuffer {uvec4 geometries[];};
layout(set = 0, binding = 5) readonly buffer IndexBuffer {uint indices[];};
layout(set = 0, binding = 6) readonly buffer VertexBuffer {vec4 vertices[];};

struct ObjectInstance {
	mat4 modelMatrix;
	mat4 custom4x4;
	mat4 modelViewMatrix;
	mat3 normalMatrix;
	vec3 position;
	vec3 custom3;
	vec4 custom4;
};

ObjectInstance GetObjectInstance(uint index) {
	ObjectInstance obj;
	obj.modelMatrix = objectInstances[index*4];
	obj.custom4x4 = objectInstances[index*4+1];
	obj.modelViewMatrix = objectInstances[index*4+2];
	mat4 secondMatrix = objectInstances[index*4+3];
	obj.normalMatrix = mat3(
		secondMatrix[0].xyz,
		vec3(secondMatrix[0].w, secondMatrix[1].x, secondMatrix[1].y),
		vec3(secondMatrix[1].z, secondMatrix[1].w, secondMatrix[2].x)
	);
	obj.position = obj.modelViewMatrix[3].xyz;
	obj.custom3 = secondMatrix[2].yzw;
	obj.custom4 = objectInstances[index*4+3][3];
	return obj;
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
		uint vertexOffset;
		uint objectIndex;
		uint material;
		
		vec3 aabbMin;
		vec3 aabbMax;
		vec4 color;
		float custom1;
		
		ObjectInstance objectInstance;
	};

	ProceduralGeometry GetProceduralGeometry(uint geometryIndex) {
		ProceduralGeometry geom;
		
		geom.vertexOffset = geometries[geometryIndex].y;
		geom.objectIndex = geometries[geometryIndex].z;
		geom.material = geometries[geometryIndex].w;
		
		geom.aabbMin = vertices[geom.vertexOffset*2].xyz;
		geom.aabbMax = vec3(vertices[geom.vertexOffset*2].w, vertices[geom.vertexOffset*2+1].xy);
		geom.color = UnpackColorFromFloat(vertices[geom.vertexOffset*2+1].z);
		geom.custom1 = vertices[geom.vertexOffset*2+1].w;
		
		geom.objectInstance = GetObjectInstance(geom.objectIndex);
		
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
