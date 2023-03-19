uniform float time;
uniform vec3 cam_pos;
uniform vec3 cam_forward;
uniform vec3 cam_up;
uniform vec3 cam_right;
uniform float cam_fov;
uniform float cam_ar;

const int DIRECT_COMPLEXITY = 128;
const int SHADOW_COMPLEXITY = 16;
const float CONTACT_THRESHOLD = 0.01;

const vec3 SUN_DIRECTION = vec3(0.3, -1.0, -0.3);
const vec3 SUN_COLOR = vec3(1.1, 1.0, 1.0);
const vec3 AMBIENT_COLOR = vec3(1.0, 1.1, 1.0) * 0.3;
const vec3 FOG_COLOR = vec3(0.7, 0.75, 1.0);
const vec3 SKY_COLOR = vec3(0.1, 0.13, 0.6);
const vec3 ATMO_SAT = vec3(0.9, 0.9, 0.95);

const vec3 GLOBE_COLOR = vec3(0.2, 0.2, 0.3);
const vec3 LAKE_COLOR = vec3(0.1, 0.1, 0.7);
const vec3 FOAM_COLOR = vec3(0.3, 0.3, 0.7);
const vec3 LAND_COLOR = vec3(0.2, 0.3, 0.1);

const vec3 GLOBE_POS = vec3(0.0, 1.2, 0.0);
const float GLOBE_RADIUS = 0.53;

const float LAKE_HEIGHT = 0.4;

float square(float x) {return x * x;}
float cube(float x) {return x * x * x;}

float frac(float x) {return x - floor(x); }

float Noise(vec3 pos) {
    return fract(abs(sin(dot(pos, vec3(12.9898, 78.233, 151.7182)))) * 43758.5453);
}

float InterpDistance(vec3 a, vec3 b) {
    return (1.0 - abs(a.x - b.x)) * (1.0 - abs(a.y - b.y)) * (1.0 - abs(a.z - b.z));
}

vec3 mixclamp(vec3 a, vec3 b, float t) {return mix(a, b, clamp(t, 0.0, 1.0));}

float SmoothNoise(vec3 pos) {
    int round_x = int(floor(pos.x));
    int round_y = int(floor(pos.y));
    int round_z = int(floor(pos.z));
    float result = 0.0;
    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = 0; dy <= 1; ++dy) {
            for (int dz = 0; dz <= 1; ++dz) {
                vec3 vertex = vec3(float(round_x + dx), float(round_y + dy), float(round_z + dz));
                // result += Noise(vertex);// * clamp(InterpDistance(vertex, pos), 0.0, 1.0);
                result += Noise(vertex) * clamp(InterpDistance(vertex, pos), 0.0, 1.0);
            }
        }
    }
    return result;
}

float SphereSdf(vec3 pos, vec3 center, float radius) {
    return distance(pos, center) - radius;
}

float GlobeSdf(vec3 pos) {
    return SphereSdf(pos, GLOBE_POS + vec3(0.0, sin(time * 0.3) * 0.1, 0.0), GLOBE_RADIUS);
}

float LandSdf(vec3 pos) {
    return (pos.y - LAKE_HEIGHT + 
        0.2 * cos(length(pos.xz) * 3.0 + 0.2) + 
        0.02 * SmoothNoise(vec3(pos.x * 10.0, 0.0, pos.z * 10.0))) * 0.9;
}

float LakeSdf(vec3 pos) {
    vec3 wave_projection = vec3(0.0, 0.0, pos.x);
    return (pos.y - LAKE_HEIGHT + 
        sin(pos.x * 34.0 + time * 3.0) * cos(pos.z * 76.0 + time * 2.0) * 0.003) * 0.9;
}

float SceneSdf(vec3 pos) {
    return min(min(GlobeSdf(pos), LakeSdf(pos)), LandSdf(pos));
}

const float DERIV_STEP = 0.001;

vec3 SceneNormal(vec3 pos) {
    vec3 normal = vec3(0.0, 0.0, 0.0);
    float dot_sdf = SceneSdf(pos);
    normal.x = (SceneSdf(pos + vec3(1.0, 0.0, 0.0) * DERIV_STEP) - dot_sdf) / DERIV_STEP;
    normal.y = (SceneSdf(pos + vec3(0.0, 1.0, 0.0) * DERIV_STEP) - dot_sdf) / DERIV_STEP;
    normal.z = (SceneSdf(pos + vec3(0.0, 0.0, 1.0) * DERIV_STEP) - dot_sdf) / DERIV_STEP;

    float lake = LakeSdf(pos);
    float globe = GlobeSdf(pos);
    float land = LandSdf(pos);

    if (lake != dot_sdf) return normalize(normal);
    // return normalize(normal);

    // Bump = sin(pos.x + time) * cos(pos.z * 3.0 + time)
    float x_factor = pos.x * 40.0;
    float z_factor = pos.z * 70.0;
    return normalize(normal + // Simulate waves on the surface of the water
        vec3(cos(x_factor + time) * cos(z_factor * 3.0 + time), 0.0,
        -3.0 * sin(x_factor + time) * sin(z_factor * 3.0 + time)) * 0.01);
}

int SceneId(vec3 pos) {
    float sdf = SceneSdf(pos);
    if (GlobeSdf(pos) == sdf) return 0;
    if (LakeSdf(pos) == sdf) return 1;
    if (LandSdf(pos) == sdf) return 2;
}

float InShadow(vec3 pos) {
    vec3 shadow_ray_pos = pos - SUN_DIRECTION * 0.01;
    for (int step_id = 0; step_id < SHADOW_COMPLEXITY; ++step_id) {
        shadow_ray_pos -= normalize(SUN_DIRECTION) * SceneSdf(shadow_ray_pos);
    }
    if (SceneSdf(shadow_ray_pos) >= CONTACT_THRESHOLD) return 1.0;

    float shadow = 0.0;
    return SceneSdf(shadow_ray_pos) / CONTACT_THRESHOLD;
}

// Blinn-Phong specular highlight base
float Specular(vec3 normal, vec3 light_dir, vec3 to_camera) {
    vec3 semi = normalize(-light_dir + to_camera);
    return max(0.0, dot(semi, normal));
}

float Directional(vec3 normal, vec3 light_dir) {
    return max(0.0, dot(-normal, light_dir));
}

void main() {
    vec3 final_color = vec3(0.0, 0.0, 0.0);

    vec2 screen_coord = gl_TexCoord[0].xy * 2.0 - vec2(1.0, 1.0);

    vec3 ray_position = cam_pos;
    vec3 ray_direction = normalize(cam_forward / tan(cam_fov / 2.0) + 
                        screen_coord.x * cam_right - screen_coord.y * cam_up / cam_ar);

    for (int step_id = 0; step_id < DIRECT_COMPLEXITY; step_id++) {
        float distance = SceneSdf(ray_position);
        if (distance < CONTACT_THRESHOLD) break;
        ray_position += ray_direction * distance;
    }

    vec3 sky_color = mix(FOG_COLOR, SKY_COLOR, sqrt(max(0.0, ray_direction.y)));

    if (SceneSdf(ray_position) > CONTACT_THRESHOLD) {
        final_color = sky_color;
    } else {
        int object_id = SceneId(ray_position);
        vec2 material = vec2(0.0, 0.0);

        if (object_id == 0) {material = vec2(0.2, 30.0); final_color = GLOBE_COLOR;}
        if (object_id == 1) {material = vec2(1.0, 50.0); 
                             final_color = mixclamp(FOAM_COLOR, LAKE_COLOR, LandSdf(ray_position) * 10.0);}
        if (object_id == 2) {material = vec2(0.2, 30.0); final_color = LAND_COLOR;}

        vec3 normal = SceneNormal(ray_position);

        float direct = Directional(normal, SUN_DIRECTION);
        if (direct > 0.0001) direct *= InShadow(ray_position);
        vec3 indirect = AMBIENT_COLOR;
        if (object_id == 0) indirect += LAKE_COLOR * max(0.0, -1.0 * normal.y) * 0.3;

        final_color *= SUN_COLOR * direct + indirect;

        float dist = distance(cam_pos, ray_position);
        vec3 lambda = vec3(pow(ATMO_SAT.x, dist), pow(ATMO_SAT.y, dist), pow(ATMO_SAT.z, dist));
        final_color = final_color * lambda + sky_color * (1.0 - lambda);
        if (direct > 0.0001)
            final_color += material.r * pow(Specular(normal, SUN_DIRECTION, normalize(cam_pos - ray_position)), material.g);
    }

    gl_FragColor = vec4(final_color.r, final_color.g, final_color.b, 1.0);

    // gl_FragColor = vec4(SmoothNoise(ray_position));
}
