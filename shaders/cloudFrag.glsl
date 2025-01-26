#version 330 core
out vec4 FragColor;

uniform vec2 iResolution; 
uniform float iTime;    
uniform float iTurn;

float noise(vec2 uv)
{
    return fract(sin(uv.x * 113. + uv.y * 412.) * 6339.);
}

vec3 noiseSmooth(vec2 uv)
{
    vec2 index = floor(uv);
    
    vec2 pq = fract(uv);
    pq = smoothstep(0., 1., pq);
     
    float topLeft = noise(index);
    float topRight = noise(index + vec2(1, 0.));
    float top = mix(topLeft, topRight, pq.x);
    
    float bottomLeft = noise(index + vec2(0, 1));
    float bottomRight = noise(index + vec2(1, 1));
    float bottom = mix(bottomLeft, bottomRight, pq.x);
    
    return vec3(mix(top, bottom, pq.y));
}

void main()
{    
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    uv.x *= iResolution.x / iResolution.y;
    uv.x += iTurn;
    
    uv.y += iTime / 40.;
    
    vec2 uv2 = uv;
    uv2.y += iTime / 10.;
    
    vec2 uv3 = uv;
    uv3.y += iTime / 30.;
        
    vec3 col = noiseSmooth(uv * 4.);
    
    col += noiseSmooth(uv * 8.) * 0.5;
    col += noiseSmooth(uv2 * 16.) * 0.25;
    col += noiseSmooth(uv3 * 32.) * 0.125;
    col += noiseSmooth(uv3 * 64.) * 0.0625;
    
    col /= 2.;   
    
    col *= smoothstep(0.2, .6, col);   
    
    col = mix(1. - (col / 7.), vec3(0.37f, 0.63f, 0.74f), 1. - col);    
    
    FragColor = vec4(vec3(col), 0.6);
}
