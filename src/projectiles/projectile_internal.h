/*
 * Functions and data types designed for use by implementors of struct
 * projectile
 */

#ifndef PROJECTILE_INTERNAL_H
#define PROJECTILE_INTERNAL_H

#include "../projectile.h"

struct projectile {
	const int radius;
	void (*const reset)(void);
	void (*const launch)(float velX, float velY);
	void (*const move)(void);
	bool (*const isMoving)(void);
	void (*const onHitGround)(float hitX, float hitY);
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
