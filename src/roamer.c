#include "global.h"
#include "random.h"
#include "overworld.h"
#include "field_specials.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"

#define LOCATION_HISTORY_COUNT 3

typedef struct RoamerLocation
{
  u8 group_number;
  u8 map_number;
} RoamerLocation;

typedef struct RoamerInfo
{
  u8 roamer_index;
  u16 roamer_species;
  RoamerLocation sRoamerLocation;
  RoamerLocation sLocationHistory[3];
} RoamerInfo;

static EWRAM_DATA RoamerInfo roamerInfos[ROAMER_SPECIES_COUNT] = {};

const u16 roamer_types[ROAMER_SPECIES_COUNT] = {SPECIES_ENTEI, SPECIES_SUICUNE, SPECIES_RAIKOU};

/** somehow prevents roamer pointer from dying */
#define saveRoamers (*(&gSaveBlock1Ptr->roamers))

enum
{
    MAP_GRP = 0, // map group
    MAP_NUM = 1, // map number
};

const u8 sRoamerLocations[][7] = {
    {MAP_NUM(ROUTE1), MAP_NUM(ROUTE2), MAP_NUM(ROUTE21_NORTH), MAP_NUM(ROUTE22), 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE2), MAP_NUM(ROUTE1), MAP_NUM(ROUTE3), MAP_NUM(ROUTE22), 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE3), MAP_NUM(ROUTE2), MAP_NUM(ROUTE4), 0xff, 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE4), MAP_NUM(ROUTE3), MAP_NUM(ROUTE5), MAP_NUM(ROUTE9), MAP_NUM(ROUTE24), 0xff, 0xff},
    {MAP_NUM(ROUTE5), MAP_NUM(ROUTE4), MAP_NUM(ROUTE6), MAP_NUM(ROUTE7), MAP_NUM(ROUTE8), MAP_NUM(ROUTE9), MAP_NUM(ROUTE24)},
    {MAP_NUM(ROUTE6), MAP_NUM(ROUTE5), MAP_NUM(ROUTE7), MAP_NUM(ROUTE8), MAP_NUM(ROUTE11), 0xff, 0xff},
    {MAP_NUM(ROUTE7), MAP_NUM(ROUTE5), MAP_NUM(ROUTE6), MAP_NUM(ROUTE8), MAP_NUM(ROUTE16), 0xff, 0xff},
    {MAP_NUM(ROUTE8), MAP_NUM(ROUTE5), MAP_NUM(ROUTE6), MAP_NUM(ROUTE7), MAP_NUM(ROUTE10), MAP_NUM(ROUTE12), 0xff},
    {MAP_NUM(ROUTE9), MAP_NUM(ROUTE4), MAP_NUM(ROUTE5), MAP_NUM(ROUTE10), MAP_NUM(ROUTE24), 0xff, 0xff},
    {MAP_NUM(ROUTE10), MAP_NUM(ROUTE8), MAP_NUM(ROUTE9), MAP_NUM(ROUTE12), 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE11), MAP_NUM(ROUTE6), MAP_NUM(ROUTE12), 0xff, 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE12), MAP_NUM(ROUTE10), MAP_NUM(ROUTE11), MAP_NUM(ROUTE13), 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE13), MAP_NUM(ROUTE12), MAP_NUM(ROUTE14), 0xff, 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE14), MAP_NUM(ROUTE13), MAP_NUM(ROUTE15), 0xff, 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE15), MAP_NUM(ROUTE14), MAP_NUM(ROUTE18), MAP_NUM(ROUTE19), 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE16), MAP_NUM(ROUTE7), MAP_NUM(ROUTE17), 0xff, 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE17), MAP_NUM(ROUTE16), MAP_NUM(ROUTE18), 0xff, 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE18), MAP_NUM(ROUTE15), MAP_NUM(ROUTE17), MAP_NUM(ROUTE19), 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE19), MAP_NUM(ROUTE15), MAP_NUM(ROUTE18), MAP_NUM(ROUTE20), 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE20), MAP_NUM(ROUTE19), MAP_NUM(ROUTE21_NORTH), 0xff, 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE21_NORTH), MAP_NUM(ROUTE1), MAP_NUM(ROUTE20), 0xff, 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE22), MAP_NUM(ROUTE1), MAP_NUM(ROUTE2), MAP_NUM(ROUTE23), 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE23), MAP_NUM(ROUTE22), MAP_NUM(ROUTE2), 0xff, 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE24), MAP_NUM(ROUTE4), MAP_NUM(ROUTE5), MAP_NUM(ROUTE9), 0xff, 0xff, 0xff},
    {MAP_NUM(ROUTE25), MAP_NUM(ROUTE24), MAP_NUM(ROUTE9), 0xff, 0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
};

void ClearRoamerData(void)
{
    u8 i;
    u8 j;
    for (i = 0; i < ROAMER_SPECIES_COUNT; i++)
    {
      gSaveBlock1Ptr->roamers[i] = (struct Roamer){};
      roamerInfos[i].sRoamerLocation = (RoamerLocation){};
      for (j = 0; j < LOCATION_HISTORY_COUNT; j++)
      {
          roamerInfos[i].sLocationHistory[j] = (RoamerLocation){};
      }
    }
}

static void CreateInitialRoamerMons(void)
{
  struct Pokemon *tmpMon;
  struct Roamer *tmpRoamer;

  int i;
  for (i = 0; i < ROAMER_SPECIES_COUNT; i++){
    tmpMon = &gEnemyParty[0];
    CreateMon(tmpMon, roamer_types[i], 50, 0x20, 0, 0, 0, 0);
    tmpRoamer = &saveRoamers[i];
    tmpRoamer->species = roamer_types[i];
    tmpRoamer->level = 50;
    tmpRoamer->status = 0;
    tmpRoamer->active = TRUE;
    tmpRoamer->ivs = GetMonData(tmpMon, MON_DATA_IVS);
    tmpRoamer->personality = GetMonData(tmpMon, MON_DATA_PERSONALITY);
    tmpRoamer->hp = GetMonData(tmpMon, MON_DATA_MAX_HP);
    tmpRoamer->cool = GetMonData(tmpMon, MON_DATA_COOL);
    tmpRoamer->beauty = GetMonData(tmpMon, MON_DATA_BEAUTY);
    tmpRoamer->cute = GetMonData(tmpMon, MON_DATA_CUTE);
    tmpRoamer->smart = GetMonData(tmpMon, MON_DATA_SMART);
    tmpRoamer->tough = GetMonData(tmpMon, MON_DATA_TOUGH);
    roamerInfos[i].roamer_index = i;
    roamerInfos[i].roamer_species = tmpRoamer->species;
    roamerInfos[i].sRoamerLocation.group_number = 3;
    // sRoamerLocation[MAP_NUM] = sRoamerLocations[Random() % (NELEMS(sRoamerLocations) - 1)][0];
    roamerInfos[i].sRoamerLocation.map_number = sRoamerLocations[0][0];
  }
}

void InitRoamer(void)
{
    ClearRoamerData();
    CreateInitialRoamerMons();
}

static void UpdateLocationHistoryForRoamer(RoamerLocation sLocationHistory[3])
{ 
   sLocationHistory[2].group_number = sLocationHistory[1].group_number;
   sLocationHistory[2].map_number = sLocationHistory[1].map_number;
   sLocationHistory[1].group_number = sLocationHistory[0].group_number;
   sLocationHistory[1].map_number = sLocationHistory[0].map_number;
   sLocationHistory[0].group_number = gSaveBlock1Ptr->location.mapGroup;
   sLocationHistory[0].map_number = gSaveBlock1Ptr->location.mapNum;
}

void UpdateRoamerHistories()
{
  int i;
  for (i = 0; i < ROAMER_SPECIES_COUNT; i++)
  {
    struct RoamerInfo *roamerInfo = &roamerInfos[i];
    UpdateLocationHistoryForRoamer(roamerInfo->sLocationHistory);
  }
}

// todo: roamer
static void RoamerMoveToOtherLocationSet(RoamerInfo *roamerInfo)
{
   roamerInfo->sRoamerLocation.group_number = 3;
   roamerInfo->sRoamerLocation.map_number = sRoamerLocations[0][0];

    // u8 mapNum = 0;
    // struct Roamer *roamer = &saveRoamers[0];

    // if (!roamer->active)
    //     return;

    // sRoamerLocation[MAP_GRP] = 3;

    // while (1)
    // {
    //     mapNum = sRoamerLocations[Random() % (NELEMS(sRoamerLocations) - 1)][0];
    //     if (sRoamerLocation[MAP_NUM] != mapNum)
    //     {
    //         sRoamerLocation[MAP_NUM] = mapNum;
    //         return;
    //     }
    // }
}

void MoveRoamersToOtherLocationSet()
{
  int i;
  for (i = 0; i < ROAMER_SPECIES_COUNT; i++)
  {
    struct RoamerInfo *roamerInfo = &roamerInfos[i];
    RoamerMoveToOtherLocationSet(roamerInfo);
  }
}

// todo: roamer
static void RoamerMove(RoamerInfo *roamerInfo)
{
    RoamerMoveToOtherLocationSet(roamerInfo);
    // u8 locSet = 0;

    // if ((Random() % 16) == 0)
    // {
    //     RoamerMoveToOtherLocationSet();
    // }
    // else
    // {
    //     struct Roamer *roamer = &saveRoamers[0];

    //     if (!roamer->active)
    //         return;

    //     while (locSet < (NELEMS(sRoamerLocations) - 1))
    //     {
    //         if (sRoamerLocation[MAP_NUM] == sRoamerLocations[locSet][0])
    //         {
    //             u8 mapNum;
    //             while (1)
    //             {
    //                 mapNum = sRoamerLocations[locSet][(Random() % 6) + 1];
    //                 if (!(sLocationHistory[2][MAP_GRP] == 3 && sLocationHistory[2][MAP_NUM] == mapNum) && mapNum != 0xFF)
    //                     break;
    //             }
    //             sRoamerLocation[MAP_NUM] = mapNum;
    //             return;
    //         }
    //         locSet++;
    //     }
    // }
}

void MoveRoamers()
{
  int i;
  for (i = 0; i < ROAMER_SPECIES_COUNT; i++)
  {
    struct RoamerInfo *roamerInfo = &roamerInfos[i];
    RoamerMove(roamerInfo);
  }
}

static bool8 IsRoamerAt(RoamerInfo *roamerInfo, u8 mapGroup, u8 mapNum)
{
  struct Roamer *roamer = &saveRoamers[roamerInfo->roamer_index];
  RoamerLocation *roamerLoc = &roamerInfo->sRoamerLocation;
  if (roamer->active && mapGroup == roamerLoc->group_number && mapNum == roamerLoc->map_number)
    return TRUE;
  else
    return FALSE;
}

static void CreateRoamerMonInstance(struct Roamer *roamer)
{
    struct Pokemon *mon;

    mon = &gEnemyParty[0];
    ZeroEnemyPartyMons();
    CreateMonWithIVsPersonality(mon, roamer->species, roamer->level, roamer->ivs, roamer->personality);
    SetMonData(mon, MON_DATA_STATUS, &roamer->status);
    SetMonData(mon, MON_DATA_HP, &roamer->hp);
    SetMonData(mon, MON_DATA_COOL, &roamer->cool);
    SetMonData(mon, MON_DATA_BEAUTY, &roamer->beauty);
    SetMonData(mon, MON_DATA_CUTE, &roamer->cute);
    SetMonData(mon, MON_DATA_SMART, &roamer->smart);
    SetMonData(mon, MON_DATA_TOUGH, &roamer->tough);
}

static struct RoamerInfo *get_roamer_by_species(u16 monSpecies)
{
    RoamerInfo *roamerInfo;
    int i;
    for (i = 0; i < ROAMER_SPECIES_COUNT; i++){
      if (monSpecies == roamer_types[i]){
        roamerInfo = &roamerInfos[i];
        break;
      }
    } 
    return roamerInfo;
}

static struct RoamerInfo *get_roamer_by_mon(struct Pokemon *mon)
{
  u16 monSpecies = GetMonData(mon, MON_DATA_SPECIES);
  return get_roamer_by_species(monSpecies);
}

bool8 TryStartRoamerEncounter()
{
  u8 randomRoamersIndexes[ROAMER_SPECIES_COUNT] = {};
  u8 randCount = 0;
  u8 i;
  u16 rand_index;
  u8 roamer_index;
  struct Roamer *roamer;

  for (i = 0; i < ROAMER_SPECIES_COUNT; i++){
    RoamerInfo *roamerInfo = &roamerInfos[i];
    if (IsRoamerAt(roamerInfo, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) == TRUE)
    // if (IsRoamerAt(roamerInfo, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) == TRUE && (Random() % 4) == 0)
    {
        randomRoamersIndexes[randCount] = i;
        randCount++;
    }
  }

  if (randCount == 0){
    return FALSE;
  }

  rand_index = (Random() % randCount);
  roamer_index = randomRoamersIndexes[rand_index];
  roamer = &saveRoamers[roamer_index];
  CreateRoamerMonInstance(roamer);
  return TRUE;
}

void UpdateRoamerHPStatus(struct Pokemon *mon)
{
   RoamerInfo *roamerInfo = get_roamer_by_mon(mon);
    struct Roamer *roamer = &saveRoamers[roamerInfo->roamer_index];
   roamer->hp = GetMonData(mon, MON_DATA_HP);
   roamer->status = GetMonData(mon, MON_DATA_STATUS);

   RoamerMoveToOtherLocationSet(roamerInfo);
}

void SetRoamerInactive(struct Pokemon *mon)
{
  RoamerInfo *roamerInfo = get_roamer_by_mon(mon);
  struct Roamer *roamer = &saveRoamers[roamerInfo->roamer_index];
  roamer->active = FALSE;
}

u16 GetRoamerLocationMapSectionId(u16 species)
{
    RoamerInfo *roamerInfo = get_roamer_by_species(species);
    RoamerLocation *roamerLoc = &roamerInfo->sRoamerLocation;
    struct Roamer *roamer = &saveRoamers[roamerInfo->roamer_index];
    if (!roamer->active){
        return MAPSEC_NONE;
    }
    return Overworld_GetMapHeaderByGroupAndId(roamerLoc->group_number, roamerLoc->map_number)->regionMapSectionId;
}
