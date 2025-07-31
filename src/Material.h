#ifndef MATERIAL_H
#define MATERIAL_H

#include "Object.h"
#include "Texture.h"

class Material {
    public:
        virtual ~Material() = default;

        virtual bool has_emission() const { return false; }
        virtual Color emit(double u, double v, const Vector3d &p) const { return Color(); };

        virtual bool scatter(const Ray &ri, const Intersection &isect, Color &attenuation, Ray &ro)
        const { return false; }
};

class Diffuse : public Material {
    public:
        Diffuse(const Color &albedo) : tex(make_shared<SolidColorTexture>(albedo)) {}

        Diffuse(shared_ptr<Texture> tex) : tex(tex) {}

        bool scatter(const Ray &ri, const Intersection &isect, Color &attenuation, Ray &ro)
        const override {
            Vector3d wo = isect.normal + sample_dir();
            if (wo.near_zero()) {
                wo = isect.normal;
            } else {
                wo = normalize(wo);
            }
            ro = Ray(isect.p, wo, ri.time());
            attenuation = tex->get_texColor(isect.tex_u, isect.tex_v, isect.p);
            return true;
        }
    
    private:
        shared_ptr<Texture> tex;  
};

inline Vector3d reflect(const Vector3d &wi, const Vector3d &N) {
    return normalize(wi - 2 * dotProduct(wi, N) * N);
}

inline Vector3d refract(const Vector3d &wi, const Vector3d &N, double refraction_index) {
    auto cos_i = std::fmin(dotProduct(-wi, N) , 1.0);
    Vector3d wo_perp = refraction_index * (wi + cos_i * N);
    Vector3d wo_parallel = -std::sqrt(std::fabs(1 - wo_perp.norm_squared())) * N;
    return wo_perp + wo_parallel;
}

class Metal : public Material {
    public:
        Metal(const Color &albedo, double fuzz) : albedo(albedo), fuzz((fuzz < 1.0) ? fuzz : 1.0) {}

        bool scatter(const Ray &ri, const Intersection &isect, Color &attenuation, Ray &ro)
        const override {
            // fuzzy dir = specular reflection dir + random vector in fuzz unit sphere .
            Vector3d wo = reflect(ri.direction(), isect.normal) + fuzz * sample_dir();
            ro = Ray(isect.p, normalize(wo), ri.time());
            attenuation = albedo;

            // if fuzzing produces inward ray, treat it as absorbed by returning false.
            return (dotProduct(ro.direction(), isect.normal) > 0.0);
        }

    private:
        Color albedo;

        // defines the roughness of the metal's surface (1 >= fuzz >= 0).
        // note: (fuzz == 0) means specular, (fuzz == 1) means nearly diffuse.
        double fuzz;
};

class Dielectric : public Material {
    public:
        Dielectric(double ior) : ior(ior) {}

        bool scatter(const Ray &ri, const Intersection &isect, Color &attenuation, Ray &ro)
        const override {
            attenuation = Color(1.0, 1.0, 1.0);
            double refraction_index = isect.happend_outside ? (1.0 / ior) : ior; // defaultly treat outside as air.

            Vector3d wi = ri.direction(), N = isect.normal;
            double cos_i = std::fmin(dotProduct(-wi, N), 1.0);
            double sin_i = std::sqrt(1 - cos_i*cos_i);

            bool cannot_refract = (refraction_index * sin_i > 1.0) ? true : false;
            Vector3d wo;
            if (cannot_refract || sample_double() < fresnel(cos_i, refraction_index)) {
                wo = reflect(wi, N);
            } else {
                wo = refract(wi, N, refraction_index);
            }

            ro = Ray(isect.p, wo, ri.time());
            return true;
        }

    private:
        // could be absolute ior (index of refraction), or relative (material's ior over enclosing material's).
        double ior;

        // use Schick's approximation to calculate reflectance.
        static double fresnel(double cos_i, double reflection_index) {
            auto r0 = (1.0 - reflection_index) / (1.0 + reflection_index);
            r0 *= r0;
            return r0 + (1-r0) * std::pow((1 - cos_i), 5);
        }
};

class DiffuseLight : public Material {
    public:
        DiffuseLight(const Color &emit) : tex(make_shared<SolidColorTexture>(emit)) {}
        DiffuseLight(shared_ptr<Texture> tex) : tex(tex) {}

        inline bool has_emission() const override { return true; }

        Color emit(double u, double v, const Vector3d &p) const override {
            return tex->get_texColor(u, v, p);
        }

    private:
        shared_ptr<Texture> tex;
};

class Isotropic : public Material {
    public:
        Isotropic(const Color &albedo) : tex(make_shared<SolidColorTexture>(albedo)) {}
        Isotropic(shared_ptr<Texture> tex) : tex(tex) {}

        bool scatter(const Ray &ri, const Intersection &isect, Color &attenuation, Ray &ro)
        const override {
            ro = Ray(isect.p, sample_dir(), ri.time()); // ro could be generated anywhere on unit sphere.
            attenuation = tex->get_texColor(isect.tex_u, isect.tex_v, isect.p);
            return true;
        }

    private:
        shared_ptr<Texture> tex;
};

#endif