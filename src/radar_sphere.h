#ifndef RADAR_SPHERE_H
#define RADAR_SPHERE_H

void calculate_spherical_uv_double_rotated(float point_x, float point_y, float point_z,
                                            float radius,
                                            float center_x, float center_y, float center_z,
                                            float rotation_angle_y_degrees, /* Y-axis rotation (Spin) */
                                            float rotation_angle_x_degrees, /* X-axis rotation (Tilt) */
                                            float *u_out, float *v_out);
void render_uv_mapped_sphere(Radar *radar, float rotation_angle_y_degrees, float rotation_angle_x_degrees) ;
void set_pixel_on_surface(SDL_Surface* surface, int x, int y, Uint32 pixel);
Uint32 get_pixel_from_surface(SDL_Surface* surface, int x, int y);

#endif
