#ifndef SCENE_H
#define SCENE_H
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

#include "hittable.h"
#include "material.h"

#include <vector>


struct scene_object {
    hittable *primitive;
    material *mat;
};


class scene: public hittable  {
    public:
        ~scene();

        void add(hittable*, material*);
        virtual bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const;

        std::vector<scene_object> objects;
};

scene::~scene() {
    for (auto object : objects) {
        delete object.primitive;
        delete object.mat;
    }
}

void scene::add(hittable* object, material* mat) {
    objects.push_back({object, mat});
}

bool scene::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    double closest_so_far = t_max;

    for (auto object : objects) {
        if (object.primitive->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
            rec.mat = object.mat;
        }
    }

    return hit_anything;
}

#endif
