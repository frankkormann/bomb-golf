/*
 * Functions and data types designed for use by implementors of a Projectile.
 */

#ifndef PROJECTILE_INTERNAL_H
#define PROJECTILE_INTERNAL_H

#include "../projectile.h"

struct projectile {
	const int radius;
	/*
	 * Restores the projectile to its default state.
	 */
	void (*const reset)(void);
	/*
	 * Launches the projectile with initial velocity (velX, velY).
	 */
	void (*const launch)(float velX, float velY);
	/*
	 * Updates the projectile for one physics frame.
	 */
	void (*const move)(void);
	/*
	 * Returns true if the projectile is considered moving.
	 */
	bool (*const isMoving)(void);
	/*
	 * Updates the projectile after it has hit a ground pixel located at
	 * (hitX, hitY).
	 */
	void (*const onHitGround)(float hitX, float hitY);
	/*
	 * Draws the projectile to the screen at z-level depth.
	 */
	void (*const draw)(float depth);
};

typedef struct {
	float x;
	float y;
	float velX;
	float velY;
} ProjectileI_Data;

/*
 * Allows reading/writing to internal data of the projectile.
 */
ProjectileI_Data* ProjectileI_AccessData();

/*
 * Default implementations of each function in struct projectile
 */
void ProjDefault_Reset();
void ProjDefault_Launch(float velX, float velY);
void ProjDefault_Move();
bool ProjDefault_IsMoving();
void ProjDefault_OnHitGround(float hitX, float hitY);
void ProjDefault_Draw(float depth);  // Does nothing

#endif
