/** 
 * @file avatarF.glsl
 *
 * $LicenseInfo:firstyear=2007&license=viewerlgpl$
 * $/LicenseInfo$
 */

uniform sampler2D diffuseMap;

varying vec3 vary_normal;

void main() 
{
	vec4 diff = gl_Color*texture2D(diffuseMap, gl_TexCoord[0].xy);
	
	if (diff.a < 0.2)
	{
		discard;
	}
	
	gl_FragData[0] = vec4(diff.rgb, 0.0);
	gl_FragData[1] = vec4(0,0,0,0);
	gl_FragData[2] = vec4(normalize(vary_normal)*0.5+0.5, 0.0);
}

