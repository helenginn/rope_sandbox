#ifndef __vagabond_Quad_fsh__
#define __vagabond_Quad_fsh__

inline std::string Quad_fsh()
{
	std::string str = 
		"#version 330 core\n"\
		"\n"\
		"in vec3 normal;\n"\
		"in vec3 position;\n"\
		"in vec4 color;\n"\
		"in vec4 extra;\n"\
		"in vec2 tex;\n"\
		"\n"\
		"out vec4 vPos;\n"\
		"out vec2 vTex;\n"\
		"\n"\
		"void main()\n"\
		"{\n"\
		"    vec4 pos = vec4(position[0], position[1], position[2], 1.0);\n"\
		"	gl_Position = pos;\n"\
		"    vPos = pos;\n"\
		"    vec2 tex = pos.xy + vec2(1., 1.);\n"\
		"	tex /= 2.;\n"\
		"	vTex = tex;\n"\
		"}\n";
	return str;
}
