#include "pipeline.h"
#include "processing.h"

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

void *worker(void *arg) {
  /*
   * À compléter:
   *
   * Dans une boucle infinie, on attend une tâches dans la file d'attente. On
   * prend un élément de la liste et on exécute la fonction de rappel. La
   * fonction est l'argument sont spécifiés dans l'item de la liste. On doit
   * également passer le pointeur vers l'étape suivante. Voici un exemple:
   *
   *    stage->func(item->data, s->next_stage);
   *
   * Attention: la liste peut être corrompue si on l'accède dans plusieurs
   * fils d'exécution en même temps. Il faut donc protéger l'accès avec un
   * verrou.
   *
   * La boucle doit se terminer si le work_item est NULL (i.e. item->data ==
   * NULL).
   */

  struct pipeline_stage *s = arg;
  while (1){
    //attend qu'il y'ait des element a traiter
    sem_wait(&s->busy);
    pthread_mutex_lock(&s->queue_lock);
    struct list_node *item = (struct list_node *)list_pop_front(s->queue);
    pthread_mutex_unlock(&s->queue_lock);
    //indique une place libre dans la file d'attente
    sem_post(&s->free);
    if (item==NULL||(item!=NULL&&item->data==NULL))
    { 
      free(item);
      item=NULL;
      break;
    }
    s->func(item->data, s->next_stage);
    free(item);
    item=NULL;
  }
  

  return NULL;
}

struct pipeline *pipeline_create() {
  /*
   * À compléter:
   *
   * Allouer la structure et créer la liste des étapes de traitement.
   * Retourner le pointeur vers la structure.
   */
  struct pipeline *pip = malloc(sizeof(*pip));
  if(pip!=NULL)
  {
    pip->stages =list_new(NULL, free_pipeline_stage);
  }
  return pip;
}

void pipeline_stage_enqueue(struct pipeline_stage *s, void *arg) {
  /*
   * À Compléter:
   *
   * Attendre qu'il y ait un espace libre dans la file
   * Ajouter l'élément dans la file (champs queue)
   * Indiquer qu'un nouvel élément est à traiter.
   *
   * Attention: il faut prendre le verrou avant de manipuler la liste, sinon une
   * condition critique pourrait survenir si le producteur ajoute un élément en
   * même temps que le consommateur retire un élément de la liste.
   */
  if(s!=NULL)
  {
    sem_wait(&s->free);
    pthread_mutex_lock(&s->queue_lock);
      list_push_back(s->queue,list_node_new(arg));
    pthread_mutex_unlock(&s->queue_lock);
    sem_post(&s->busy);
  }
}

void pipeline_enqueue(struct pipeline *p, void *arg) {
  /*
   * À compléter:
   *
   * Faire un appel à pipeline_stage_enqueue() avec la première étape de
   * traitement.
   */
  pipeline_stage_enqueue((list_head(p->stages))->data, arg);

}

void pipeline_join(struct pipeline *p) {
  /*
   * À compléter:
   *
   * Terminer chaque étape de traitement dans l'ordre (boucle sur la liste étapes)
   *
   * Ajouter un élément de travail NULL pour chaque fil d'exécution. Quand un
   * élément NULL est pris de la file d'attente, le fil d'exécution doit se
   * terminer.
   *
   * Ensuite, attendre que les fils d'exécution se terminent, puis libérer
   * la mémoire allouée avec pipeline_destroy.
   *
   * Quand la fonction retourne, le pipeline n'est plus utilisable..
   */
  struct pipeline_stage *s=NULL;
  int size_stages=list_size(p->stages);
  for (int i = 0; i < size_stages; i++)
  { 
      s=list_index(p->stages,i)->data;
      for (int j = 0; j < s->num_threads; j++)
      {
          pipeline_stage_enqueue(s,NULL);
      }
      //
      for (int j = 0; j < s->num_threads; j++)
      {
          pthread_join(s->threads[j],NULL);
      }
  }
  pipeline_destroy(p);
  return;
}

int pipeline_add_stage(struct pipeline *p, func_t f, int num_thread,
                       int queue_limit) {

  /* À compléter:
   *
   * Allouer un struct pipeline_stage
   * Initialiser les champs
   * Connecter les étapes à l'aide du champs next_stage
   * Ajouter la nouvelle étape à la fin de la liste du pipeline
   * Démarrer les fils d'exécution de cette étape
   */
  struct pipeline_stage *s=malloc(sizeof(*s));
  if(s==NULL) return -1;
  //initialisation des semaphores
  sem_init(&s->busy,0,queue_limit);
  sem_init(&s->free,0,queue_limit);
  //decremente pour obliger les threads a attendre 
  for (int i = 0; i < queue_limit; i++)
  {
      sem_wait(&s->busy);
  }
  //initialisation de queue 
  s->queue=list_new(NULL,free_work_item);
  //initialisation du verrou
  pthread_mutex_init(&s->queue_lock, NULL);
  //Connecter les étapes à l'aide du champs next_stage
  s->next_stage=NULL;
  if(list_size(p->stages)>0)
  {
    struct pipeline_stage *pred=(struct pipeline_stage *)list_back(p->stages);
    pred->next_stage=s;
  }
  s->stage_id=list_size(p->stages);
  //
  s->num_threads=num_thread;
  s->func=f;
  s->threads=malloc(sizeof(*s->threads)*s->num_threads);
  //
  list_push_back(p->stages,list_node_new(s));
  //demarrage des fils d'execution
  for (int i = 0; i < s->num_threads; i++)
  {
    pthread_create(s->threads+i,NULL,worker,s);
  }
  
  return s->stage_id;
}

void pipeline_destroy(struct pipeline *p)
{
  /*
   * Libérer les ressources du pipeline
   * incluant struct pipeline lui-même
   */
  list_free(p->stages);
  free(p);
  p=NULL;
}

void free_pipeline_stage(void *obj)
{
  /* À Compléter:
   * libérer les ressources de struct pipeline_stage
   * incluant le pipeline_stage lui-même.
   */

  struct pipeline_stage *s = obj;
  sem_destroy(&s->busy);
  sem_destroy(&s->free);
  list_free(s->queue);
  free(s->threads);
  pthread_mutex_destroy(&s->queue_lock);
  free(s);
  s=NULL;
}
