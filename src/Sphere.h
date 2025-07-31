#ifndef SPHERE_H
#define SPHERE_H

#include "Object.h"

class Sphere : public Object {
    public:
        // static sphere
        Sphere(const Point3d &static_center, double radius, shared_ptr<Material> m)
          : center(static_center, Vector3d(0,0,0)), radius(std::fmax(0,radius)), m(m) 
        {
            auto rVec = Vector3d(radius, radius, radius);
            aabb = AABB(static_center - rVec, static_center + rVec);
        }

        // moving sphere
        Sphere(const Point3d &center1, const Point3d &center2, double radius,
               shared_ptr<Material> m)
          : center(center1, center2 - center1), radius(std::fmax(0,radius)), m(m)
        {
            auto rVec = Vector3d(radius, radius, radius);
            AABB aabb1 = AABB(center1 - rVec, center1 + rVec);
            AABB aabb2 = AABB(center2 - rVec, center2 + rVec);
            aabb = AABB(aabb1, aabb2);
        }

        bool intersect(const Ray &ri, Interval t_interval, Intersection &isect) const override {
            Point3d current_center = center.at(ri.time());
            Vector3d d = ri.direction(), oc = current_center - ri.origin();
            auto a = d.norm_squared();
            auto h = dotProduct(d, oc);
            auto c = oc.norm_squared() - radius*radius;

            auto discriminant = h*h - a*c;
            if (discriminant < 0) {
                return false; 
            } 

            auto sqrtd = std::sqrt(discriminant);
            auto t = (h - sqrtd) / a;
            if (!t_interval.surrounds(t)) {
                t = (h + sqrtd) / a;
                if (!t_interval.surrounds(t)) {
                    return false;
                }
            }

            isect.p = ri.at(t);
            isect.distance = t;
            auto outward_normal = (isect.p - current_center) / radius;
            isect.set_normal(ri, outward_normal);
            get_tex_uv(outward_normal, isect.tex_u, isect.tex_v);
            isect.m = m;

            return true;
        }

        inline AABB get_AABB() const override { return aabb; }

        inline double get_area() const override { 
            return 4 * pi * radius * radius;
        }

        inline void sample(Intersection &pos) const override {
            Point3d current_center = center.at(sample_double());
            pos.p = current_center + radius * sample_dir();
            pos.normal = (pos.p - current_center) / radius;
        }
        
        inline bool has_emission() const override {
            return m->has_emission();
        }
    
    private:
        Ray center; // allows center to move from start (t = 0) to end (t = 1).
        double radius;
        shared_ptr<Material> m;
        AABB aabb;

        static void get_tex_uv(const Point3d &p, double &u, double &v) {
            // p: a given point on the unit sphere centered at (0, 0, 0).
            // u: returned value [0,1] of angle around the Y axis from X=-1.
            // v: returned value [0,1] of angle from Y=-1 to Y=+1.
            //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
            //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
            //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

            double theta = std::acos(-p.y());
            double phi = std::atan2(-p.z(), p.x()) + pi;

            u = phi / (2*pi);
            v = theta / pi;
        }
};

#endif