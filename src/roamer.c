#include "global.h"
#include "random.h"
#include "overworld.h"
#include "field_specials.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"

typedef struct RoamerLocation {
  u8 group_number;
  u8 map_number;
} RoamerLocation;

typedef struct RoamerInfo {
  struct Roamer roamer;
  RoamerLocation sRoamerLocation;
  RoamerLocation sLocationHistory[3];
} RoamerInfo;

// todo: fix roamer
// static EWRAM_DATA u8 sLocationHistory[3][2] = {};
// static EWRAM_DATA u8 sRoamerLocation[2] = {};
static EWRAM_DATA RoamerLocation sRoamerLocation = {};
#define LOCATION_HISTORY_COUNT 3
#define ROAMER_SPECIES_COUNT 3
static EWRAM_DATA RoamerLocation sLocationHistory[LOCATION_HISTORY_COUNT] = {};
static EWRAM_DATA RoamerInfo roamerInfos[ROAMER_SPECIES_COUNT] = {};

const u16 roamer_types[ROAMER_SPECIES_COUNT] = {SPECIES_ENTEI, SPECIES_SUICUNE, SPECIES_RAIKOU};
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
      // sRoamerLocation[MAP_GRP] = 0;
      // sRoamerLocation[MAP_NUM] = 0;
      roamerInfos[i].sRoamerLocation.group_number = 0;
      roamerInfos[i].sRoamerLocation.map_number = 0;
      sRoamerLocation.group_number = 0;
      sRoamerLocation.map_number = 0;
      for (j = 0; j < LOCATION_HISTORY_COUNT; j++)
      {
          // sLocationHistory[i][MAP_GRP] = 0;
          // sLocationHistory[i][MAP_NUM] = 0;
          sLocationHistory[j].group_number = 0;
          sLocationHistory[j].map_number = 0;
          roamerInfos[i].sLocationHistory[j].group_number = 0;
          roamerInfos[i].sLocationHistory[j].map_number = 0;
      }
    }
}

#define GetRoamerSpecies() ({\
    u16 a;\
    switch (GetStarterSpecies())\
    {\
    default:\
        a = SPECIES_RAIKOU;\
        break;\
    case SPECIES_BULBASAUR:\
        a = SPECIES_ENTEI;\
        break;\
    case SPECIES_CHARMANDER:\
        a = SPECIES_SUICUNE;\
        break;\
    }\
    a;\
})

static void CreateInitialRoamerMons(void)
{
  struct Pokemon *tmpMon;
  struct Roamer *tmpRoamer;

  int i;
  for (i = 0; i < ROAMER_SPECIES_COUNT; i++){
    tmpMon = &gEnemyParty[0];
    CreateMon(tmpMon, roamer_types[i], 50, 0x20, 0, 0, 0, 0);
    // tmpRoamer = &saveRoamers[i];
    tmpRoamer = &gSaveBlock1Ptr->roamers[i];
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
    // sRoamerLocation[MAP_NUM] = sRoamerLocations[Random() % (NELEMS(sRoamerLocations) - 1)][0];
    // u8 tempLocation= MAP_NUM(ROUTE1);
    // sRoamerLocation[MAP_NUM] = tempLocation;
    roamerInfos[i].sRoamerLocation.group_number = 3;
    roamerInfos[i].sRoamerLocation.map_number = sRoamerLocations[0][0];
    sRoamerLocation.group_number = 3;
    sRoamerLocation.map_number = sRoamerLocations[0][0];
    // sRoamerLocation[MAP_GRP] = 3;
    // sRoamerLocation[MAP_NUM] = sRoamerLocations[0][0];
  }
}

void InitRoamer(void)
{
    ClearRoamerData();
    CreateInitialRoamerMons();
}

// todo: fix roamer
void UpdateLocationHistoryForRoamer(void)
{
    sLocationHistory[2].group_number = sLocationHistory[1].group_number;
    sLocationHistory[2].map_number = sLocationHistory[1].map_number;
    sLocationHistory[1].group_number = sLocationHistory[0].group_number;
    sLocationHistory[1].map_number = sLocationHistory[0].map_number;
    sLocationHistory[0].group_number = gSaveBlock1Ptr->location.mapGroup;
    sLocationHistory[0].map_number = gSaveBlock1Ptr->location.mapNum;
}

// todo: fix roamer
void RoamerMoveToOtherLocationSet(void)
{
    sRoamerLocation.group_number = 3;
    sRoamerLocation.map_number = sRoamerLocations[0][0];
    // sRoamerLocation[MAP_GRP] = 3;
    // sRoamerLocation[MAP_NUM] = sRoamerLocations[0][0];

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


// todo: fix roamer
void RoamerMove(void)
{
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

static bool8 IsRoamerAt(struct Roamer *roamer, u8 mapGroup, u8 mapNum)
{
    // if (roamer->active && mapGroup == sRoamerLocation[MAP_GRP] && mapNum == sRoamerLocation[MAP_NUM])
    if (roamer->active && mapGroup == sRoamerLocation.group_number && mapNum == sRoamerLocation.map_number)
        return TRUE;
    else
        return FALSE;
}

void CreateRoamerMonInstance(struct Roamer *roamer)
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

static struct Roamer *get_roamer_by_species(u16 monSpecies){
    struct Roamer *roamer;
    int i;
    for (i = 0; i < ROAMER_SPECIES_COUNT; i++){
      if (monSpecies == roamer_types[i]){
        roamer = &saveRoamers[i];
        break;
      }
    } 
    return roamer;
}

static struct Roamer *get_roamer_by_mon(struct Pokemon *mon){
    u16 monSpecies = GetMonData(mon, MON_DATA_SPECIES);
    return get_roamer_by_species(monSpecies);
}

static struct Roamer *get_random_roamer(){
    u16 rand = Random() % ROAMER_SPECIES_COUNT;
    struct Roamer *roamer = &saveRoamers[rand];
    return roamer;
}

bool8 TryStartRoamerEncounter(void)
{
    struct Roamer *randomRoamer = get_random_roamer();
    // if (IsRoamerAt(randomRoamer, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) == TRUE)
    if (IsRoamerAt(randomRoamer, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) == TRUE && (Random() % 4) == 0)
    {
        CreateRoamerMonInstance(randomRoamer);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void UpdateRoamerHPStatus(struct Pokemon *mon)
{
    struct Roamer *roamer = get_roamer_by_mon(mon);
    roamer->hp = GetMonData(mon, MON_DATA_HP);
    roamer->status = GetMonData(mon, MON_DATA_STATUS);

    RoamerMoveToOtherLocationSet();
}

void SetRoamerInactive(struct Pokemon *mon)
{
    struct Roamer *roamer = get_roamer_by_mon(mon);
    roamer->active = FALSE;
}

u16 GetRoamerLocationMapSectionId(u16 species)
{
    struct Roamer *roamer = get_roamer_by_species(species);
    if (!roamer->active)
        return MAPSEC_NONE;
    // return Overworld_GetMapHeaderByGroupAndId(sRoamerLocation[MAP_GRP], sRoamerLocation[MAP_NUM])->regionMapSectionId;
    return Overworld_GetMapHeaderByGroupAndId(sRoamerLocation.group_number, sRoamerLocation.map_number)->regionMapSectionId;
}
