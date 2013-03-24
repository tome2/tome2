/* File: plots.h */

/* Purpose: extern plots declarations */

extern bool_ quest_null_hook(int q);

/******* Random Quests ********/
extern bool_ is_randhero(int level);
extern bool_ quest_random_init_hook(int q_idx);
extern bool_ quest_random_describe(FILE *fff);

/******* Plot main ********/
extern bool_ quest_necro_init_hook(int q_idx);
extern bool_ quest_one_init_hook(int q_idx);
extern bool_ quest_sauron_init_hook(int q_idx);
extern bool_ quest_morgoth_init_hook(int q_idx);
extern bool_ quest_ultra_good_init_hook(int q_idx);
extern bool_ quest_ultra_evil_init_hook(int q_idx);

/******* Plot Bree *********/
extern bool_ quest_thieves_init_hook(int q_idx);
extern bool_ quest_hobbit_init_hook(int q_idx);
extern bool_ quest_troll_init_hook(int q_idx);
