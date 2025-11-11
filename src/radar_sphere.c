#include <SDL2_gfxPrimitives.h>
#include <SDL2/SDL.h>
#include <radar.h>


/* Helper function to get pixel from an SDL_Surface (easier than locked texture) */
Uint32 get_pixel_from_surface(SDL_Surface* surface, int x, int y) {
    if (!surface || x < 0 || y < 0 || x >= surface->w || y >= surface->h) return 0;

    int bytes_per_pixel = surface->format->BytesPerPixel;
    /* Get the pixel data at the specified coordinates */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bytes_per_pixel;

    switch(bytes_per_pixel) {
        case 1:
            return *p;
        case 2:
            return *(Uint16 *)p;
        case 3:
            if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
        case 4:
            return *(Uint32 *)p;
        default:
            return 0;   /* Should not happen */
    }
}

/* Helper function to put pixel to an SDL_Surface (easier than locked texture) */
void set_pixel_on_surface(SDL_Surface* surface, int x, int y, Uint32 pixel) {
    if (!surface || x < 0 || y < 0 || x >= surface->w || y >= surface->h) return;

    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1:
            *p = pixel;
            break;

        case 2:
            *(Uint16 *)p = pixel;
            break;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
                p[0] = (pixel >> 16) & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = pixel & 0xff;
            } else {
                p[0] = pixel & 0xff;
                p[1] = (pixel >> 8) & 0xff;
                p[2] = (pixel >> 16) & 0xff;
            }
            break;

        case 4:
            *(Uint32 *)p = pixel;
            break;
    }
}

void calculate_spherical_uv_double_rotated(float point_x, float point_y, float point_z,
                                            float radius,
                                            float center_x, float center_y, float center_z,
                                            float rotation_angle_y_degrees, /* Y-axis rotation (Spin) */
                                            float rotation_angle_x_degrees, /* X-axis rotation (Tilt) */
                                            float *u_out, float *v_out) {

    float p_x = point_x - center_x;
    float p_y = point_y - center_y;
    float p_z = point_z - center_z;

    /* --- Step 1: Apply Y-Axis Rotation (Spin) --- */
    float angle_y_rad = rotation_angle_y_degrees * (M_PI / 180.0f);
    float cos_y = cos(angle_y_rad);
    float sin_y = sin(angle_y_rad);

    float temp_x = p_x * cos_y - p_z * sin_y;
    float temp_z = p_x * sin_y + p_z * cos_y;
    float temp_y = p_y; // Y is unchanged by Y-axis rotation

    /* --- Step 2: Apply X-Axis Rotation (Tilt) to the result --- */
    float angle_x_rad = rotation_angle_x_degrees * (M_PI / 180.0f);
    float cos_x = cos(angle_x_rad);
    float sin_x = sin(angle_x_rad);

    // X is unchanged by X-axis rotation
    float rotated_p_x = temp_x;
    /* y' = y*cos(A) - z*sin(A) */
    float rotated_p_y = temp_y * cos_x - temp_z * sin_x;
    /* z' = y*sin(A) + z*cos(A) */
    float rotated_p_z = temp_y * sin_x + temp_z * cos_x;

    /* --- Step 3: Calculate UV coordinates from the fully rotated point --- */
    // We can't simply use acos(-rotated_p_y / radius) if the point is *not* guaranteed to be radius distance.
    // Since our input points are guaranteed to be on the surface of a perfect sphere *before* rotation
    // and rotation preserves distance, we can use the original radius for normalization.
    float phi = acos(-rotated_p_y / radius);
    float theta = atan2(rotated_p_x, rotated_p_z);

    *u_out = 0.5f + theta / (2.0f * M_PI);
    *v_out = phi / M_PI;

    if (*u_out < 0.0f) {
        *u_out += 1.0f;
    }
}


/**
 * Render radar on a sphere
 * @param radar Radar object
 * @param rotation_angle_y_degrees Angle rotation
 * @param rotation_angle_x_degrees Angle rotation
 */
void render_uv_mapped_sphere(Radar *radar, float rotation_angle_y_degrees, float rotation_angle_x_degrees) {

    SDL_Renderer* radar_renderer = radar->renderer;

    if (radar->renderedTexture != NULL) {
        SDL_DestroyTexture(radar->renderedTexture);
        radar->renderedTexture = NULL;
    }

    // READ from Texture and copy to Surface :
    int tWidth, tHeight;
    SDL_QueryTexture(radar->workingTexture, NULL, NULL, &tWidth, &tHeight);

    SDL_Surface* source_surface = SDL_CreateRGBSurfaceWithFormat(0, tWidth, tHeight, 32, SDL_PIXELFORMAT_RGBA8888);

    SDL_SetRenderTarget(radar_renderer, radar->workingTexture);
    SDL_RenderReadPixels(
        radar_renderer, NULL, SDL_PIXELFORMAT_RGBA8888, source_surface->pixels, source_surface->pitch
    );
    SDL_SetRenderTarget(radar_renderer, NULL);

    /* 1. Create a new target surface for the final sphere image */
    /* It's easier to manipulate pixels in a Surface first, then convert to a Texture */
    SDL_Surface* sphere_surface = SDL_CreateRGBSurfaceWithFormat(
        0, tWidth, tHeight, 32, source_surface->format->format
    );

    if (!sphere_surface) {
        fprintf(stderr, "Could not create sphere surface: %s\n", SDL_GetError());
        return;
    }

    SDL_LockSurface(sphere_surface);

    /* 2. Iterate over all pixels in the sphere surface */
    for (int y = 0; y < sphere_surface->h; ++y) {
        for (int x = 0; x < sphere_surface->w; ++x) {
            /* Translate screen coords (x,y) to local surface coords relative to center */
            float local_x = (float)x -  radar->radius;
            float local_y = (float)y -  radar->radius;
            /* Calculate distance from a center (squared) to check if inside circle */
            float dist_sq = local_x * local_x + local_y * local_y;

            if (dist_sq <=  radar->radius *  radar->radius) {
                /* Point is inside the 2D screen projection of the sphere */

                /* Now we need a 3D point on the sphere's surface that projects to this (x, y) */
                /* Use local_x, local_y, and calculate z using Pythagoras (z = sqrt(r^2 - x^2 - y^2)) */
                float local_z = sqrtf( radar->radius *  radar->radius - dist_sq);

                /* Map this 3D point to UV coordinates */
                float u, v;
                calculate_spherical_uv_double_rotated(local_x, local_y, local_z,  radar->radius,
                                       0.0f, 0.0f, 0.0f, rotation_angle_y_degrees, rotation_angle_x_degrees, &u, &v);

                /* Get the corresponding pixel from the source texture */
                /* Map u, v (0.0 to 1.0) to actual pixel coordinates (0 to width/height - 1) */
                int tex_x = (int)(u * (source_surface->w - 1));
                /* In SDL, V=0 is typically top, but in UV mapping V=0 is often bottom. Invert V if needed. */
                int tex_y = (int)((1.0f - v) * (source_surface->h - 1));

                Uint32 color = get_pixel_from_surface(source_surface, tex_x, tex_y);

                /* Set the pixel color on our new sphere surface */
                set_pixel_on_surface(sphere_surface, x, y, color);

            } else {
                /* Point is outside the sphere, set to transparent or background color */
                /* Assuming a format with alpha channel (RGBA8888) and 0 is transparent */
                set_pixel_on_surface(sphere_surface, x, y, 0);
            }
        }
    }

    SDL_UnlockSurface(sphere_surface);

    radar->renderedTexture = SDL_CreateTextureFromSurface(radar_renderer, sphere_surface);
    if (!radar->renderedTexture) {
        fprintf(stderr, "Could not create sphere texture: %s\n", SDL_GetError());
    }

}