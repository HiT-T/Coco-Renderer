#ifndef TEXTURE_H
#define TEXTURE_H

#include "Perlin.h"
#include "Image.h"

class Texture {
    public:
        virtual ~Texture() = default;

        virtual Color get_texColor(double u, double v, const Vector3d &p) const = 0;
};

class SolidColorTexture : public Texture {
    public:
        SolidColorTexture(const Color &albedo) : albedo(albedo) {}

        SolidColorTexture(double r, double g, double b) : albedo(Color(r,g,b)) {}

        Color get_texColor(double u, double v, const Vector3d &p) const override {
            return albedo;
        }

    private:
        Color albedo;
};

// a checker solid texture.
class CheckerTexture : public Texture {
    public:
        CheckerTexture(double scale, shared_ptr<Texture> odd, shared_ptr<Texture> even)
          : invScale(1.0 / scale), odd(odd), even(even) {}

        CheckerTexture(double scale, const Color &c1, const Color &c2)
          : CheckerTexture(scale, make_shared<SolidColorTexture>(c1), make_shared<SolidColorTexture>(c2)) {}

        Color get_texColor(double u, double v, const Vector3d &p) const override {
            auto x_int = int(std::floor(invScale * p.x()));
            auto y_int = int(std::floor(invScale * p.y()));
            auto z_int = int(std::floor(invScale * p.z()));

            bool isEven = (x_int + y_int + z_int) % 2 == 0;

            return isEven ? even->get_texColor(u, v, p) : odd->get_texColor(u, v, p);
        }

    private:
        double invScale;
        shared_ptr<Texture> odd;
        shared_ptr<Texture> even;      
};

class ImageTexture : public Texture {
    public:
        ImageTexture(const char* filename) : image(filename) {}

        Color get_texColor(double u, double v, const Point3d& p) const override {

            // if we have no texture data, then return solid cyan as a debugging aid.
            if (image.height() <= 0) return Color(0,1,1);

            // clamp input texture coordinates to [0,1] x [1,0].
            u = Interval(0,1).clamp(u);
            v = 1.0 - Interval(0,1).clamp(v);  // flip V to image coordinates.

            auto i = int(u * image.width());
            auto j = int(v * image.height());
            auto pixel = image.get_pixel_data(i,j);

            auto color_scale = 1.0 / 255.0;
            return Color(color_scale*pixel[0], color_scale*pixel[1], color_scale*pixel[2]);
        }

    private:
        Image image;
};

class NoiseTexture : public Texture {
    public:
        NoiseTexture(double scale) : scale(scale) {}

        Color get_texColor(double u, double v, const Point3d& p) const override { 
            return Color(.5,.5,.5) * (1 + std::sin(scale * p.z() + 10 * perlin.turb(p, 7)));
        }
    private:
        Perlin perlin;
        double scale;
};

#endif