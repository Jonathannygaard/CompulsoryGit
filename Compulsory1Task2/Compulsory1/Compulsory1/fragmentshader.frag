#version 330 core
out vec4 fragmentColor;

in vec3 vertexColor;

void main(){
	fragmentColor = vec4(vertexColor,1.0);
}