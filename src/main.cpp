#include "global.h"

#include "Object.h"
#include "Sphere.h"
#include "Quad.h"
#include "ConstantMedium.h"
#include "BVH.h"
#include "Texture.h"
#include "Material.h"
#include "Scene.h"
#include "Renderer.h"

Color sky_color = Color(0.70, 0.80, 1.00);

void metal_spheres() {

    Scene scene(400, 16.0 / 9.0, sky_color);

    auto material_ground = make_shared<Diffuse>(Color(0.8, 0.8, 0.0));
    auto material_center = make_shared<Metal>(Color(0.1, 0.2, 0.5), 0.3);
    auto material_left   = make_shared<Metal>(Color(0.8, 0.8, 0.8), 0.0);
    auto material_right  = make_shared<Metal>(Color(0.8, 0.6, 0.2), 1.0);

    scene.add(make_shared<Sphere>(Point3d( 0.0, -100.5, -1.0), 100.0, material_ground));
    scene.add(make_shared<Sphere>(Point3d( 0.0,    0.0, -1.2),   0.5, material_center));
    scene.add(make_shared<Sphere>(Point3d(-1.0,    0.0, -1.0),   0.5, material_left));
    scene.add(make_shared<Sphere>(Point3d( 1.0,    0.0, -1.0),   0.5, material_right));

    scene.buildBVH();

    scene.vfov      = 90;
    scene.eye_pos   = Point3d(0,0,0);
    scene.gaze_pos  = Point3d(0,0,-1);
    scene.up_dir    = Vector3d(0,1,0);

    scene.defocus_angle = 0;

    Renderer r;
    r.spp = 100;

    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

void bouncing_spheres() {
    
    // create the scene with image size params.
    Scene scene(400, 16.0 / 9.0, sky_color);

    // define materials.
    auto checker_texture = make_shared<CheckerTexture>(0.32, Color(.2, .3, .1), Color(.9, .9, .9));
    scene.add(make_shared<Sphere>(Point3d(0,-1000,0), 1000, make_shared<Diffuse>(checker_texture)));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_m = sample_double();
            Point3d center(a + 0.9*sample_double(), 0.2, b + 0.9*sample_double());

            if ((center - Point3d(4, 0.2, 0)).norm() > 0.9) {
                shared_ptr<Material> sphere_material;

                if (choose_m < 0.8) {
                    // diffuse
                    auto albedo = Color::sample() * Color::sample();
                    sphere_material = make_shared<Diffuse>(albedo);
                    auto center2 = center + Vector3d(0, sample_double(0,.5), 0);
                    scene.add(make_shared<Sphere>(center, center2, 0.2, sphere_material));
                } else if (choose_m < 0.95) {
                    // metal
                    auto albedo = Color::sample(0.5, 1);
                    auto fuzz = sample_double(0, 0.5);
                    sphere_material = make_shared<Metal>(albedo, fuzz);
                    scene.add(make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<Dielectric>(1.5);
                    scene.add(make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<Dielectric>(1.5);
    scene.add(make_shared<Sphere>(Point3d(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<Diffuse>(Color(0.4, 0.2, 0.1));
    scene.add(make_shared<Sphere>(Point3d(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
    scene.add(make_shared<Sphere>(Point3d(4, 1, 0), 1.0, material3));

    // build BVH for added objects.
    scene.buildBVH();

    // define camera params.
    scene.vfov     = 20;
    scene.eye_pos  = Point3d(13, 2, 3);
    scene.gaze_pos = Point3d(0, 0, 0);
    scene.up_dir   = Vector3d(0, 1, 0);

    scene.defocus_angle = 0.6;
    scene.focal_dist    = 10.0;

    // define renderer and image size.
    Renderer r;
    r.spp = 100;

    // render the image.
    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

void checkered_spheres() {

    Scene scene(400, 16.0 / 9.0, sky_color);

    auto checker_texture = make_shared<CheckerTexture>(0.32, Color(.2, .3, .1), Color(.9, .9, .9));

    scene.add(make_shared<Sphere>(Point3d(0,-10, 0), 10, make_shared<Diffuse>(checker_texture)));
    scene.add(make_shared<Sphere>(Point3d(0, 10, 0), 10, make_shared<Diffuse>(checker_texture)));

    scene.buildBVH();

    scene.vfov      = 20;
    scene.eye_pos   = Point3d(13,2,3);
    scene.gaze_pos  = Point3d(0,0,0);
    scene.up_dir    = Vector3d(0,1,0);

    scene.defocus_angle = 0;

    Renderer r;
    r.spp = 256;

    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

void earth() {

    Scene scene(400, 16.0 / 9.0, sky_color);

    auto earth_texture = make_shared<ImageTexture>("earthmap.jpg");
    auto earth_material = make_shared<Diffuse>(earth_texture);
    scene.add(make_shared<Sphere>(Point3d(0,0,0), 2, earth_material));

    scene.buildBVH();

    scene.vfov      = 20;
    scene.eye_pos   = Point3d(0,0,12);
    scene.gaze_pos  = Point3d(0,0,0);
    scene.up_dir    = Vector3d(0,1,0);

    scene.defocus_angle = 0;

    Renderer r;
    r.spp = 100;

    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

void perlin_spheres() {

    Scene scene(400, 16.0 / 9.0, sky_color);

    auto perlin_texture = make_shared<NoiseTexture>(4);
    scene.add(make_shared<Sphere>(Point3d(0,-1000,0), 1000, make_shared<Diffuse>(perlin_texture)));
    scene.add(make_shared<Sphere>(Point3d(0,2,0), 2, make_shared<Diffuse>(perlin_texture)));

    scene.buildBVH();

    scene.vfov      = 20;
    scene.eye_pos   = Point3d(13,2,3);
    scene.gaze_pos  = Point3d(0,0,0);
    scene.up_dir    = Vector3d(0,1,0);

    scene.defocus_angle = 0;

    Renderer r;
    r.spp = 100;

    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

void Quads() {

    Scene scene(400, 16.0 / 9.0, sky_color);

    // Materials
    auto left_red     = make_shared<Diffuse>(Color(1.0, 0.2, 0.2));
    auto back_green   = make_shared<Diffuse>(Color(0.2, 1.0, 0.2));
    auto right_blue   = make_shared<Diffuse>(Color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<Diffuse>(Color(1.0, 0.5, 0.0));
    auto lower_teal   = make_shared<Diffuse>(Color(0.2, 0.8, 0.8));

    // Quads
    scene.add(make_shared<Quad>(Point3d(-3,-2, 5), Vector3d(0, 0,-4), Vector3d(0, 4, 0), left_red));
    scene.add(make_shared<Quad>(Point3d(-2,-2, 0), Vector3d(4, 0, 0), Vector3d(0, 4, 0), back_green));
    scene.add(make_shared<Quad>(Point3d( 3,-2, 1), Vector3d(0, 0, 4), Vector3d(0, 4, 0), right_blue));
    scene.add(make_shared<Quad>(Point3d(-2, 3, 1), Vector3d(4, 0, 0), Vector3d(0, 0, 4), upper_orange));
    scene.add(make_shared<Quad>(Point3d(-2,-3, 5), Vector3d(4, 0, 0), Vector3d(0, 0,-4), lower_teal));

    scene.buildBVH();

    scene.vfov      = 80;
    scene.eye_pos   = Point3d(0,0,9);
    scene.gaze_pos  = Point3d(0,0,0);
    scene.up_dir    = Vector3d(0,1,0);

    scene.defocus_angle = 0;

    Renderer r;
    r.spp = 100;

    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

void simple_light() {

    Scene scene(400, 16.0 / 9.0, Color());

    auto perlin_texture = make_shared<NoiseTexture>(4);
    scene.add(make_shared<Sphere>(Point3d(0,-1000,0), 1000, make_shared<Diffuse>(perlin_texture)));
    scene.add(make_shared<Sphere>(Point3d(0,2,0), 2, make_shared<Diffuse>(perlin_texture)));

    auto diffuse_light = make_shared<DiffuseLight>(Color(4,4,4));
    scene.add(make_shared<Sphere>(Point3d(0,7,0), 2, diffuse_light));
    scene.add(make_shared<Quad>(Point3d(3,1,-2), Vector3d(2,0,0), Vector3d(0,2,0), diffuse_light));

    scene.buildBVH();

    scene.vfov      = 20;
    scene.eye_pos   = Point3d(26,3,6);
    scene.gaze_pos  = Point3d(0,2,0);
    scene.up_dir    = Vector3d(0,1,0);

    scene.defocus_angle = 0;

    Renderer r;
    r.spp = 100;

    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

void cornell_box_1() {
    Scene scene(600, 1.0, Color());

    auto red   = make_shared<Diffuse>(Color(.65, .05, .05));
    auto white = make_shared<Diffuse>(Color(.73, .73, .73));
    auto green = make_shared<Diffuse>(Color(.12, .45, .15));
    auto light = make_shared<DiffuseLight>(Color(15, 15, 15));

    scene.add(make_shared<Quad>(Point3d(555,0,0), Vector3d(0,555,0), Vector3d(0,0,555), green));
    scene.add(make_shared<Quad>(Point3d(0,0,0), Vector3d(0,555,0), Vector3d(0,0,555), red));
    scene.add(make_shared<Quad>(Point3d(343, 554, 332), Vector3d(-130,0,0), Vector3d(0,0,-105), light));
    scene.add(make_shared<Quad>(Point3d(0,0,0), Vector3d(555,0,0), Vector3d(0,0,555), white));
    scene.add(make_shared<Quad>(Point3d(555,555,555), Vector3d(-555,0,0), Vector3d(0,0,-555), white));
    scene.add(make_shared<Quad>(Point3d(0,0,555), Vector3d(555,0,0), Vector3d(0,555,0), white));

    shared_ptr<Object> box1 = box(Point3d(0,0,0), Point3d(165,330,165), white);
    box1 = make_shared<RotateY>(box1, 15);
    box1 = make_shared<Translate>(box1, Vector3d(265,0,295));
    scene.add(box1);

    shared_ptr<Object> box2 = box(Point3d(0,0,0), Point3d(165,165,165), white);
    box2 = make_shared<RotateY>(box2, -18);
    box2 = make_shared<Translate>(box2, Vector3d(130,0,65));
    scene.add(box2);

    scene.buildBVH();

    scene.vfov      = 40;
    scene.eye_pos   = Point3d(278, 278, -800);
    scene.gaze_pos  = Point3d(278, 278, 0);
    scene.up_dir    = Vector3d(0,1,0);

    scene.defocus_angle = 0;

    Renderer r;
    r.spp = 200;

    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

void cornell_smoke() {
    Scene scene(600, 1.0, Color());

    auto red   = make_shared<Diffuse>(Color(.65, .05, .05));
    auto white = make_shared<Diffuse>(Color(.73, .73, .73));
    auto green = make_shared<Diffuse>(Color(.12, .45, .15));
    auto light = make_shared<DiffuseLight>(Color(7, 7, 7));

    scene.add(make_shared<Quad>(Point3d(555,0,0), Vector3d(0,555,0), Vector3d(0,0,555), green));
    scene.add(make_shared<Quad>(Point3d(0,0,0), Vector3d(0,555,0), Vector3d(0,0,555), red));
    scene.add(make_shared<Quad>(Point3d(113,554,127), Vector3d(330,0,0), Vector3d(0,0,305), light));
    scene.add(make_shared<Quad>(Point3d(0,555,0), Vector3d(555,0,0), Vector3d(0,0,555), white));
    scene.add(make_shared<Quad>(Point3d(0,0,0), Vector3d(555,0,0), Vector3d(0,0,555), white));
    scene.add(make_shared<Quad>(Point3d(0,0,555), Vector3d(555,0,0), Vector3d(0,555,0), white));

    shared_ptr<Object> box1 = box(Point3d(0,0,0), Point3d(165,330,165), white);
    box1 = make_shared<RotateY>(box1, 15);
    box1 = make_shared<Translate>(box1, Vector3d(265,0,295));

    shared_ptr<Object> box2 = box(Point3d(0,0,0), Point3d(165,165,165), white);
    box2 = make_shared<RotateY>(box2, -18);
    box2 = make_shared<Translate>(box2, Vector3d(130,0,65));

    scene.add(make_shared<ConstantMedium>(box1, 0.01, Color(0,0,0)));
    scene.add(make_shared<ConstantMedium>(box2, 0.01, Color(1,1,1)));

    scene.buildBVH();

    scene.vfov      = 40;
    scene.eye_pos   = Point3d(278, 278, -800);
    scene.gaze_pos  = Point3d(278, 278, 0);
    scene.up_dir    = Vector3d(0,1,0);

    scene.defocus_angle = 0;

    Renderer r;
    r.spp = 200;

    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

void RTNW(int image_width, int spp) {

    // test quad & box.
    Scene boxes1;
    auto ground = make_shared<Diffuse>(Color(0.48, 0.83, 0.53));

    int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = sample_double(1,101);
            auto z1 = z0 + w;

            boxes1.add(box(Point3d(x0,y0,z0), Point3d(x1,y1,z1), ground));
        }
    }

    boxes1.buildBVH();

    Scene scene(image_width, 1.0, Color());

    scene.add(make_shared<Scene>(boxes1));

    // test light.
    auto light = make_shared<DiffuseLight>(Color(7, 7, 7));
    scene.add(make_shared<Quad>(Point3d(123,554,147), Vector3d(300,0,0), Vector3d(0,0,265), light));

    // test motion blur.
    auto center1 = Point3d(400, 400, 200);
    auto center2 = center1 + Vector3d(30,0,0);
    auto sphere_material = make_shared<Diffuse>(Color(0.7, 0.3, 0.1));
    scene.add(make_shared<Sphere>(center1, center2, 50, sphere_material));

    // test dielectric & metal.
    scene.add(make_shared<Sphere>(Point3d(260, 150, 45), 50, make_shared<Dielectric>(1.5)));
    scene.add(make_shared<Sphere>(
        Point3d(0, 150, 145), 50, make_shared<Metal>(Color(0.8, 0.8, 0.9), 1.0)
    ));

    // test volume.
    auto boundary = make_shared<Sphere>(Point3d(360,150,145), 70, make_shared<Dielectric>(1.5));
    scene.add(boundary);
    scene.add(make_shared<ConstantMedium>(boundary, 0.2, Color(0.2, 0.4, 0.9)));
    
    boundary = make_shared<Sphere>(Point3d(0,0,0), 5000, make_shared<Dielectric>(1.5));
    scene.add(make_shared<ConstantMedium>(boundary, .0001, Color(1,1,1)));

    // test image texture.
    auto image_texture = make_shared<ImageTexture>("earthmap.jpg");
    scene.add(make_shared<Sphere>(Point3d(400,200,400), 100, make_shared<Diffuse>(image_texture)));

    // test perlin.
    auto perlin_texture = make_shared<NoiseTexture>(0.2);
    scene.add(make_shared<Sphere>(Point3d(220,280,300), 80, make_shared<Diffuse>(perlin_texture)));

    // test diffuse.
    Scene boxes2;
    auto white = make_shared<Diffuse>(Color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<Sphere>(Point3d::sample(0,165), 10, white));
    }

    boxes2.buildBVH();

    // test instance.
    scene.add(make_shared<Translate>(
        make_shared<RotateY>(
            make_shared<Scene>(boxes2), 15),
            Vector3d(-100,270,395)
        )
    );

    scene.buildBVH();

    scene.vfov      = 40;
    scene.eye_pos   = Point3d(478, 278, -600);
    scene.gaze_pos  = Point3d(278, 278, 0);
    scene.up_dir    = Vector3d(0,1,0);

    scene.defocus_angle = 0;

    Renderer r;
    r.spp = spp;

    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

void cornell_box_2() {
    Scene scene(600, 1.0, Color());

    auto red   = make_shared<Metal>(Color(.65, .05, .05), 0.0);
    auto white = make_shared<Diffuse>(Color(.73, .73, .73));
    auto green = make_shared<Metal>(Color(.12, .45, .15), 0.0);
    auto light = make_shared<DiffuseLight>(Color(15, 15, 15));

    scene.add(make_shared<Quad>(Point3d(555,0,0), Vector3d(0,555,0), Vector3d(0,0,555), green));
    scene.add(make_shared<Quad>(Point3d(0,0,0), Vector3d(0,555,0), Vector3d(0,0,555), red));
    scene.add(make_shared<Quad>(Point3d(343, 554, 332), Vector3d(-130,0,0), Vector3d(0,0,-105), light));
    scene.add(make_shared<Quad>(Point3d(0,0,0), Vector3d(555,0,0), Vector3d(0,0,555), white));
    scene.add(make_shared<Quad>(Point3d(555,555,555), Vector3d(-555,0,0), Vector3d(0,0,-555), white));
    scene.add(make_shared<Quad>(Point3d(0,0,555), Vector3d(555,0,0), Vector3d(0,555,0), white));

    shared_ptr<Object> box1 = box(Point3d(0,0,0), Point3d(165,330,165), white);
    box1 = make_shared<RotateY>(box1, 15);
    box1 = make_shared<Translate>(box1, Vector3d(265,0,295));
    scene.add(box1);

    scene.add(make_shared<Sphere>(Point3d(190,90,190), 90, make_shared<Dielectric>(1.5)));

    scene.buildBVH();

    scene.vfov      = 40;
    scene.eye_pos   = Point3d(278, 278, -800);
    scene.gaze_pos  = Point3d(278, 278, 0);
    scene.up_dir    = Vector3d(0,1,0);

    scene.defocus_angle = 0;

    Renderer r;
    r.spp = 64;

    auto start = std::chrono::system_clock::now();
    r.render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "\nDone!\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << "h";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() % 60 << "min";
    std::cout << " : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() % 60 << "s\n";
}

int main() {
    int scene_index = 8;
    switch(scene_index) {
        case 1:   metal_spheres();      break;
        case 2:   bouncing_spheres();   break;
        case 3:   checkered_spheres();  break;
        case 4:   earth();              break;
        case 5:   perlin_spheres();     break;
        case 6:   Quads();              break;
        case 7:   simple_light();       break;
        case 8:   cornell_box_1();      break;
        case 9:   cornell_smoke();      break;
        case 10:  RTNW(800, 10240);     break;
        case 11:  RTNW(400,   128);     break;
        case 12:  cornell_box_2();      break;
    }
}