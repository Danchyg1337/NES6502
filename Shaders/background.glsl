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
uniform mat3 palette;
in vec2 texPos;
uniform uint tilesWidth;
uniform uint tilesHeight;
uniform sampler2D chrTex;
uniform uint data[256];
uniform bool bank;


void main () {
	uint screenW = 32U;
	uint screenH = 30U;

	float PosX = screenW * texPos.x;
	float PosY = screenH * texPos.y;

	uint relPosX = uint(PosX);
	uint relPosY = uint(PosY);

	float fractX = fract(PosX);
	float fractY = fract(PosY);
	uint int32indexX = uint(fract(PosX * .25) * 4.0);

	uint a[4];
	a[0] = 0U;
	a[1] = 8U;
	a[2] = 16U;
	a[3] = 24U;

	uint value = data[(screenW * relPosY + relPosX) / 4U];

	value = (value >> a[int32indexX]) & 0xFFU;

	if(bank) value += 256U;

	Frag_color = texture(chrTex, vec2((value % tilesWidth + fractX) / tilesWidth, (value / tilesWidth + fractY) / tilesHeight));
	//Frag_color = vec4(vec3(float (int32indexX) / 4., .5, .5 ), 1);
	//Frag_color = vec4(float(value) * .01, 0, 0, 1);
}