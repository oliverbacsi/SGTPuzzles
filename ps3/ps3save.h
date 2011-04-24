/*
 * ps3save.h : PS3 Puzzles Loading/Saving mechanism
 *
 * Copyright (C) Youness Alaoui (KaKaRoTo)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#ifndef __PS3_SAVE_H__
#define __PS3_SAVE_H__

#include "ps3.h"

int ps3_save_game (frontend *fe);
int ps3_load_game (frontend *fe);

#endif /* __PS3_SAVE_H__ */
