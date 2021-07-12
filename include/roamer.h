#ifndef GUARD_ROAMER_H
#define GUARD_ROAMER_H

#include "global.h"

void ClearRoamerData(void);
void ClearRoamerLocationData(void);
void InitRoamer(void);
void UpdateRoamerHistories();
void MoveRoamersToOtherLocationSet();
void MoveRoamers();
void CreateRoamerMonInstance(void);
u8 TryStartRoamerEncounter(void);
void UpdateRoamerHPStatus(struct Pokemon *mon);
void SetRoamerInactive(struct Pokemon *mon);
u8 GetRoamerMapNumber(u8 index);
u16 GetRoamerLocationMapSectionId(u16 species);

#endif // GUARD_ROAMER_H
