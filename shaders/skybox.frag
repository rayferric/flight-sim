#version 460

layout(location = 0) out vec4 out_Color;

layout(location = 1) in vec2 v_TexCoord;

layout(std140, binding = 0) uniform FrameUBO {
	mat4 projection;
	mat4 view;
} u_Frame;

vec3 normalizedMul(in mat4 matrix, in vec3 pos) {
	vec4 clip = matrix * vec4(pos, 1.0);
	return clip.xyz / clip.w;
}

vec3 screenToView(in vec2 screenPos, in float depth) {
	return normalizedMul(
	    inverse(u_Frame.projection), vec3(screenPos, depth) * 2.0 - 1.0
	);
}

#define PI 3.14159265359

// Dimensions
#define PLANET_RADIUS     6371e3
#define ATMOSPHERE_HEIGHT 100e3
#define RAYLEIGH_HEIGHT   8e3
#define MIE_HEIGHT        1.2e3
#define OZONE_PEAK_LEVEL  30e3
#define OZONE_FALLOFF     3e3
// Scattering coefficients
#define BETA_RAY   vec3(3.8e-6, 13.5e-6, 33.1e-6) // vec3(5.5e-6, 13.0e-6, 22.4e-6)
#define BETA_MIE   vec3(1e-6)
#define BETA_OZONE vec3(2.04e-5, 4.97e-5, 1.95e-6)
#define G          0.75
// Samples
#define SAMPLES          4
#define LIGHT_SAMPLES    1 // Set to more than 1 for a realistic, less vibrant sunset

// Other
#define SUN_ILLUMINANCE   128000.0
#define MOON_ILLUMINANCE  0.32
#define SPACE_ILLUMINANCE 0.01

const float ATMOSPHERE_RADIUS = PLANET_RADIUS + ATMOSPHERE_HEIGHT;

/**
 * Computes entry and exit points of ray intersecting a sphere.
 *
 * @param origin    ray origin
 * @param dir       normalized ray direction
 * @param radius    radius of the sphere
 *
 * @return    .x - position of entry point relative to the ray origin | .y - position of exit point relative to the ray origin | if there's no intersection at all, .x is larger than .y
 */
vec2 raySphereIntersect(in vec3 origin, in vec3 dir, in float radius) {
	float a = dot(dir, dir);
	float b = 2.0 * dot(dir, origin);
	float c = dot(origin, origin) - (radius * radius);
	float d = (b * b) - 4.0 * a * c;
    
	if(d < 0.0)return vec2(1.0, -1.0);
	return vec2(
		(-b - sqrt(d)) / (2.0 * a),
		(-b + sqrt(d)) / (2.0 * a)
	);
}

/**
 * Phase function used for Rayleigh scattering.
 *
 * @param cosTheta    cosine of the angle between light vector and view direction
 *
 * @return    Rayleigh phase function value
 */
float phaseR(in float cosTheta) {
    return (3.0 * (1.0 + cosTheta * cosTheta)) / (16.0 * PI);
}

/**
 * Henyey-Greenstein phase function, used for Mie scattering.
 *
 * @param cosTheta    cosine of the angle between light vector and view direction
 * @param g           scattering factor | -1 to 0 - backward | 0 - isotropic | 0 to 1 - forward
 *
 * @return    Henyey-Greenstein phase function value
 */
float phaseM(in float cosTheta, in float g) {
	float gg = g * g;
	return (1.0 - gg) / (4.0 * PI * pow(1.0 + gg - 2.0 * g * cosTheta, 1.5));
}

/**
 * Approximates density values for a given point around the planet.
 *
 * @param pos    position of the point, for which densities are calculated
 *
 * @return    .x - Rayleigh density | .y - Mie density | .z - ozone density
 */
vec3 avgDensities(in vec3 pos) {
	float height = length(pos) - PLANET_RADIUS; // Height above surface
	vec3 density;
	density.x = exp(-height / RAYLEIGH_HEIGHT);
	density.y = exp(-height / MIE_HEIGHT);
    density.z = (1.0 / cosh((OZONE_PEAK_LEVEL - height) / OZONE_FALLOFF)) * density.x; // Ozone absorption scales with rayleigh
    return density;
}

/**
 * Calculates atmospheric scattering value for a ray intersecting the planet.
 *
 * @param pos         ray origin
 * @param dir         ray direction
 * @param lightDir    light vector
 *
 * @return    sky color
 */
vec3 atmosphere(
	in vec3 pos,
	in vec3 dir,
	in vec3 lightDir
) {
	// Intersect the atmosphere
    vec2 intersect = raySphereIntersect(pos, dir, ATMOSPHERE_RADIUS);

	// Accumulators
	vec3 opticalDepth = vec3(0.0); // Accumulated density of particles participating in Rayleigh, Mie and ozone scattering respectively
    vec3 sumR = vec3(0.0);
    vec3 sumM = vec3(0.0);
    
    // Here's the trick - we clamp the sampling length to keep precision at the horizon
    // This introduces banding, but we can compensate for that by scaling the clamp according to horizon angle
    float rayPos = max(0.0, intersect.x);
    float maxLen = ATMOSPHERE_HEIGHT;
    maxLen *= (1.0 - abs(dir.y) * 0.5);
	float stepSize = min(intersect.y - rayPos, maxLen) / float(SAMPLES);
    rayPos += stepSize * 0.5; // Let's sample in the center
    
    for(int i = 0; i < SAMPLES; i++) {
        vec3 samplePos = pos + dir * rayPos; // Current sampling position

		// Similar to the primary iteration
		vec2 lightIntersect = raySphereIntersect(samplePos, lightDir, ATMOSPHERE_RADIUS); // No need to check if intersection happened as we already are inside the sphere

        vec3 lightOpticalDepth = vec3(0.0);
        
        // We're inside the sphere now, hence we don't have to clamp ray pos
        float lightStep = lightIntersect.y / float(LIGHT_SAMPLES);
        float lightRayPos = lightStep * 0.5; // Let's sample in the center
        
        for(int j = 0; j < LIGHT_SAMPLES; j++) {
            vec3 lightSamplePos = samplePos + lightDir * (lightRayPos);

			lightOpticalDepth += avgDensities(lightSamplePos) * lightStep;

            lightRayPos += lightStep;
        }

		// Accumulate optical depth
		vec3 densities = avgDensities(samplePos) * stepSize;
		opticalDepth += densities;

		// Accumulate scattered light
        vec3 scattered = exp(-(BETA_RAY * (opticalDepth.x + lightOpticalDepth.x) + BETA_MIE * (opticalDepth.y + lightOpticalDepth.y) + BETA_OZONE * (opticalDepth.z + lightOpticalDepth.z)));
        sumR += scattered * densities.x;
        sumM += scattered * densities.y;

        rayPos += stepSize;
    }

    float cosTheta = dot(dir, lightDir);
    
    return max(
        phaseR(cosTheta)    * BETA_RAY * sumR + // Rayleigh color
       	phaseM(cosTheta, G) * BETA_MIE * sumM,  // Mie color
    	0.0
    );
}

/**
 * Draws a blackbody as seen from the planet.
 *
 * @param dir         ray direction
 * @param lightDir    light vector
 *
 * @return    blackbody color
 */
vec3 renderBlackbody(in vec3 dir, in vec3 lightDir) {
    float cosTheta = dot(dir, lightDir);
    
    float intensity = smoothstep(0.999, 0.9995, cosTheta);
    float glow = pow(max(cosTheta, 0.0), 4.0) * 0.01;
	
    float fade = smoothstep(0.05, 0.25, dir.y);
    float glowFade = smoothstep(0.05, 0.25, lightDir.y);
    
    return vec3(intensity + glow * glowFade) * fade;
}

/**
 * Calculates daylight factor at given sun height.
 *
 * @param sunHeight    sun height
 *
 * @return    daylight factor in range <0.0, 1.0>
 */
float getDayFactor(in float sunHeight) {
    return pow(smoothstep(-0.6, 0.6, sunHeight), 8.0);
}

/**
 * Computes shadow light illuminance at given sun height.
 *
 * @param sunHeight    sun height
 *
 * @return    shadow light illuminance
 */
float getShadowIlluminance(in float sunHeight) {
    return mix(MOON_ILLUMINANCE, SUN_ILLUMINANCE, getDayFactor(sunHeight - 0.2));
}

/**
 * Rotates two dimensional coordinate around the origin.
 *
 * @param coord    two-component coordinate
 * @param angle    rotation angle in radians
 *
 * @return    rotated coordinate
 */
vec2 rotate(in vec2 coord, float angle) {
    vec2 t = vec2(sin(angle), cos(angle));
    return vec2(coord.x * t.y - coord.y * t.x, dot(coord, t));
}

/**
 * Calculates the view direction of a pixel based on its location.
 *
 * @param uv    fragment position in range [0.0, 1.0] on both axes
 *
 * @return    normalized view direction
 */
vec3 viewDir(in vec2 uv, in float ratio) {
    uv = uv * 2.0 - 1.0;
	uv.x *= ratio;
	return normalize(vec3(uv.x, uv.y, -1.0));
    
	// vec2 t = ((uv * 2.0) - vec2(1.0)) * vec2(PI, PI * 0.5); 
    // return normalize(vec3(cos(t.y) * cos(t.x), sin(t.y), cos(t.y) * sin(t.x)));
}

/**
 * Transforms HDR color to LDR space using the ACES operator.
 * Ported from original source:
 * https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve
 * For a more accurate curve, head to:
 * https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
 *
 * @param color    HDR color
 *
 * @return    LDR color
 */
vec3 tonemapACES(in vec3 color) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main() {
	vec3 viewFragPos  = screenToView(v_TexCoord, 0.0);
	vec3 viewEyeDir  = normalize(viewFragPos);
	vec3 worldEyeDir = normalize(mat3(inverse(u_Frame.view)) * viewEyeDir);

	vec3 worldEyeDirGl = vec3(-worldEyeDir.y, worldEyeDir.z, -worldEyeDir.x);
	
	vec3 pos = vec3(0.0, PLANET_RADIUS + 2.0, 0.0);
	vec3 dir = worldEyeDirGl;
	vec3 sunDir = vec3(0.5, -0.05, -1.0);
    dir = normalize(dir);
    sunDir = normalize(sunDir);
    
    float shadowIlluminance = getShadowIlluminance(sunDir.y);
	// Sky
	vec3 color = atmosphere(pos, dir, sunDir) * shadowIlluminance;
    color += atmosphere(pos, dir, -sunDir) * shadowIlluminance;
    // // Blackbodies
    // color += renderBlackbody(dir, sunDir) * shadowIlluminance;
    // color += renderBlackbody(dir, -sunDir) * shadowIlluminance;
    
    // Tonemapping
    float exposure = 16.0 / shadowIlluminance;
    exposure = min(exposure, 16.0 / (MOON_ILLUMINANCE * 8.0)); // Clamp the exposure to make night appear darker
    color = tonemapACES(color * exposure);
    color = pow(color, vec3(1.0 / 2.2));

	out_Color = vec4(color, 1.0);
}
