#include "processing.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "image.h"
#include "pipeline.h"

typedef image_t* (*filter_fn)(image_t* img);

static const filter_fn filters[] = {
    filter_scale_up2,      //
    filter_desaturate,     //
    filter_gaussian_blur,  //
    filter_edge_detect,    //
    NULL,                  //
};

// Fonction qui traite une image
void* process_one_image(void* arg) {
  struct work_item* item = arg;
  const char* fname = item->input_file;
  printf("processing image: %s\n", fname);

  image_t* img = image_create_from_png(fname);
  if (!img) {
    printf("failed to load image %s\n", fname);
    goto err;
  }

  int i = 0;
  while (filters[i]) {
    filter_fn fn = filters[i];
    image_t* next = fn(img);
    if (!next) {
      printf("failed to process image%s\n", fname);
      image_destroy(img);
      goto err;
    }
    image_destroy(img);
    img = next;
    i++;
  }
  image_save_png(img, item->output_file);
  image_destroy(img);

  return 0;
err:
  return (void*)-1UL;
}

int process_serial(struct list* items) {
  struct list_node* node = list_head(items);
  while (!list_end(node)) {
    unsigned long ret = (unsigned long)process_one_image(node->data);
    if (ret < 0) {
      return -1;
    }
    node = node->next;
  }
  return 0;
}

void* worker_stage1_loader(void* arg, struct pipeline_stage* next_stage) {
  // À compléter:
  // Attendre un élément à charger
  // Charger l'image
  // Mettre l'image dans la liste de traitement

  printf("stage1\n");

  struct work_item* work = arg;
  image_t* img = image_create_from_png(work->input_file);

  if (!img) {
    printf("failed to load image %s\n", work->input_file);
    return NULL;
  }

  work->image = img;
  pipeline_stage_enqueue(next_stage, work);
  return NULL;
}

void* worker_stage2_filter(void* arg, struct pipeline_stage* next_stage) {
  // À compléter:
  // Attendre un élément à traiter
  // Appliquer les filtres
  // Mettre l'image dans la liste pour la sauvegarde

  printf("stage2\n");
  struct work_item* work = arg;

  int i = 0;
  int ok = 1;
  image_t* img = work->image;

  if (!img) {
    printf("image is NULL %s\n", work->input_file);
    return NULL;
  }

  while (filters[i]) {
    filter_fn fn = filters[i];
    image_t* next = fn(img);
    if (!next) {
      printf("failed to process image %s\n", work->input_file);
      ok = 0;
      break;
    }
    image_destroy(img);
    img = next;
    i++;
  }

  if (!ok) {
    return NULL;
  }

  // Mettre à jour le pointeur final
  work->image = img;

  pipeline_stage_enqueue(next_stage, work);
  return NULL;
}

void* worker_stage3_writer(void* arg, struct pipeline_stage* next_stage) {
  // À compléter:
  // Attendre un élément à sauvegarder
  // Sauvegarder l'image
  // Supprimer l'élément
  printf("stage3\n");
  (void) next_stage;

  struct work_item* work = arg;

  if (image_save_png(work->image, work->output_file) < 0) {
    printf("failed to save image %s\n", work->output_file);
  }

  return NULL;
}

int process_multithread(struct list* items, int nb_thread) {
  /*
   * À compléter:
   *   - Créer un pipeline de traitement
   *   - Ajouter toutes les images à traiter en file
   *   - Attendre que le traitement soit terminé
   */
  printf("À IMPLÉMENTER\n");

  struct pipeline* p = pipeline_create();

  pipeline_add_stage(p, worker_stage1_loader, 2, 10);
  pipeline_add_stage(p, worker_stage2_filter, nb_thread, 10);
  pipeline_add_stage(p, worker_stage3_writer, 2, 10);

  struct list_node* node = list_head(items);
  while (!list_end(node)) {
    struct work_item* item = node->data;
    pipeline_enqueue(p, item);
    node = node->next;
  }

  // Terminer le pipeline
  pipeline_join(p);

  return 0;
}

struct work_item* make_work_item(const char* input_file,
                                 const char* output_dir) {
  struct work_item* item = malloc(sizeof(struct work_item));
  item->input_file = strdup(input_file);
  item->output_file = strdup(output_dir);
  item->image = NULL;
  return item;
}

void free_work_item(void* item) {
  struct work_item* w = item;
  free(w->input_file);
  free(w->output_file);
  image_destroy(w->image);
  free(item);
}
