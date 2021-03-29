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
uniform vec3 palettes[12];
uniform uint tilesWidth;
uniform uint tilesHeight;
uniform sampler2D chrTex;
uniform sampler2D bgTex;
uniform uint spriteX;
uniform uint spriteY;
uniform uint spriteTile;
uniform uint tilePalette;
uniform bool bank;
uniform bool depth;
uniform bool flipH;
uniform bool flipV;
uniform bool mode8x16;


void main () {
	uint screenW = 32U;
	uint screenH = 30U;
	float spritePosX = screenW * 8U * texPos.x;
	float spritePosY = screenH * 8U * texPos.y;

	bool onX = (spritePosX > spriteX) && (spritePosX < spriteX + 8U);
	bool onY = (spritePosY > spriteY) && (spritePosY < spriteY + (mode8x16 ? 16U : 8U));

	uint onY2 = uint(spritePosY - spriteY) / 8U;

	uint value = spriteTile;
	if(mode8x16 && flipV) value++;
	if(bank) value += 256U;


	Frag_color = texture(bgTex, texPos);
	if(onX && onY){
		if(bool(onY2) && mode8x16){
			if(flipV)
				value--;
			else
				value++;
			spritePosY -= 8;
		}
		float tileX = (value % tilesWidth + abs((int(flipH) * 8 - (spritePosX - spriteX)) / 8)) / tilesWidth;
		float tileY = (value / tilesWidth + abs((int(flipV) * 8 - (spritePosY - spriteY)) / 8)) / tilesHeight;
		vec4 tileColor = texture(chrTex, vec2(tileX, tileY));
		if(tileColor.r == 1.) Frag_color = vec4(palettes[tilePalette * 3U] / 255, 1);
		else if(tileColor.g == 1.) Frag_color = vec4(palettes[tilePalette * 3U + 1U] / 255, 1);
		else if(tileColor.b == 1.) Frag_color = vec4(palettes[tilePalette * 3U + 2U] / 255, 1);
	}
}