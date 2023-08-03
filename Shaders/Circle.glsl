#shader vertex
#version 330 core

layout(location=0) in vec4 position;
uniform mat4 u_MVP;

void main()
{
  gl_Position = u_MVP * position;
};

#shader fragment
#version 330 core

layout(location=0) out vec4 color;

uniform vec4 u_Color;
uniform float RectWidth;
uniform float RectHeight;
uniform vec2 StartPoint;
uniform float CameraZoom;
void main()
{
  vec2 Point;
  vec2 Center = vec2(0.5, 0.5);
  Point.x = (gl_FragCoord.x / CameraZoom - StartPoint.x) / RectWidth;
  Point.y = (gl_FragCoord.y / CameraZoom - StartPoint.y) / RectHeight;
  float Length = sqrt(((Point.x - Center.x) * (Point.x - Center.x)) + ((Point.y - Center.y) * (Point.y - Center.y)));
  if(Length <= 0.5){
    color = u_Color;
  }else{
    color = vec4(0.0, 0.0, 0.0, 0.0);
  }
};