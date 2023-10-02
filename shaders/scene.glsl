#version 460 core

uniform mat4 view;
uniform vec3 cam_pos;
uniform bool enable_smoothing;
uniform float smoothing;
in vec2 pos;

out vec4 o_color;

const uint NB_STEPS = 256;
const float MAX_DIST = 512;

#define SPHERE 0
#define BOX 1
#define TORUS 2
#define PLANE 3

struct ObjectDesc {
    uint type;
    vec3 pos;
    vec3 material;
    // this should actually be a union
    float radius; // spheres
    vec3 size; // box
    vec2 torus; // torus
    vec4 normal; // plane
};

struct Object {
    float distance;
    vec3 material;
};

uniform ObjectDesc objects[32];
uniform uint nb_objects;

float smax(float a, float b, float k) {
    return log(exp(k * a) + exp(k * b)) / k;
}

float smin(float a, float b, float k) {
    return -smax(-a, -b, k);
}

float sdf_sphere(vec3 p1, vec3 p2, float radius) {
    return length(p2 - p1) - radius;
}

float sdf_box(vec3 pos, vec3 c, vec3 s) {
    float x = max(
        pos.x - c.x - s.x / 2.0,
        c.x - pos.x - s.x / 2.0
    );

    float y = max(
        pos.y - c.y - s.y / 2.0,
        c.y - pos.y - s.y / 2.0
    );

    float z = max(
        pos.z - c.z - s.z / 2.0,
        c.z - pos.z - s.z / 2.0
    );

    float d = x;
    d = max(d, y);
    d = max(d, z);
    return d;
}

float sdf_torus(vec3 p1, vec3 p2, vec2 t) {
    vec2 q = vec2(length((p2 - p1).xz) - t.x, p2.y - p1.y);
    return length(q) - t.y;
}

float sdf_plane(vec3 pos, vec4 n) {
    return dot(pos, n.xyz) + n.w;
}

Object obj_union(Object a, Object b) {
    return (a.distance < b.distance) ? a : b;
}

Object obj_sunion(Object a, Object b, float k) {
    Object u = {
        smin(a.distance, b.distance, k),
        // (a.distance < b.distance) ? a.material : b.material
        mix(a.material, b.material, clamp(a.distance - b.distance, 0.0, 1.0))
    };
    return u;
}

Object from_desc(vec3 pos, ObjectDesc desc) {
    Object obj;

    switch(desc.type) {
    case SPHERE:
        obj.distance = sdf_sphere(pos, desc.pos, desc.radius),
        obj.material = desc.material;
        break;
    case BOX:
        obj.distance = sdf_box(pos, desc.pos, desc.size),
        obj.material = desc.material;
        break;
    case TORUS:
        obj.distance = sdf_torus(pos, desc.pos, desc.torus),
        obj.material = desc.material;
        break;
    case PLANE:
        obj.distance = sdf_plane(pos, desc.normal),
        obj.material = desc.material;
        break;
    }

    return obj;
}

Object sample_sdf(vec3 pos) {
    Object result = from_desc(pos, objects[0]);
    for(uint i = 1; i < nb_objects; i++) {
        if(enable_smoothing) {
            result = obj_sunion(result, from_desc(pos, objects[i]), smoothing);
        } else {
            result = obj_union(result, from_desc(pos, objects[i]));
        }
    }

    /*
    Object sphere = {
        sdf_sphere(pos, vec3(0.0, 1.0, 3.0), 1.0),
        1
    };
    Object box = {
        sdf_box(pos, vec3(3.0, 2.0, 2.0), vec3(2.0, 1.0, 2.0)),
        2
    };
    Object torus = {
        sdf_torus(pos, vec3(0.0, 1.0, 14.0), vec2(0.5, 0.25)),
        2
    };
    Object plane = {
        sdf_plane(pos, vec4(0.0, 1.0, 0.0, 1.0)),
        3
    };
    return obj_union(obj_union(obj_union(sphere, torus), box), plane);
    */

    return result;
}

vec3 gen_normal(vec3 pos) {
    vec3 eps = vec3(0.0005, 0.0, 0.0);
    return normalize(vec3(
        sample_sdf(pos + eps.xyy).distance - sample_sdf(pos - eps.xyy).distance,
        sample_sdf(pos + eps.yxy).distance - sample_sdf(pos - eps.yxy).distance,
        sample_sdf(pos + eps.yyx).distance - sample_sdf(pos - eps.yyx).distance
    ));
}

struct RayResult {
    Object obj;
    float total_dist;
    vec3 pos;
};

RayResult raymarch(vec3 pos, vec3 dir) {
    vec3 ray = pos;
    RayResult res;
    res.pos = vec3(pos);
    res.total_dist = 0.0;

    for(uint i = 0; i < NB_STEPS; i++) {
        res.obj = sample_sdf(ray);

        if(res.obj.distance < 0.0001) {
            return res;
        }

        ray += res.obj.distance * dir;
        res.total_dist += res.obj.distance;
        res.pos = ray;
    }

    return res;
}

vec3 get_light(vec3 pos, vec3 dir, vec3 color) {
    vec3 light_pos = vec3(20.0, 40.0, -30.0);
    vec3 l = normalize(light_pos - pos);
    vec3 normal = gen_normal(pos);

    vec3 ambient = color * 0.05;

    vec3 diffuse = color * clamp(dot(l, normal), 0.0, 1.0);

    // shadows
    RayResult res = raymarch(pos + normal * 0.02, normalize(light_pos));
    if(res.obj.distance < length(light_pos - pos)) {
        return ambient;
    }

    return ambient + diffuse;
}

vec3 apply_fog(vec3 col, vec3 fog_col, float dist) {
    float start = 300.0;
    float amount = 1.0 - exp(-(dist - 8.0) * (1.0 / start));
    return mix(col, fog_col, amount);
}

void main() {
    vec3 raydir = normalize((vec4(pos, 1.0, 0.0)).xyz);
    vec3 direction = (view * vec4(raydir, 0.0)).xyz;

    vec3 background = vec3(0.5, 0.8, 0.9);

    vec3 col = vec3(0.0);

    RayResult res = raymarch(cam_pos, direction);
    if(res.obj.distance >= MAX_DIST) {
        col += background - max(0.4 * direction.y, 0.0);
    } else {
        // vec3 mat = get_material(res.pos, res.obj.id);
        col += get_light(res.pos, direction, res.obj.material);
        // fog
        col = apply_fog(col, background, res.total_dist * 3.0);
    }


    // gamma correction
    col = pow(col, vec3(0.4545));
    o_color = vec4(col, 1.0);
}
