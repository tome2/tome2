/* File: plots.h */

/* Purpose: extern plots declarations */

extern bool quest_null_hook(int q);

/******* Random Quests ********/
extern bool is_randhero(int level);
extern bool quest_random_init_hook(int q_idx);

/******* Plot main ********/
extern bool quest_necro_init_hook(int q_idx);
extern bool quest_one_init_hook(int q_idx);
extern bool quest_sauron_init_hook(int q_idx);
extern bool quest_morgoth_init_hook(int q_idx);
extern bool quest_ultra_good_init_hook(int q_idx);
extern bool quest_ultra_evil_init_hook(int q_idx);

/******* Plot Bree *********/
extern bool quest_thieves_init_hook(int q_idx);
extern bool quest_hobbit_init_hook(int q_idx);
extern bool quest_troll_init_hook(int q_idx);
extern bool quest_wight_init_hook(int q_idx);
extern bool quest_nazgul_init_hook(int q_idx);
extern bool quest_shroom_init_hook(int q_idx);

/******* Plot Lorien *********/
extern bool quest_wolves_init_hook(int q_idx);
extern bool quest_spider_init_hook(int q_idx);
extern bool quest_poison_init_hook(int q_idx);

/******* Plot Gondolin *********/
extern bool quest_dragons_init_hook(int q_idx);
extern bool quest_eol_init_hook(int q_idx);
extern bool quest_nirnaeth_init_hook(int q_idx);
extern bool quest_invasion_init_hook(int q_idx);

/******* Plot Minas Anor *********/
extern bool quest_haunted_init_hook(int q_idx);
extern bool quest_between_init_hook(int q_idx);

/******* Plot Khazad-dum *********/
extern bool quest_evil_init_hook(int q_idx);

/******* Plot Other *********/
extern bool quest_narsil_init_hook(int q_idx);
extern bool quest_thrain_init_hook(int q_idx);
