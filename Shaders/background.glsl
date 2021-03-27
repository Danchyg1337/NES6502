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
in vec2 texPos;
uniform vec3 bgColor;
uniform vec3 palettes[12];
uniform uint tilesWidth;
uniform uint tilesHeight;
uniform sampler2D chrTex;
uniform uint data[256];
uniform uint colors[36];
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
	uint int32indexX = uint(PosX) % 4U;

	uint a[4];
	a[0] = 0U;
	a[1] = 8U;
	a[2] = 16U;
	a[3] = 24U;

	uint value = data[(screenW * relPosY + relPosX) / 4U];

	value = (value >> a[int32indexX]) & 0xFFU;

	if(bank) value += 256U;

	vec4 tileColor = texture(chrTex, vec2((value % tilesWidth + fractX) / tilesWidth, (value / tilesWidth + fractY) / tilesHeight));

	float colorsPosX = 2 * texPos.x;
	float colorsPosY = 8 * texPos.y;

	uint colorsRelPosX = uint(colorsPosX);
	uint colorsRelPosY = uint(colorsPosY);

	uint colorsSet = colors[colorsRelPosY * 2U + colorsRelPosX];
	float colorsFractX = fract(colorsPosX);
	uint colorsTileIndex = uint(colorsFractX * 4.0);

	uint colorsTile = (colorsSet >> a[colorsTileIndex]) & 0xFFU;

	uint tileSideX = uint(fract(colorsPosX * 4) * 2.0);
	uint tileSideY = uint(fract(colorsPosY) * 2.0);

	uint colorNum = (colorsTile >> ((tileSideY * 4U) + (uint(tileSideX) * 2U))) & 0x3U;

	if(tileColor.r == 1.) Frag_color = vec4(palettes[colorNum * 3U] / 255, 1);
	else if(tileColor.g == 1.) Frag_color = vec4(palettes[colorNum * 3U + 1U] / 255, 1);
	else if(tileColor.b == 1.) Frag_color = vec4(palettes[colorNum * 3U + 2U] / 255, 1);
	else Frag_color = vec4(bgColor / 255, 1);
	//Frag_color = vec4(vec3(float (tileSideY) / 4., .5, .5 ), 1);
	//Frag_color = texture(chrTex, vec2((value % tilesWidth + fractX) / tilesWidth, (value / tilesWidth + fractY) / tilesHeight));
}