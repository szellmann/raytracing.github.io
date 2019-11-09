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


class scene: public hittable  {
    public:
        scene() {}
        scene(hittable **l, int n) {list = l; list_size = n; }

        virtual bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const;
        virtual bool bounding_box(double t0, double t1, aabb& output_box) const;

        hittable **list;
        int list_size;
};

bool scene::bounding_box(double t0, double t1, aabb& output_box) const {
    if (list_size < 1) return false;

    aabb temp_box;
    bool first_true = list[0]->bounding_box(t0, t1, temp_box);

    if (!first_true)
        return false;
    else
        output_box = temp_box;

    for (int i = 1; i < list_size; i++) {
        if (list[i]->bounding_box(t0, t1, temp_box))
            output_box = surrounding_box(output_box, temp_box);
        else
            return false;
    }

    return true;
}

bool scene::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    double closest_so_far = t_max;

    for (int i = 0; i < list_size; i++) {
        if (list[i]->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }
    return hit_anything;
}

#endif
