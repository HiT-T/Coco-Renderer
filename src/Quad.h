#ifndef QUAD_H
#define QUAD_H

#include "Object.h"
#include "Material.h"
#include "Scene.h"

class Quad : public Object {
    public:
        Quad(const Point3d &Q, const Vector3d &u, const Vector3d &v, shared_ptr<Material> m)
          : Q(Q), u(u), v(v), m(m)
        {
            Vector3d n = crossProduct(u, v);
            normal = normalize(n);
            D = dotProduct(Q, normal);
            w = n / dotProduct(n, n);

            set_AABB();
        }

        bool intersect(const Ray &ri, Interval t_interval, Intersection &isect) const override {

            double denom = dotProduct(ri.direction(), normal);
            if (std::fabs(denom) < 1e-8) 
                return false; 
            
            double t = (D - dotProduct(ri.origin(), normal)) / denom;
            Point3d P = ri.at(t);
            Vector3d p = P - Q;
            double alpha = dotProduct(w, crossProduct(p, v));
            double beta = dotProduct(w, crossProduct(u, p));
            if (!t_interval.contains(t) || !inside_quad(alpha, beta))
                return false;

            isect.p = P;
            isect.distance = t;
            isect.set_normal(ri, normal);
            isect.tex_u = alpha;
            isect.tex_v = beta;
            isect.m = m;
            if (m->has_emission()) {
                isect.emission = m->emit(isect.tex_u, isect.tex_v, isect.p);
            }

            return true;
        }

        void set_AABB() {
            // Compute the bounding box of all four vertices.
            auto aabb_diagonal1 = AABB(Q, Q+u+v);
            auto aabb_diagonal2 = AABB(Q+u, Q+v);
            aabb = AABB(aabb_diagonal1, aabb_diagonal2);
        }

        inline AABB get_AABB() const override { return aabb; }

        inline double get_area() const override { 
            return crossProduct(u, v).norm();
        }

        inline void sample(Intersection &pos) const override {
            pos.p = Q + sample_double() * u + sample_double() * v;
            pos.normal = normal;
        }
        
        inline bool has_emission() const override {
            return m->has_emission();
        }
    
    private:
        Point3d Q; // quad's left-bottom vertice.
        Vector3d u, v; // two edge vectors from Q.
        Vector3d w;
        shared_ptr<Material> m;
        AABB aabb;
        Vector3d normal; // quad's normal.
        double D; // the D for quad's implicit fomula: ax + by + cz = D.

        bool inside_quad(double alpha, double beta) const {
            Interval unit_interval = Interval(0, 1);
            if (!unit_interval.contains(alpha) || !unit_interval.contains(beta))
                return false;

            return true;
        }
};

// returns the 3D box (6 sides) that contains the two opposite vertices p1 & p2.
inline shared_ptr<Scene> box(const Point3d &p1, const Point3d &p2, shared_ptr<Material> m) {
    auto x = p1.x() < p2.x() ? Interval(p1.x(), p2.x()) : Interval(p2.x(), p1.x());
    auto y = p1.y() < p2.y() ? Interval(p1.y(), p2.y()) : Interval(p2.y(), p1.y());
    auto z = p1.z() < p2.z() ? Interval(p1.z(), p2.z()) : Interval(p2.z(), p1.z());

    auto dx = Vector3d(x.max - x.min, 0, 0);
    auto dy = Vector3d(0, y.max - y.min, 0);
    auto dz = Vector3d(0, 0, z.max - z.min);

    auto sides = make_shared<Scene>();

    sides->add(make_shared<Quad>(Point3d(x.min, y.min, z.max),  dx,  dy, m)); // front
    sides->add(make_shared<Quad>(Point3d(x.max, y.min, z.max), -dz,  dy, m)); // right
    sides->add(make_shared<Quad>(Point3d(x.max, y.min, z.min), -dx,  dy, m)); // back
    sides->add(make_shared<Quad>(Point3d(x.min, y.min, z.min),  dz,  dy, m)); // left
    sides->add(make_shared<Quad>(Point3d(x.min, y.max, z.max),  dx, -dz, m)); // top
    sides->add(make_shared<Quad>(Point3d(x.min, y.min, z.min),  dx,  dz, m)); // bottom

    sides->buildBVH();

    return sides;
}

#endif