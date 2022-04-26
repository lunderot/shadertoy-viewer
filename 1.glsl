void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec2 uv = (fragCoord - iResolution.xy * 0.5) / iResolution.xy;
    float time = iTime;
    float x = uv.x;
    float y = -uv.y;
    float pi = 3.141592;

    float v = 0.0;
    v += sin((x * 10.0 + time));
    v += sin((y * 10.0 + time) / 2.0);
    v += sin((x * 10.0 + y * 10.0 + time) / 2.0);
    float cx = x + .5 * sin(time / 5.0);
    float cy = y + .5 * cos(time / 3.0);
    v += sin(sqrt(100.0 * (cx * cx + cy * cy) + 1.0) + time);
    v = v / 2.0;

    vec3 color;

    color.r = sin(v*pi) * .5 + .5;
    color.g = sin(v*pi+(2.0*pi/3.0)) * .5 + .5;
    color.b = sin(v*pi+(4.0*pi/3.0)) * .5 + .5;

    fragColor = vec4(color, 1.0);
}
