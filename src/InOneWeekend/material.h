#ifndef MATERIAL_H
#define MATERIAL_H
//==============================================================================================
// Originally written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication
// along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==============================================================================================

#include "common/rtweekend.h"
#include "hittable.h"
#include "ray.h"

#include <algorithm>

struct hit_record;


double schlick(double cosine, double ref_idx) {
    auto r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}

inline vec3 refract(const vec3& uv, const vec3& n, double eta_i_over_eta_t) {
    auto dt = dot(uv, n);
    auto discriminant = 1.0 - eta_i_over_eta_t*eta_i_over_eta_t*(1-dt*dt);
    return eta_i_over_eta_t*(uv - n*dt) - n*sqrt(discriminant);
}


vec3 reflect(const vec3& v, const vec3& n) {
     return v - 2*dot(v,n)*n;
}


vec3 random_unit_vector() {
    auto a = 2*pi * random_double();
    auto z = 2*random_double() - 1;
    auto r = sqrt(1 - z*z);
    return vec3(r*cos(a), r*sin(a), z);
}


vec3 random_in_unit_sphere() {
    vec3 p;
    do {
        p = 2.0*vec3(random_double(),random_double(),random_double()) - vec3(1,1,1);
    } while (p.squared_length() >= 1.0);
    return p;
}


class material  {
    public:
        virtual bool scatter(
            const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered
        ) const = 0;
};


class lambertian : public material {
    public:
        lambertian(const vec3& a) : albedo(a) {}
        virtual bool scatter(
            const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered
        ) const  {
             vec3 target = rec.p + rec.normal + random_unit_vector();
             scattered = ray(rec.p, target-rec.p);
             attenuation = albedo;
             return true;
        }

        vec3 albedo;
};


class metal : public material {
    public:
        metal(const vec3& a, double f) : albedo(a) { if (f < 1) fuzz = f; else fuzz = 1; }
        virtual bool scatter(
            const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered
        ) const  {
            vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
            scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
            attenuation = albedo;
            return (dot(scattered.direction(), rec.normal) > 0);
        }
        vec3 albedo;
        double fuzz;
};


class dielectric : public material {
    public:
        dielectric(double ri) : ref_idx(ri) {}
        virtual bool scatter(
            const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered
        ) const  {
            vec3 unit_direction = unit_vector(r_in.direction());
            
            vec3 local_normal;
            float eta_i, eta_t;
            if (dot(r_in.direction(), rec.normal) < 0) {
                local_normal = rec.normal;
                eta_i = 1.0;
                eta_t = ref_idx;
            }
            else {
                local_normal = -rec.normal;
                eta_i = ref_idx;
                eta_t = 1.0;
            }

            vec3 scatter_direction;
            double cos_theta_i = std::min(dot(-unit_direction, local_normal), 1.0);
            double sin_theta_i = sqrt(1.0 - cos_theta_i*cos_theta_i);
            if ((eta_i/eta_t) * sin_theta_i > 1.0 ) {
                scattered = ray(rec.p, reflect(unit_direction, local_normal));
                attenuation = vec3(1.0, 1.0, 1.0);
                return true;
            }
            
            double reflect_prob = schlick(cos_theta_i, eta_i / eta_t);
            if (random_double() < reflect_prob)
            {
                scattered = ray(rec.p, reflect(unit_direction, local_normal));
                attenuation = vec3(1.0, 1.0, 1.0);
                return true;
            }
            else
            {
                double sin_theta_t = std::min(((eta_i/eta_t) * sin_theta_i), 1.0);
                double cos_theta_t = sqrt(1.0 - sin_theta_t * sin_theta_t);
                scattered = ray(rec.p, 
                    (eta_i/eta_t)*(unit_direction + cos_theta_i * local_normal) -
                    cos_theta_t * local_normal);
                attenuation = vec3(1.0, 1.0, 1.0);
                return true;
            }
        }

        double ref_idx;
};

#endif
