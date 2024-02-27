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
  float len = 1.0 - (Length * 2.0);
  // The smooth step function basically interpolates well.. smoother (the step function would give a value of 0 
  // (outside the circle) or 1 (inside the circle) but the
  // smooth step will interpolate a bit so it blurs the end of the circle)
  // so it gives a way of anti-aliasing.
  vec4 col = vec4(smoothstep(0.0, 0.005, len)); 
  color = col * u_Color;
};