#shader vertex
#version 330 core

layout(location=0) in vec4 position;

void main()
{
  gl_Position = position;
};

#shader fragment
#version 330 core

layout(location=0) out vec4 color;

uniform vec4 u_Color;
uniform float CameraZoom;
uniform vec2 CenterPoint;

void main()
{
  vec2 Point = CenterPoint - gl_FragCoord.xy / CameraZoom;
  if(mod(Point.x, 100) >= 0.0 && mod(Point.x , 100) <= 2.0 || mod(Point.y, 100) >= 0.0 && mod(Point.y, 100) <= 2.0){
    color = u_Color;
  }else{
    color = vec4(0);
  }
};