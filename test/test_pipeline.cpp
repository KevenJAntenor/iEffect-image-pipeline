#include <dirent.h>
#include <gtest/gtest.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <fstream>
#include <istream>
#include <iterator>
#include <memory>
#include <unordered_map>

#include "barrier.h"
#include "inf3173/config.h"
#include "pipeline.h"
#include "processing.h"

static const struct timespec jiffy = {.tv_sec = 0,
                                      .tv_nsec = 10000000};  // 10 ms

struct trace {
    int t1;
    int t2;
    int t3;
    void *s1;
    void *s2;
    void *s3;
};

std::ostream &operator<<(std::ostream &os, const std::vector<int> &vec) {
    os << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i != vec.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}

bool are_files_identical(const std::string &file1_path,
                         const std::string &file2_path) {
    std::ifstream file1(file1_path, std::ifstream::binary);
    std::ifstream file2(file2_path, std::ifstream::binary);

    if (!file1.is_open() || !file2.is_open()) {
        return false;
    }

    std::istreambuf_iterator<char> file1_iter(file1), file1_end;
    std::istreambuf_iterator<char> file2_iter(file2), file2_end;

    return std::equal(file1_iter, file1_end, file2_iter, file2_end);
}

void get_threads_list(std::vector<int> &tids) {
    DIR *dir;
    struct dirent *dirent;
    char path[256];

    tids.clear();

    sprintf(path, "/proc/%d/task", getpid());

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((dirent = readdir(dir)) != NULL) {
        // Filter out the "." and ".." directories
        if (dirent->d_name[0] == '.') {
            continue;
        }
        tids.push_back(std::atoi(dirent->d_name));
    }
    closedir(dir);
}

void *step1(void *arg, struct pipeline_stage *next_stage) {
    struct trace *t = static_cast<struct trace *>(arg);
    printf("Step1\n");
    t->t1 = gettid();
    t->s1 = next_stage;
    pipeline_stage_enqueue(next_stage, arg);
    return NULL;
}
void *step2(void *arg, struct pipeline_stage *next_stage) {
    printf("Step2\n");
    struct trace *t = static_cast<struct trace *>(arg);
    t->t2 = gettid();
    t->s2 = next_stage;
    pipeline_stage_enqueue(next_stage, arg);
    return NULL;
}
void *step3(void *arg, struct pipeline_stage *next_stage) {
    printf("Step3\n");
    struct trace *t = static_cast<struct trace *>(arg);
    t->t3 = gettid();
    t->s3 = next_stage;
    pipeline_stage_enqueue(next_stage, arg);
    return NULL;
}

void *sleeper(void *arg, struct pipeline_stage *next_stage) {
    (void)next_stage;
    (void)arg;

    nanosleep(&jiffy, NULL);
    return NULL;
}

TEST(Pipeline, CheckCreateJoin) {
    int n = 2;
    std::vector<int> t1, t2, t3;
    get_threads_list(t1);
    struct pipeline *p = pipeline_create();
    ASSERT_TRUE(p != nullptr);

    pipeline_add_stage(p, step1, n, 1);
    pipeline_add_stage(p, step2, n, 1);
    pipeline_add_stage(p, step3, n, 1);

    get_threads_list(t2);
    pipeline_join(p);

    // FIXME: un délai semble nécessaire pour que la liste de tâche dans
    // /proc/self soit à jour.
    nanosleep(&jiffy, NULL);
    get_threads_list(t3);

#if 0
    std::cout << "debug t1: " << t1 << std::endl;
    std::cout << "debug t2: " << t2 << std::endl;
    std::cout << "debug t3: " << t3 << std::endl;
#endif
    ASSERT_EQ(t1.size() + n * 3, t2.size())
        << "Mauvais nombre de fil d'execution démarré";

    ASSERT_EQ(t1.size(), t3.size()) << "Fils d'exécution encore actif";
}

TEST(Pipeline, CheckStage) {
    int n = 1;
    struct trace t = {-1, -1, -1, NULL, NULL, NULL};

    struct pipeline *p = pipeline_create();
    ASSERT_TRUE(p != nullptr);

    pipeline_add_stage(p, step1, n, 1);
    pipeline_add_stage(p, step2, n, 1);
    pipeline_add_stage(p, step3, n, 1);

    ASSERT_EQ(list_size(p->stages), 3);

    void *s1 = list_index(p->stages, 1)->data;
    void *s2 = list_index(p->stages, 2)->data;
    pipeline_enqueue(p, &t);
    pipeline_join(p);
    printf("sjsjjsjsdkdkg\n");
    ASSERT_GT(t.t1, 0) << "Étage 1 pas exécuté";
    ASSERT_GT(t.t2, 0) << "Étage 2 pas exécuté";
    ASSERT_GT(t.t3, 0) << "Étage 3 pas exécuté";
    ASSERT_NE(t.t1, t.t2) << "Traitement série détecté";
    ASSERT_NE(t.t2, t.t3) << "Traitement série détecté";

    // Vérifier que les étapes sont connectées
    ASSERT_EQ(t.s1, s1) << "Erreur chainage étapes";
    ASSERT_EQ(t.s2, s2) << "Erreur chainage étapes";
    ASSERT_EQ(t.s3, nullptr) << "Erreur chainage étapes";
}

TEST(Pipeline, CheckQueueLimit) {
    int limit = 10;
    int n = 20;
    int dummy = 0;

    struct pipeline *p = pipeline_create();
    ASSERT_TRUE(p != nullptr);

    pipeline_add_stage(p, sleeper, 1, limit);
    struct pipeline_stage *s =
        static_cast<struct pipeline_stage *>(list_front(p->stages));

    for (int i = 0; i < n; i++) {
        pipeline_enqueue(p, &dummy);
    }

    ASSERT_EQ(s->queue->max_size, limit)
        << "La file d'attente n'a pas atteint la bonne taille";

    pipeline_join(p);
}

static const char *img = SOURCE_DIR "/test/cat.png";
static const char *img_serial = BINARY_DIR "/test/cat-serial.png";
static const char *img_multithread = BINARY_DIR "/test/cat-multithread.png";

TEST(Pipeline, Image) {
    struct list *work_list = list_new(NULL, free_work_item);
    struct work_item *item =
        (struct work_item *)calloc(1, sizeof(struct work_item));

    item->input_file = strdup(img);
    item->output_file = strdup(img_serial);
    struct list_node *n = list_node_new(item);
    list_push_back(work_list, n);

    EXPECT_EQ(process_serial(work_list), 0);

    free(item->output_file);
    item->output_file = strdup(img_multithread);
    EXPECT_EQ(process_multithread(work_list, 2), 0);

    list_free(work_list);
    ASSERT_TRUE(are_files_identical(img_serial, img_multithread));
}
