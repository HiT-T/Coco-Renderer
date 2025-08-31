#ifndef OBJECT_LIST_H
#define OBJECT_LIST_H

#include "AABB.h"
#include "BVH.h"
#include "Object.h"
#include "Material.h"

#include <vector>

class Scene : public Object {
    public:
        // scene component: (1) camera (2) objects

        int    image_w      = 100, image_h;
        double aspect_ratio = 1.0;
        double viewport_w   = 0.0, viewport_h = 0.0;
        Color bgColor; // background color.

        double   vfov     = 90;               // define vertical field of view.
        Point3d  eye_pos  = Point3d(0,0,0);   // camera's position
        Point3d  gaze_pos = Point3d(0,0,-1);  // point camera is gazing at
        Vector3d up_dir   = Vector3d(0,1,0);  // camera-relative "up" direction

        double defocus_angle = 0;  // angle of the cone with apex at viewport center and base (defocus disk) at eye_pos.
        double focal_dist = 10;    // distance from camera center & defocus disk to focal plane.

        Scene() {}
        Scene(int image_w, double aspect_ratio, Color bgColor)
          : image_w(image_w), aspect_ratio(aspect_ratio), bgColor(bgColor) {}

        void initialize_camera() {

            image_h = int(image_w / aspect_ratio);
            image_h = (image_h < 1) ? 1 : image_h;

            // define camera & viewport related params.
            auto h = std::tan(degrees_to_radians(vfov)/2);
            viewport_h = 2 * h * focal_dist, viewport_w = viewport_h * (double(image_w)/image_h);

            // calculate the u,v,w unit basis vectors for the camera coordinate frame.
            w = normalize(eye_pos - gaze_pos);
            u = normalize(crossProduct(up_dir, w));
            v = crossProduct(w, u);

            // calculate the vectors across the horizontal and down the vertical viewport edges.
            auto viewport_u = viewport_w * u;
            auto viewport_v = viewport_h * -v;

            // calculate the horizontal and vertical delta vectors between adjacent pixels.
            pixel_delta_u = viewport_u / image_w;
            pixel_delta_v = viewport_v / image_h;

            // calculate the location of the upper left pixel.
            pixel00_loc = eye_pos - (focal_dist * w) - viewport_u/2 - viewport_v/2;

            // calculate defocus disk's basis vectors.
            auto disk_radius = focal_dist * std::tan(degrees_to_radians(defocus_angle)/2);
            defocus_disk_u = u * disk_radius;
            defocus_disk_v = v * disk_radius;
        }

        // construct a camera ray originating from the defocus disk and directed at a randomly
        // sampled point around the pixel located at (i, j).
        Ray cast_ray(int i, int j) const {
            
            double dx = sample_double();
            double dy = sample_double();

            auto pixel_center = pixel00_loc 
                              + ((i + dx) * pixel_delta_u) 
                              + ((j + dy) * pixel_delta_v);

            auto ray_origin = (defocus_angle <= 0) ? eye_pos : sample_in_defocus_disk();
            auto ray_direction = normalize(pixel_center - ray_origin);
            auto ray_time = sample_double();

            return Ray(ray_origin, ray_direction, ray_time);
        }

        // shared_ptr ? 1. automatically frees memory; 2. allows multiple references.
        std::vector<shared_ptr<Object>> objects;
        shared_ptr<BVHNode> bvh;

        void clear() { objects.clear(); }

        inline AABB get_AABB() const override { return aabb; }

        void add(shared_ptr<Object> object) { 
            objects.push_back(object); 
            aabb = AABB(aabb, object->get_AABB());
            if (object->has_emission()) {
                emit_area_sum += object->get_area();
            }
        }

        void buildBVH() {
            this->bvh = make_shared<BVHNode>(objects);
        }

        bool intersect(const Ray &ri, Interval t_interval, Intersection& isect) const override {
            return this->bvh->intersect(ri, t_interval, isect);
        }

        void sample_light(Intersection &pos, double &pdf_light) const {
            double p = sample_double() * emit_area_sum;
            double temp_area_sum = 0;
            for (int i = 0; i < objects.size(); i++) {
                if (objects[i]->has_emission()) {
                    temp_area_sum += objects[i]->get_area();
                    if (temp_area_sum > p) {
                        objects[i]->sample(pos);
                        pdf_light = 1 / emit_area_sum;
                        break;
                    }
                }
            }
        }
    
    private:
        AABB aabb;
        double emit_area_sum;

        Vector3d   pixel_delta_u;   // offset to pixel to the right
        Vector3d   pixel_delta_v;   // offset to pixel below
        Point3d    pixel00_loc;     // location of pixel 0, 0
        Vector3d   u, v, w;         // camera frame basis vectors
        Vector3d   defocus_disk_u;  // defocus disk's horizontal basis vector
        Vector3d   defocus_disk_v;  // defocus disk's vertical basis vector

        // returns a random point in the camera defocus disk.
        Point3d sample_in_defocus_disk() const {
            auto p = sample_in_unit_disk();
            return eye_pos + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }
};

#endif
