#pragma once

#include "h-basic.hpp"

/**
 * Module descriptor.
 */
struct module_type
{
	/* Metadata about the module: author, description, etc. */
	struct {
		/* Module name */
		const char *name;

		/* Module version number */
		struct {
			s32b major;
			s32b minor;
			s32b patch;
		} version;

		/* Module author */
		struct {
			const char *name;
			const char *email;
		} author;

		/* Module description */
		const char *desc;

		/* Save file tag */
		const char *save_file_tag;

		/* Module directory */
		const char *module_dir;
	} meta;

	/* Random artifact generation chances */
	struct {
		s32b weapon_chance;
		s32b armor_chance;
		s32b jewelry_chance;
	} randarts;

	/* Skills */
	struct {
		/* Skill points per level */
		s32b skill_per_level;
		/* Maximum "overage" for skill points, i.e. how many skill
		   points you can go above your current level. */
		s32b max_skill_overage;
	} skills;

	/* Function to show introduction to module */
	void (*intro)();

	/* Function to compute race status, i.e. whether monsters
	   are friendly/neutral towards the player. Returns NULL
	   to indicate that no override happens. */
	s16b *(*race_status)(int r_idx);
};
