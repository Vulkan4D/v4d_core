layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

layout(location = 0) in V2F in_v2f[];
layout(location = 0) out V2F out_v2f;

void main() {
	gl_Position = gl_in[0].gl_Position;
	out_v2f = in_v2f[0];
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	out_v2f = in_v2f[1];
	EmitVertex();
	
	EndPrimitive();
	
	gl_Position = gl_in[1].gl_Position;
	out_v2f = in_v2f[1];
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	out_v2f = in_v2f[2];
	EmitVertex();
	
	EndPrimitive();
	
	gl_Position = gl_in[2].gl_Position;
	out_v2f = in_v2f[2];
	EmitVertex();
	gl_Position = gl_in[0].gl_Position;
	out_v2f = in_v2f[0];
	EmitVertex();
	
	EndPrimitive();
}
