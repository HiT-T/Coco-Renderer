#ifndef OBJECT_H
#define OBJECT_H

#include "AABB.h"

class Material; // pre-defining solves circularity of references between Object & Material classes.

// this stores the intersection information between ray & objects.
class Intersection {
    public:
        Point3d p;
        Vector3d normal;
        shared_ptr<Material> m; // material of hitted object.
        double tex_u, tex_v; // texture (u,v) coordinate of hitted object at p.
        double distance; // which is t (t>=0).
        bool happend_outside; // if ray-object intersection happens at object's outer surface.

        // this guarantees normal always points agianst the ray.
        void set_normal(const Ray &ri, const Vector3d &outward_normal) {
            happend_outside = dotProduct(ri.direction(), outward_normal) < 0.0;
            normal = happend_outside ? outward_normal : -outward_normal;
        }
};

class Object {
    // parent class defaultly define unused virtual functions, let for child classes to override.
    public:
        virtual ~Object() = default;

        // stores result both in return value (if intersect) & isect (intersect data).
        virtual bool intersect(const Ray& ri, Interval t_interval, Intersection& isect) const = 0;
        virtual AABB get_AABB() const = 0;
        virtual double get_area() const { return .0; };
        virtual void sample(Intersection &pos) const { return; };
        virtual bool has_emission() const { return false; }; 
};

class Translate : public Object {
    public:
        Translate(shared_ptr<Object> obj, Vector3d offset)
          : obj(obj), offset(offset) 
        {
            aabb = obj->get_AABB() + offset;
        }

        bool intersect(const Ray& ri, Interval t_interval, Intersection& isect) 
        const override {
            Ray offset_ri(ri.origin() - offset, ri.direction(), ri.time());
            if(!obj->intersect(offset_ri, t_interval, isect))
                return false;
            
            isect.p += offset;

            return true;
        }

        inline AABB get_AABB() const override { return aabb; }
        inline double get_area() const override { return obj->get_area(); }
        inline void sample(Intersection &pos) const override { obj->sample(pos); }
        inline bool has_emission() const override { return obj->has_emission(); }

    private:
        shared_ptr<Object> obj;
        Vector3d offset;
        AABB aabb;
};

class RotateY : public Object {
    public:
        RotateY(shared_ptr<Object> obj, double angle) : obj(obj) {
            auto theta = degrees_to_radians(angle);
            cos_theta = std::cos(theta);
            sin_theta = std::sin(theta);
            aabb = obj->get_AABB();

            Point3d min( infinity,  infinity,  infinity);
            Point3d max(-infinity, -infinity, -infinity);
            
            auto x_min = aabb.x.min, x_max = aabb.x.max;
            auto y_min = aabb.y.min, y_max = aabb.y.max;
            auto z_min = aabb.z.min, z_max = aabb.z.max;

            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        auto x = i * x_max + (1-i) * x_min;
                        auto y = i * y_max + (1-i) * y_min;
                        auto z = i * z_max + (1-i) * z_min;

                        auto rotated_x = ( cos_theta * x) + (sin_theta * z);
                        auto rotated_z = (-sin_theta * x) + (cos_theta * z);

                        Vector3d v(rotated_x, y, rotated_z);

                        for (int dim = 0; dim < 3; dim++) {
                            min[dim] = std::fmin(min[dim], v[dim]);
                            max[dim] = std::fmax(max[dim], v[dim]);
                        }
                    }
                }
            }

            aabb = AABB(min, max);
        }

        bool intersect(const Ray& ri, Interval t_interval, Intersection& isect) 
        const override {
            auto rotated_orig = Point3d(
                (cos_theta * ri.origin().x()) - (sin_theta * ri.origin().z()),
                ri.origin().y(),
                (sin_theta * ri.origin().x()) + (cos_theta * ri.origin().z())
            );

            auto rotated_dir = Vector3d(
                (cos_theta * ri.direction().x()) - (sin_theta * ri.direction().z()),
                ri.direction().y(),
                (sin_theta * ri.direction().x()) + (cos_theta * ri.direction().z())
            );

            Ray rotated_ri(rotated_orig, rotated_dir, ri.time());
            if(!obj->intersect(rotated_ri, t_interval, isect))
                return false;

            isect.p = Point3d(
                (cos_theta * isect.p.x()) + (sin_theta * isect.p.z()),
                isect.p.y(),
                (-sin_theta * isect.p.x()) + (cos_theta * isect.p.z())
            );

            isect.normal = Vector3d(
                (cos_theta * isect.normal.x()) + (sin_theta * isect.normal.z()),
                isect.normal.y(),
                (-sin_theta * isect.normal.x()) + (cos_theta * isect.normal.z())
            );

            return true;
        }

        inline AABB get_AABB() const override { return aabb; }
        inline double get_area() const override { return obj->get_area(); }
        inline void sample(Intersection &pos) const override { obj->sample(pos); }
        inline bool has_emission() const override { return obj->has_emission(); }

    private:
        shared_ptr<Object> obj;
        double cos_theta, sin_theta;
        AABB aabb;
};

#endif