#define _GNU_SOURCE
#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"
#include "pipeline.h"
#include "processing.h"
#include "utils.h"

struct app {
  char *input;
  char *output;
  int multithread;
  struct list *work_list;
  int nb_threads;
};

void print_usage() { fprintf(stderr, "Usage: %s [-ioenh]\n", "ieffect"); }

int main(int argc, char **argv) {
  int ret = 0;
  printf("ieffect\n");

  struct option options[] = {
      {"input", 1, 0, 'i'},       {"output", 1, 0, 'o'},
      {"multithread", 0, 0, 'm'}, {"nb-thread", 1, 0, 'n'},
      {"help", 0, 0, 'h'},        {0, 0, 0, 0}};

  struct app app = {
      .input = NULL,              //
      .output = NULL,             //
      .multithread = 0,           //
      .nb_threads = get_nprocs(), //
  };

  app.work_list = list_new(NULL, free_work_item);

  int opt;
  int idx;
  while ((opt = getopt_long(argc, argv, "i:o:n:mh", options, &idx)) != -1) {
    printf("opt=%d optarg=%s\n", opt, optarg);
    switch (opt) {
    case 'i':
      app.input = optarg;
      break;
    case 'o':
      app.output = optarg;
      break;
    case 'm': {
      app.multithread = 1;
    } break;
    case 'n':
      app.nb_threads = atoi(optarg);
      break;
    default:
      print_usage();
    }
  }

  {
    // Options summary
    printf("options\n");
    printf(" input           : %s\n", app.input);
    printf(" output          : %s\n", app.output);
    printf(" multithread     : %d\n", app.multithread);
    printf(" nb_threads      : %d\n", app.nb_threads);
  }

  if (app.nb_threads < 1 || app.nb_threads > 128) {
    printf("wrong number of threads\n");
    ret = 1;
    goto out_list;
  }

  if (!app.input || !app.output) {
    print_usage();
    ret = 1;
    goto out_list;
  }

  // Lister les fichiers si input est un repertoire
  if (is_regular_file(app.input)) {
    if (!ends_with(app.input, ".png")) {
      printf("Only png files are supported\n");
      ret = 1;
      goto out_list;
    }

    struct work_item *item = malloc(sizeof(struct work_item));
    item->image = NULL;
    item->input_file = strdup(app.input);
    item->output_file = strdup(app.output);
    struct list_node *n = list_node_new(item);
    list_push_back(app.work_list, n);
  } else if (is_dir(app.input)) {
    // list repertoire

    // Si la sortie est un fichier regulier, c'est invalide
    if (is_regular_file(app.output)) {
      print_usage();
      ret = 1;
      goto out_list;
    }

    // Creer le repertoire de sortie si necessaire
    if (!is_dir(app.output)) {
      if (mkdir(app.output, 0755) < 0) {
        perror("mkdir");
        ret = 1;
        goto out_list;
      }
    }

    struct dirent *entry;
    DIR *dir = opendir(app.input);
    if (!dir) {
      printf("failed to list directory\n");
      ret = 1;
      goto out_list;
    }

    while ((entry = readdir(dir)) != NULL) {
      if (ends_with(entry->d_name, ".png")) {
        struct work_item *item = calloc(1, sizeof(struct work_item));
        struct list_node *n = list_node_new(item);
        list_push_back(app.work_list, n);

        if (asprintf(&item->input_file,  //
                     "%s/%s", app.input, //
                     entry->d_name) < 0)
          goto out_list;

        if (asprintf(&item->output_file, "%s/%s", //
                     app.output,                  //
                     entry->d_name) < 0)
          goto out_list;
      }
    }
    closedir(dir);
  } else {
    printf("unkown file type");
    ret = 1;
    goto out_list;
  }

  printf("Number of files to process: %lu\n", list_size(app.work_list));

  if (app.multithread) {
    process_multithread(app.work_list, app.nb_threads);
  } else {
    process_serial(app.work_list);
  }

out_list:
  list_free(app.work_list);

  return ret;
}
