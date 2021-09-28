#ifndef __Plot__Image_vsh__
#define __Plot__Image_vsh__

inline std::string Ramaplot_vsh()
{
	std::string str = 
	"#version 330 core\n"\
	"in vec3 normal;\n"\
	"in vec3 position;\n"\
	"in vec4 color;\n"\
	"in vec4 extra;\n"\
	"in vec2 tex;\n"\
	"\n"\
	"uniform mat4 model;\n"\
	"uniform mat4 projection;\n"\
	"uniform float time;\n"\
	"\n"\
	"out vec4 vPos;\n"\
	"out vec2 vTex;\n"\
	"out vec4 vColor;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"    vec4 pos = vec4(position[0], position[1], position[2], 1.0);\n"\
	"	 vPos = model * pos;\n"\
	"    gl_Position = projection * (vPos);\n"\
	"    gl_Position /= gl_Position.w;\n"\
	"    gl_Position += extra / 3;\n"\
	"    gl_Position.z = -0.1;\n"\
	"    vTex = tex;\n"\
	"    vColor = color;\n"\
	"    gl_PointSize = 10;\n"\
	"}";
	return str;
}

inline std::string Ramaplot_fsh() 
{
	std::string str = 
	"#version 330 core\n"\
	"in vec4 vPos;\n"\
	"in vec2 vTex;\n"\
	"\n"\
	"uniform sampler2D pic_tex;\n"\
	"\n"\
	"out vec4 FragColor;\n"\

	"void main()\n"\
	"{\n"\
	"	FragColor = texture(pic_tex, vTex);\n"\
	"}\n";

	return str;
}

inline std::string Ramapoint_fsh() 
{
	static std::string Blob_fsh =
	"#version 330 core\n"\
	"in vec4 vPos;\n"\
	"in vec3 vTex;\n"\
	"in vec4 vColor;\n"\
	"\n"\
	"uniform int depth;\n"\
	"\n"\
	"layout (location = 0) out vec4 FragColor;\n"\
	"layout (location = 1) out vec4 BrightColor;\n"\
	"\n"\
	"void toBright(vec4 basis)\n"\
	"{\n"\
	"	FragColor = basis;\n"\
	"	BrightColor = vec4(0, 0, 0, 1);\n"\
	"\n"\
	"	for (int i = 0; i < 3; i++)\n"\
	"	{\n"\
	"		if (FragColor[i] > 1)\n"\
	"		{\n"\
	"			BrightColor[i] = FragColor[i] - 1;\n"\
	"			FragColor[i] = 1;\n"\
	"		}\n"\
	"	}\n"\
	"}\n"\

	"void main()\n"\
	"{\n"\
	"\n"\
	"	vec2 pos = gl_PointCoord - vec2(0.5, 0.5);\n"\
	"	if (length(pos) > 0.5) discard;\n"\
	"	toBright(vColor);\n"\
	"\n"\
	"\n"\
	"\n"\
	"\n"\
	"}\n";
	return Blob_fsh;
}



#endif

