#include <RayTracer/render.h>
#include <RayTracer/utility.h>
#include <RayTracer/hittables/sphere.h>
#include <RayTracer/color.h>
#include <RayTracer/common.h>
#include <omp.h> // TODO : READ https://pvs-studio.com/en/blog/posts/cpp/a0054/

render::render() 
{
}

void render::init() 
{
    auto R = cos(pi/4);

    auto material_left  = make_shared<lambertian>(color(0,0,1));
    auto material_right = make_shared<lambertian>(color(1,0,0));

    world.add(make_shared<sphere>(location(-R, 0, -1), R, material_left));
    world.add(make_shared<sphere>(location( R, 0, -1), R, material_right));

    // Write to a file
    freopen("output.ppm","w",stdout);

    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
}

void render::process() 
{
    for (int j = image_height-1; j >= 0; --j) 
    {
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
        for (int i = 0; i < image_width; ++i) 
        {
            color pixel_color(0, 0, 0);
            #pragma omp parallel for
            for (int s =0 ; s < samples_per_pixel ; s++)
            {
                // Set up UV
                double u = double(i + random_double()) / (image_width - 1.);
                double v = double(j + random_double()) / (image_height - 1.);
                
                ray r = cam.get_ray(u, v);
                color result_pixel_color = ray_color(r, world, max_depth);
                #pragma omp critical
                {
                    pixel_color += result_pixel_color;
                }
            }
            write_color_ppm(std::cout, pixel_color, samples_per_pixel);
        }
    }
    std::cerr << "\nDone.\n";
}

color render::ray_color(const ray& r, const hittable& world, const int depth) 
{
    if (depth <= 0)
    {
        return color(.0, .0, .0);
    }

    hit_record rec;
    if (world.hit(r, 0.001, infinity, rec))
    {
        ray scattered;
        color attenuation;
        if(rec.mat_ptr->scatter(r, rec, attenuation, scattered))
        {
            return attenuation * ray_color(scattered, world, depth-1);
        }
        else 
        {
            return color(0.);
        }
    }

    vec4 unit_direction = unit_vector(r.direction());
    auto t  = .5 * (unit_direction.y() + 1.);
    return (1.-t) * color(1., 1., 1.) + t * color(.5, .7, 1.);
}