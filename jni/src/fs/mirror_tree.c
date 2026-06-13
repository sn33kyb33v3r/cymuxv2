#include "../core/cymux_engine.h"
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NODES 1024

typedef struct {
    char path[512];
    bool is_directory;
} FMNode;

typedef struct {
    FMNode nodes[MAX_NODES];
    size_t node_count;
} FMTree;

void scan_directory_tree(FMTree* tree, const char* base_path) {
    DIR* dir = opendir(base_path);
    if (!dir) return;

    struct dirent* entry;
    tree->node_count = 0;

    while ((entry = readdir(dir)) != NULL && tree->node_count < MAX_NODES) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        FMNode* node = &tree->nodes[tree->node_count];
        snprintf(node->path, sizeof(node->path), "%s/%s", base_path, entry->d_name);
        node->is_directory = (entry->d_type == DT_DIR);
        tree->node_count++;
    }
    closedir(dir);
}
