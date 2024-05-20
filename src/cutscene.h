#ifndef CUTSCENE_H
#define CUTSCENE_H

typedef int (*CutSceneFn) (Game *);

void RegisterCutscene(Game *game, CutSceneFn function, void *data);

extern const CutSceneFn cutscene_table[];

#endif
