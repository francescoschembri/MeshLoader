#version 330 core

out vec4 FragColor;

uniform vec2 mousePos;
uniform vec2 resolution;
uniform float radius;


float check_inside_circle(vec2 coord, float radius) {
    return step(length(coord), radius);
}


void main()
{   
    vec2 coord = gl_FragCoord.xy/resolution;
    vec2 offset = vec2(mousePos.x, -mousePos.y)/resolution;
	FragColor = vec4(1.0,0.0,0.0,check_inside_circle(coord-offset, 0.2));
	if(FragColor.a < 0.1)
		discard;
}

