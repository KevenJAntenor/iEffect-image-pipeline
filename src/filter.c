/*
 * Auteur: Gabriel-Andrew Pollo Guilbert
 *
 * Utilisation avec permission
 */

#include <math.h>
#include <stdlib.h>

#include "filter.h"
#include "image.h"

#define max(a, b) (((a) < (b)) ? (b) : (a))
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define clamp(x, min, max) ((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x))

static void hsv_to_rgb(unsigned char hsv[3], unsigned char rgb[3]) {
  unsigned char h = hsv[0];
  unsigned char s = hsv[1];
  unsigned char v = hsv[2];

  unsigned char r = 0;
  unsigned char g = 0;
  unsigned char b = 0;

  /* taken from https://stackoverflow.com/a/14733008 */

  if (s == 0) {
    r = v;
    g = v;
    b = v;
    goto done;
  }

  unsigned char region = h / 43;
  unsigned char remainder = (h - (region * 43)) * 6;

  unsigned char p = (v * (255 - s)) >> 8;
  unsigned char q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  unsigned char t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

  switch (region) {
  case 0:
    r = v;
    g = t;
    b = p;
    break;
  case 1:
    r = q;
    g = v;
    b = p;
    break;
  case 2:
    r = p;
    g = v;
    b = t;
    break;
  case 3:
    r = p;
    g = q;
    b = v;
    break;
  case 4:
    r = t;
    g = p;
    b = v;
    break;
  default:
    r = v;
    g = p;
    b = q;
    break;
  }

done:
  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
}

static void rgb_to_hsv(unsigned char rgb[3], unsigned char hsv[3]) {
  unsigned char r = rgb[0];
  unsigned char g = rgb[1];
  unsigned char b = rgb[2];

  /* taken from https://stackoverflow.com/a/14733008 */

  unsigned char cmin = min(r, min(g, b));
  unsigned char cmax = max(r, max(g, b));

  unsigned char h = 0;
  unsigned char s = 0;
  unsigned char v = cmax;

  if (v == 0) {
    h = 0;
    s = 0;
    goto done;
  }

  s = (255 * ((long)(cmax - cmin))) / v;
  if (s == 0) {
    h = 0;
    goto done;
  }

  if (cmax == r) {
    h = 0 + 43 * (g - b) / (cmax - cmin);
  } else if (cmax == g) {
    h = 85 + 43 * (b - r) / (cmax - cmin);
  } else {
    h = 171 + 43 * (r - g) / (cmax - cmin);
  }

done:
  hsv[0] = h;
  hsv[1] = s;
  hsv[2] = v;
}

image_t *filter_scale_up2(image_t *image) { return filter_scale_up(image, 2); }

image_t *filter_scale_up(image_t *image, size_t factor) {
  image_t *new_image =
      image_create(image->id, factor * image->width, factor * image->height);
  if (new_image == NULL) {
    goto fail_exit;
  }

  for (int j = 0; j < image->height; j++) {
    for (int i = 0; i < image->width; i++) {
      pixel_t *pixel = image_get_pixel(image, i, j);

      for (int kj = 0; kj < factor; kj++) {
        for (int ki = 0; ki < factor; ki++) {
          pixel_t *new_pixel =
              image_get_pixel(new_image, factor * i + ki, factor * j + kj);
          *new_pixel = *pixel;
        }
      }
    }
  }

  return new_image;

fail_exit:
  return NULL;
}

image_t *filter_sobel(image_t *image) {
  image_t *new_image =
      image_create(image->id, image->width - 2, image->height - 2);
  if (new_image == NULL) {
    goto fail_exit;
  }

  const int gx[3][3] = {
      {1, 0, -1},
      {2, 0, -2},
      {1, 0, -1},
  };

  const int gy[3][3] = {
      {1, 2, 1},
      {0, 0, 0},
      {-1, -2, -1},
  };

  for (int j = 1; j < image->height - 1; j++) {
    for (int i = 1; i < image->width - 1; i++) {
      int values_x[4] = {0, 0, 0, 0};
      int values_y[4] = {0, 0, 0, 0};

      for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
          pixel_t *pixel = image_get_pixel(image, i + x, j + y);

          for (int k = 0; k < 4; k++) {
            values_x[k] += pixel->bytes[k] * gx[y + 1][x + 1];
            values_y[k] += pixel->bytes[k] * gy[y + 1][x + 1];
          }
        }
      }

      pixel_t *new_pixel = image_get_pixel(new_image, i - 1, j - 1);
      pixel_t *pixel = image_get_pixel(image, i, j);

      for (int k = 0; k < 3; k++) {
        new_pixel->bytes[k] =
            clamp(abs(values_x[k]) + abs(values_y[k]), 0, 255);
      }
      new_pixel->bytes[3] = pixel->bytes[3];
    }
  }

  return new_image;

fail_exit:
  return NULL;
}

image_t *filter_to_hsv(image_t *image) {
  image_t *new_image = image_create(image->id, image->width, image->height);
  if (new_image == NULL) {
    goto fail_exit;
  }

  for (int j = 0; j < image->height; j++) {
    for (int i = 0; i < image->width; i++) {
      pixel_t *pixel = image_get_pixel(image, i, j);
      pixel_t *new_pixel = image_get_pixel(new_image, i, j);

      rgb_to_hsv(pixel->bytes, new_pixel->bytes);
      new_pixel->bytes[3] = pixel->bytes[3];
    }
  }

  return new_image;

fail_exit:
  return NULL;
}

image_t *filter_to_rgb(image_t *image) {
  image_t *new_image = image_create(image->id, image->width, image->height);
  if (new_image == NULL) {
    goto fail_exit;
  }

  for (int j = 0; j < image->height; j++) {
    for (int i = 0; i < image->width; i++) {
      pixel_t *pixel = image_get_pixel(image, i, j);
      pixel_t *new_pixel = image_get_pixel(new_image, i, j);

      hsv_to_rgb(pixel->bytes, new_pixel->bytes);
      new_pixel->bytes[3] = pixel->bytes[3];
    }
  }

  return new_image;

fail_exit:
  return NULL;
}

image_t *filter_add_pixel(image_t *image, pixel_t *add_pixel) {
  image_t *new_image = image_create(image->id, image->width, image->height);
  if (new_image == NULL) {
    goto fail_exit;
  }

  for (int j = 0; j < image->height; j++) {
    for (int i = 0; i < image->width; i++) {
      pixel_t *pixel = image_get_pixel(image, i, j);
      pixel_t *new_pixel = image_get_pixel(new_image, i, j);

      for (int k = 0; k < 3; k++) {
        new_pixel->bytes[k] = pixel->bytes[k] + add_pixel->bytes[k];
      }

      new_pixel->bytes[3] = pixel->bytes[3];
    }
  }

  return new_image;

fail_exit:
  return NULL;
}

image_t *filter_desaturate(image_t *image) {
  image_t *new_image = image_create(image->id, image->width, image->height);
  if (new_image == NULL) {
    goto fail_exit;
  }

  for (int j = 0; j < image->height; j++) {
    for (int i = 0; i < image->width; i++) {
      pixel_t *pixel = image_get_pixel(image, i, j);
      pixel_t *new_pixel = image_get_pixel(new_image, i, j);

      double value = 0;
      value += 0.30 * ((double)pixel->bytes[0]);
      value += 0.59 * ((double)pixel->bytes[1]);
      value += 0.11 * ((double)pixel->bytes[2]);

      new_pixel->bytes[0] = (unsigned char)value;
      new_pixel->bytes[1] = (unsigned char)value;
      new_pixel->bytes[2] = (unsigned char)value;
      new_pixel->bytes[3] = pixel->bytes[3];
    }
  }

  return new_image;

fail_exit:
  return NULL;
}

image_t *filter_convolution33(image_t *image, const double m[3][3]) {
  image_t *new_image =
      image_create(image->id, image->width - 2, image->height - 2);
  if (new_image == NULL) {
    goto fail_exit;
  }

  for (int j = 1; j < image->height - 1; j++) {
    for (int i = 1; i < image->width - 1; i++) {
      double values[3] = {0, 0, 0};

      for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
          pixel_t *pixel = image_get_pixel(image, i + x, j + y);

          for (int k = 0; k < 3; k++) {
            values[k] += pixel->bytes[k] * m[y + 1][x + 1];
          }
        }
      }

      pixel_t *new_pixel = image_get_pixel(new_image, i - 1, j - 1);
      pixel_t *pixel = image_get_pixel(image, i, j);

      for (int k = 0; k < 3; k++) {
        new_pixel->bytes[k] = (unsigned char)clamp(values[k], 0, 255);
      }

      new_pixel->bytes[3] = pixel->bytes[3];
    }
  }

  return new_image;

fail_exit:
  return NULL;
}

image_t *filter_edge_identity(image_t *image) {
  const double m[3][3] = {
      {0, 0, 0},
      {0, 1, 0},
      {0, 0, 0},
  };

  return filter_convolution33(image, m);
}

image_t *filter_edge_detect(image_t *image) {
  const double m[3][3] = {
      {-1, -1, -1},
      {-1, 8, -1},
      {-1, -1, -1},
  };

  return filter_convolution33(image, m);
}

image_t *filter_sharpen(image_t *image) {
  const double m[3][3] = {
      {0, -2, 0},
      {-2, 9, -2},
      {0, -2, 0},
  };

  return filter_convolution33(image, m);
}

image_t *filter_box_blur(image_t *image) {
  const double m[3][3] = {
      {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0},
      {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0},
      {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0},
  };

  return filter_convolution33(image, m);
}

image_t *filter_gaussian_blur(image_t *image) {
  const double m[3][3] = {
      {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0},
      {2.0 / 16.0, 4.0 / 16.0, 4.0 / 16.0},
      {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0},
  };

  return filter_convolution33(image, m);
}

image_t *filter_horizontal_flip(image_t *image) {
  image_t *new_image = image_create(image->id, image->width, image->height);
  if (new_image == NULL) {
    goto fail_exit;
  }

  for (int j = 0; j < image->height; j++) {
    for (int i = 0; i < image->width; i++) {
      pixel_t *pixel = image_get_pixel(image, i, j);
      pixel_t *new_pixel =
          image_get_pixel(new_image, (image->width - 1) - i, j);

      *new_pixel = *pixel;
    }
  }

  return new_image;

fail_exit:
  return NULL;
}

image_t *filter_vertical_flip(image_t *image) {
  image_t *new_image = image_create(image->id, image->width, image->height);
  if (new_image == NULL) {
    goto fail_exit;
  }

  for (int j = 0; j < image->height; j++) {
    for (int i = 0; i < image->width; i++) {
      pixel_t *pixel = image_get_pixel(image, i, j);
      pixel_t *new_pixel =
          image_get_pixel(new_image, i, (image->height - j) - 1);

      *new_pixel = *pixel;
    }
  }

  return new_image;

fail_exit:
  return NULL;
}
