#shader vertex
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texturePos;

out vec2 texPos;
void main(){
	texPos = texturePos;
	gl_Position = vec4(position, 0.0, 1.0);
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 Frag_color;
uniform int textureWidth;
uniform int textureHeight;
#define dataSize 256
uniform uint data[dataSize];
uniform mat3 palette;
in vec2 texPos;	

//15		//bottom
//240
//3840
//61440		//top

void main(){
	float PosX = 22. * texPos.x;
	float PosY = 24. * texPos.y;
	uint relPosX = uint(PosX);
	uint relPosY = uint(24 * texPos.y);
	bool left = (float(relPosX) + .5) > (PosX);
	bool up = (float(relPosY) + .5) > (PosY);
	uint redPos = ((relPosY * 4U * 22U) + relPosX * 4U) + (left ? (!up ? 1U : 0U) : (!up ? 3U : 2U));
	uint line = uint(PosY - float(relPosY) * 4.); ////fix
	//else tile - 2;

	uint MSB = (line >> 7) & 2U;
	uint res = line & 3U;



	Frag_color = vec4(palette[res], 1);
}