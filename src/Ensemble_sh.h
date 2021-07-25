#ifndef __Blot__Image_vsh__
#define __Blot__Image_vsh__

inline std::string Ensemble_vsh()
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
	"out vec4 vColor;\n"\
	"out vec4 vGlow;\n"\
	"out vec4 vPos;\n"\
	"out vec2 vTex;\n"\
	"\n"\
	"void main()\n"\
	"{\n"\
	"    vec4 pos = vec4(position[0], position[1], position[2], 1.0);\n"\
	"	 vec4 norm4 = vec4(normal[0], normal[1], normal[2], 1.0);\n"\
	"	 pos = model * pos;\n"\
	"	 vec3 norm3 = mat3(model) * norm4.xyz;\n"\
	"	 vec3 lightpos = vec3(pos[0], pos[1], pos[2]);\n"\
	"    float mag = dot(normalize(norm3), normalize(lightpos));"\
	"    float red   = color[0] * mag;\n"
	"    float green = color[1] * mag;\n"
	"    float blue =  color[2] * mag;\n"
	"    vGlow = extra;\n"\
	"    vPos = projection * pos;\n"\
	"    gl_Position = vPos;\n"\
	"	 vColor = vec4(red, green, blue, 1);\n"\
	"    vTex = tex;\n"\
	"}";
	return str;
}

inline std::string Ensemble_fsh() 
{
	std::string str = 
	"#version 330 core\n"\
	"in vec4 vColor;\n"\
	"in vec4 vGlow;\n"\
	"in vec2 vTex;\n"\
	"in vec4 vPos;\n"\
	"\n"\
	"uniform sampler2D pic_tex;\n"\
	"\n"\
	"layout (location = 0) out vec4 FragColor;\n"\
	"layout (location = 1) out vec4 BrightColor;\n"\

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

	"\n"\
	"void main()\n"\
	"{\n"\
	"	toBright(vColor);\n"\
	"	BrightColor += vGlow;\n"\
	"	FragColor[3] = vColor[3];\n"\
	"\n"\
	"\n"\
	"\n"\
	"}\n";
	return str;
}


#endif
