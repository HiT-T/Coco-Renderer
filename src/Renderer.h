#ifndef RENDERER_H
#define RENDERER_H

#include "Object.h"
#include "Material.h"

class Renderer {
    public:
        int spp = 10; // count of samples per pixel.

        Renderer() {}

        void render(Scene &scene) {
            
            scene.initialize_camera();

            // calculate each pixel's RGB color value and store into image.
            
            FILE* fp = fopen("binary.ppm", "wb");
            (void)fprintf(fp, "P6\n%d %d\n255\n", scene.image_w, scene.image_h);

            std::cout << "SPP: " << spp << "\n";
            double pps = 1 / double(spp);

            for (auto j = 0; j < scene.image_h; j++) {
                for (auto i = 0; i < scene.image_w; i++) {
                    // compute color of the ray/pixel.
                    auto pixel_color = Color();
                    for (int s = 0; s < spp; s++) {
                        auto r = scene.cast_ray(i, j);
                        pixel_color += get_color(r, scene);
                    }

                    // write the computed pixel_color into image.
                    write_color(fp, pixel_color * pps);
                }
                UpdateProgress(j / double(scene.image_h));
            }
            UpdateProgress(1.);
    
            fclose(fp);
        }

        private:
            double RussianRoulette = 0.8;

            Color get_color(const Ray &ri, const Scene &scene) const {
                
                double epsilon = 5e-4;
                auto isect = Intersection();

                // if doesn't intersect or (t < .001) at all, return background color.
                // note: (t_min == epsilon (> 0)) avoids self-intersection caused by floating point rounding errors.
                if (!scene.intersect(ri, Interval(epsilon, infinity), isect)) {
                    return scene.bgColor;
                }

                // if hits a light source, just return object's emission.
                if (isect.m->has_emission()) {
                    return isect.m->emit(isect.tex_u, isect.tex_v, isect.p);
                }

                Color attenuation; Ray ro;
                if (!isect.m->scatter(ri, isect, attenuation, ro)) {
                    return Color();
                }

                Color Lo_dir;
                {
                    Intersection hit_light; double pdf_light;
                    scene.sample_light(hit_light, pdf_light);

                    Vector3d obj2light = hit_light.p - isect.p;
                    Ray shadow_ray(isect.p, normalize(obj2light));
                    hit_light.set_normal(shadow_ray, hit_light.normal);

                    Intersection isect_obj2light;
                    scene.intersect(shadow_ray, Interval(epsilon, infinity), isect_obj2light);

                    if (isect_obj2light.distance - obj2light.norm() > -epsilon) {
                        double cosine = dotProduct(hit_light.normal, -shadow_ray.direction());

                        Lo_dir = hit_light.emission
                               * attenuation 
                               * cosine
                               / (hit_light.p - isect.p).norm_squared() 
                               / pdf_light;
                    }
                }

                Color Lo_indir;
                {
                    // test RR to decide if continues bouncing.
                    if (sample_double() > RussianRoulette) { return Color(); }

                    Lo_indir = attenuation * get_color(ro, scene) / RussianRoulette;
                }

                return Lo_dir + Lo_indir;
            }
};

#endif