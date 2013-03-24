/* File: plots.h */

/* Purpose: extern plots declarations */

extern bool_ quest_null_hook(int q);

/******* Random Quests ********/
extern bool_ is_randhero(int level);
extern bool_ quest_random_init_hook(int q_idx);
extern bool_ quest_random_describe(FILE *fff);
