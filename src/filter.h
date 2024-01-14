/*
 * Auteur: Gabriel-Andrew Pollo Guilbert
 *
 * Utilisation avec permission
 */

#ifndef INF3170_FILTER_H_
#define INF3170_FILTER_H_

#include "image.h"

/* all filter return a newly allocated image, input image is not freed  */

#ifdef __cplusplus
extern "C" {
#endif

image_t *filter_scale_up(image_t *image, size_t factor);
image_t *filter_scale_up2(image_t *image);
image_t *filter_sobel(image_t *image);
image_t *filter_to_hsv(image_t *image);
image_t *filter_to_rgb(image_t *image);
image_t *filter_add_pixel(image_t *image, pixel_t *add_pixel);
image_t *filter_desaturate(image_t *image);
image_t *filter_convolution33(image_t *image, const double m[3][3]);
image_t *filter_edge_identity(image_t *image);
image_t *filter_edge_detect(image_t *image);
image_t *filter_sharpen(image_t *image);
image_t *filter_box_blur(image_t *image);
image_t *filter_gaussian_blur(image_t *image);
image_t *filter_horizontal_flip(image_t *image);
image_t *filter_vertical_flip(image_t *image);

#ifdef __cplusplus
}
#endif

#endif
