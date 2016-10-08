#version 150

uniform sampler2D uTex;
uniform float uTime;
uniform vec2 uSize;

in vec2 TexCoord;

out vec4 oColor;

// Adapted from https://www.shadertoy.com/view/MtGGRm
// Created by inigo quilez - iq/2015
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// 0 = Multiply
// 1 = Screen
// 2 = Overlay
#define BLEND_MODE 2

vec3 blend( vec3 a, vec3 b )
{
#if BLEND_MODE == 1
    return 1.0 - ( ( 1.0 - a ) * ( 1.0 - b ) );
#elif BLEND_MODE == 0
    return a * b;
#elif BLEND_MODE == 2
    if( a.x < 0.5 && a.y < 0.5 && a.z < 0.5 )
        return 2.0 * a * b;
    return 1.0 - 2.0 * ( 1.0 - a ) * ( 1.0 - b );
#endif
    return a;
}

void main()
{
    vec2 pos = 256.0 * TexCoord.xy * uSize.xy / uSize.x + uTime;
    vec3 col = vec3( 0.0 );
	
    for( int i = 0; i < 6; i++ ) {
        vec2 a = floor( pos );
        vec2 b = fract( pos );
        
        vec4 w = fract( ( sin( a.x * 7.0 + 31.0 * a.y + 0.01 * uTime ) + vec4( 0.035, 0.01, 0.0, 0.7 ) ) * 13.545317 ); // randoms
                
        col += w.xyz *                                   				 // color
               smoothstep( 0.45, 0.55, w.w) *            				 // intensity
               sqrt( 16.0 * b.x * b.y * ( 1.0 - b.x ) * ( 1.0 - b.y ) ); // pattern
        
        pos /= 2.0; // lacunarity
        col /= 2.0; // attenuate high frequencies
    }
    
    col = pow( 1.5 * col, vec3( 1.0, 1.0, 0.7 ) );    // contrast and color shape
    
    vec3 originalColor = texture( uTex, TexCoord ).rgb;
	
    oColor.xyz = blend( originalColor, col );
	oColor.a = 1.0;
}
