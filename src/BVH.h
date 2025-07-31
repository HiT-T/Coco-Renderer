#ifndef BVH_H
#define BVH_H

#include "AABB.h"
#include "Object.h"

#include <algorithm>

class BVHNode : public Object {
    public:
        BVHNode(std::vector<shared_ptr<Object>> &objects) : BVHNode(objects, 0, objects.size()) {}

        BVHNode(std::vector<shared_ptr<Object>> &objects, size_t start, size_t end) {

            aabb = AABB::empty;
            for (size_t obj_index = start; obj_index < end; obj_index++)
                aabb = AABB(aabb, objects[obj_index]->get_AABB());
            
            size_t object_span = end - start;

            if (object_span == 1) {
                left = right = objects[start];
            } else if (object_span == 2) {
                left = objects[start];
                right = objects[start+1];
            } else {
                int axis = aabb.longest_axis();
                auto comparator = (axis == 0) ? compare_AABB_x
                                : (axis == 1) ? compare_AABB_y
                                              : compare_AABB_z;
                std::sort(std::begin(objects) + start, std::begin(objects) + end, comparator);

                auto middle = start + object_span/2;
                left = make_shared<BVHNode>(objects, start, middle);
                right = make_shared<BVHNode>(objects, middle, end);
            }
        }

        bool intersect(const Ray &ri, Interval t_interval, Intersection &isect) const override {
            if (!aabb.intersectP(ri, t_interval)) return false;

            // isect stores the closest intersection between ray & {left, right}.
            bool hit_left = left->intersect(ri, t_interval, isect);
            bool hit_right = right->intersect(ri, Interval(t_interval.min, hit_left ? isect.distance : t_interval.max), isect);

            return hit_left || hit_right;
        }

        inline AABB get_AABB() const override { return aabb; }

    private:
        shared_ptr<Object> left;
        shared_ptr<Object> right;
        AABB aabb;

        static bool compare_AABB_pos(const shared_ptr<Object> &a, const shared_ptr<Object> &b,
                                     int axis_index) 
        {
            double a_AABB_pos = a->get_AABB().Centriod()[axis_index];
            double b_AABB_pos = b->get_AABB().Centriod()[axis_index];
            return a_AABB_pos < b_AABB_pos;
        }

        static bool compare_AABB_x(const shared_ptr<Object> &a, const shared_ptr<Object> &b) {
            return compare_AABB_pos(a, b, 0);
        }

        static bool compare_AABB_y(const shared_ptr<Object> &a, const shared_ptr<Object> &b) {
            return compare_AABB_pos(a, b, 1);
        }

        static bool compare_AABB_z(const shared_ptr<Object> &a, const shared_ptr<Object> &b) {
            return compare_AABB_pos(a, b, 2);
        }
};

#endif