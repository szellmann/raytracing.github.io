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


struct hit_record;


double schlick(double cosine, double ref_idx) {
    auto r0 = (1-ref_idx) / (1+ref_idx);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}


bool refract(const vec3& v, const vec3& n, double ni_over_nt, vec3& refracted) {
    vec3 uv = unit_vector(v);
    auto dt = dot(uv, n);
    auto discriminant = 1.0 - ni_over_nt*ni_over_nt*(1-dt*dt);
    if (discriminant > 0) {
        refracted = ni_over_nt*(uv - n*dt) - n*sqrt(discriminant);
        return true;
    }
    else
        return false;
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

vec3 random_in_hemisphere(const vec3& normal) {
    vec3 in_unit_sphere = random_in_unit_sphere();
    if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
        return in_unit_sphere;
    else
        return -in_unit_sphere;
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
            vec3 outward_normal;
            vec3 reflected = reflect(r_in.direction(), rec.normal);
            double ni_over_nt;
            attenuation = vec3(1.0, 1.0, 1.0);
            vec3 refracted;
            double reflect_prob;
            double cosine;
            if (dot(r_in.direction(), rec.normal) > 0) {
                 outward_normal = -rec.normal;
                 ni_over_nt = ref_idx;
              // cosine =
              //     ref_idx * dot(r_in.direction(), rec.normal) / r_in.direction().length();
                 cosine = dot(r_in.direction(), rec.normal) / r_in.direction().length();
                 cosine = sqrt(1 - ref_idx*ref_idx*(1-cosine*cosine));
            }
            else {
                 outward_normal = rec.normal;
                 ni_over_nt = 1.0 / ref_idx;
                 cosine = -dot(r_in.direction(), rec.normal) / r_in.direction().length();
            }

            if (refract(r_in.direction(), outward_normal, ni_over_nt, refracted))
               reflect_prob = schlick(cosine, ref_idx);
            else
               reflect_prob = 1.0;

            if (random_double() < reflect_prob)
               scattered = ray(rec.p, reflected);
            else
               scattered = ray(rec.p, refracted);

            return true;
        }

        double ref_idx;
};

#endif
