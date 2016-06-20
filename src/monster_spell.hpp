#pragma once

#include "monster_spell_flag_set.hpp"

#include <vector>

struct monster_spell {

	/**
	 * The global index of the spell.
	 */
	const std::size_t spell_idx;

	/**
	 * Flag set representation of the spell.
	 */
	const monster_spell_flag_set flag_set;

	/**
	 * System name of the spell as a string.
	 */
	const char *name;

	/**
	 * Is the spell a summoning spell?
	 */
	const bool is_summon;

	/**
	 * Is the spell an "annoyance" spell?
	 */
	const bool is_annoy;

	/**
	 * Is the spell a direct damage spell?
	 */
	const bool is_damage;

	/**
	 * Is the spell a bolt spell, i.e. would it
	 * affect any creature along the trajectory from
	 * the source to its target?
	 */
	const bool is_bolt;

	/**
	 * Does the spell require an intelligent caster?
	 */
	const bool is_smart;

	/**
	 * Is the spell an innate attack? For example, breaths
	 * are innate attacks.
	 */
	const bool is_innate;

	/**
	 * Is the spell an escape spell?
	 */
	const bool is_escape;

	/**
	 * Is the spell a "tactical" spell?
	 */
	const bool is_tactic;

	/**
	 * Does the spell apply haste?
	 */
	const bool is_haste;

	/**
	 * Does the spell apply any healing?
	 */
	const bool is_heal;

	/**
	 * Is the spell "magical" in nature? Magical spells
	 * can be stopped by the anti-magic field, and non-magical
	 * ones cannot.
	 *
	 * This is the inverse of the "innate" flag.
	 */
	const bool is_magic;

};

/**
 * Get a vector of all the spells.
 */
std::vector<monster_spell const *> const &monster_spells();
