#include "random.h"
#include "overworld.h"
#include "field_specials.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"

// Despite having a variable to track it, the roamer is
// hard-coded to only ever be in map group 3
#define ROAMER_MAP_GROUP 3
#define LOCATION_HISTORY_COUNT 3

typedef struct RoamerLocation
{
  u8 group_number;
  u8 map_number;
} RoamerLocation;

typedef struct RoamerHistory
{
  RoamerLocation sRoamerLocation;
  RoamerLocation sLocationHistory[3];
} RoamerHistory;

typedef struct RoamerInfo
{
  struct Roamer *roamer;
  RoamerHistory *roamerHistory;
} RoamerInfo;

typedef struct RoamerType
{
  u16 species;
  u8 level;
  bool8 random;
} RoamerType;

const RoamerType roamer_types[ROAMER_SPECIES_COUNT] = 
{
  {.species = SPECIES_ENTEI, .level = 50},
  {.species = SPECIES_SUICUNE, .level = 50},
  {.species = SPECIES_RAIKOU, .level = 50},
  {.species = SPECIES_BULBASAUR, .level = 5, .random = TRUE},
  {.species = SPECIES_SQUIRTLE, .level = 5, .random = TRUE},
  {.species = SPECIES_CHARMANDER, .level = 5, .random = TRUE}
};

static EWRAM_DATA RoamerHistory roamerHistories[ROAMER_SPECIES_COUNT] = {};

/** somehow prevents roamer pointer from dying */
#define saveRoamers (*(&gSaveBlock1Ptr->roamers))

#define ROAMER (&gSaveBlock1Ptr->roamer)
EWRAM_DATA u8 sLocationHistory[3][2] = {};
EWRAM_DATA u8 sRoamerLocation[2] = {};

#define ___ MAP_NUM(UNDEFINED) // For empty spots in the location table

// Note: There are two potential softlocks that can occur with this table if its maps are
//       changed in particular ways. They can be avoided by ensuring the following:
//       - There must be at least 2 location sets that start with a different map,
//         i.e. every location set cannot start with the same map. This is because of
//         the while loop in RoamerMoveToOtherLocationSet.
//       - Each location set must have at least 3 unique maps. This is because of
//         the while loop in RoamerMove. In this loop the first map in the set is
//         ignored, and an additional map is ignored if the roamer was there recently.
//       - Additionally, while not a softlock, it's worth noting that if for any
//         map in the location table there is not a location set that starts with
//         that map then the roamer will be significantly less likely to move away
//         from that map when it lands there.
static const u8 sRoamerLocations[][7] = {
    {MAP_NUM(ROUTE1), MAP_NUM(ROUTE2), MAP_NUM(ROUTE21_NORTH), MAP_NUM(ROUTE22), ___, ___, ___},
    {MAP_NUM(ROUTE2), MAP_NUM(ROUTE1), MAP_NUM(ROUTE3), MAP_NUM(ROUTE22), ___, ___, ___},
    {MAP_NUM(ROUTE3), MAP_NUM(ROUTE2), MAP_NUM(ROUTE4), ___, ___, ___, ___},
    {MAP_NUM(ROUTE4), MAP_NUM(ROUTE3), MAP_NUM(ROUTE5), MAP_NUM(ROUTE9), MAP_NUM(ROUTE24), ___, ___},
    {MAP_NUM(ROUTE5), MAP_NUM(ROUTE4), MAP_NUM(ROUTE6), MAP_NUM(ROUTE7), MAP_NUM(ROUTE8), MAP_NUM(ROUTE9), MAP_NUM(ROUTE24)},
    {MAP_NUM(ROUTE6), MAP_NUM(ROUTE5), MAP_NUM(ROUTE7), MAP_NUM(ROUTE8), MAP_NUM(ROUTE11), ___, ___},
    {MAP_NUM(ROUTE7), MAP_NUM(ROUTE5), MAP_NUM(ROUTE6), MAP_NUM(ROUTE8), MAP_NUM(ROUTE16), ___, ___},
    {MAP_NUM(ROUTE8), MAP_NUM(ROUTE5), MAP_NUM(ROUTE6), MAP_NUM(ROUTE7), MAP_NUM(ROUTE10), MAP_NUM(ROUTE12), ___},
    {MAP_NUM(ROUTE9), MAP_NUM(ROUTE4), MAP_NUM(ROUTE5), MAP_NUM(ROUTE10), MAP_NUM(ROUTE24), ___, ___},
    {MAP_NUM(ROUTE10), MAP_NUM(ROUTE8), MAP_NUM(ROUTE9), MAP_NUM(ROUTE12), ___, ___, ___},
    {MAP_NUM(ROUTE11), MAP_NUM(ROUTE6), MAP_NUM(ROUTE12), ___, ___, ___, ___},
    {MAP_NUM(ROUTE12), MAP_NUM(ROUTE10), MAP_NUM(ROUTE11), MAP_NUM(ROUTE13), ___, ___, ___},
    {MAP_NUM(ROUTE13), MAP_NUM(ROUTE12), MAP_NUM(ROUTE14), ___, ___, ___, ___},
    {MAP_NUM(ROUTE14), MAP_NUM(ROUTE13), MAP_NUM(ROUTE15), ___, ___, ___, ___},
    {MAP_NUM(ROUTE15), MAP_NUM(ROUTE14), MAP_NUM(ROUTE18), MAP_NUM(ROUTE19), ___, ___, ___},
    {MAP_NUM(ROUTE16), MAP_NUM(ROUTE7), MAP_NUM(ROUTE17), ___, ___, ___, ___},
    {MAP_NUM(ROUTE17), MAP_NUM(ROUTE16), MAP_NUM(ROUTE18), ___, ___, ___, ___},
    {MAP_NUM(ROUTE18), MAP_NUM(ROUTE15), MAP_NUM(ROUTE17), MAP_NUM(ROUTE19), ___, ___, ___},
    {MAP_NUM(ROUTE19), MAP_NUM(ROUTE15), MAP_NUM(ROUTE18), MAP_NUM(ROUTE20), ___, ___, ___},
    {MAP_NUM(ROUTE20), MAP_NUM(ROUTE19), MAP_NUM(ROUTE21_NORTH), ___, ___, ___, ___},
    {MAP_NUM(ROUTE21_NORTH), MAP_NUM(ROUTE1), MAP_NUM(ROUTE20), ___, ___, ___, ___},
    {MAP_NUM(ROUTE22), MAP_NUM(ROUTE1), MAP_NUM(ROUTE2), MAP_NUM(ROUTE23), ___, ___, ___},
    {MAP_NUM(ROUTE23), MAP_NUM(ROUTE22), MAP_NUM(ROUTE2), ___, ___, ___, ___},
    {MAP_NUM(ROUTE24), MAP_NUM(ROUTE4), MAP_NUM(ROUTE5), MAP_NUM(ROUTE9), ___, ___, ___},
    {MAP_NUM(ROUTE25), MAP_NUM(ROUTE24), MAP_NUM(ROUTE9), ___, ___, ___, ___},
    {___, ___, ___, ___, ___, ___, ___}
};

#undef ___
#define NUM_LOCATION_SETS (ARRAY_COUNT(sRoamerLocations) - 1)
#define NUM_LOCATIONS_PER_SET (ARRAY_COUNT(sRoamerLocations[0]))

void ClearRoamerData(void)
{
    u8 i;
    for (i = 0; i < ROAMER_SPECIES_COUNT; i++)
    {
      //*ROAMER = (struct Roamer){};
      gSaveBlock1Ptr->roamers[i] = (struct Roamer){};
      roamerHistories[i] = (RoamerHistory){};
    }
}

static void CreateInitialRoamerMons()
{
  struct Pokemon *tmpMon;
  struct Roamer *tmpRoamer;

  u8 i;
  for (i = 0; i < ROAMER_SPECIES_COUNT; i++){
    RoamerType roamer_type = roamer_types[i];

    tmpMon = &gEnemyParty[0];
    CreateMon(tmpMon, roamer_type.species, roamer_type.level, 0x20, 0, 0, 0, 0);
    tmpRoamer = &saveRoamers[i];
    tmpRoamer->species = roamer_type.species;
    tmpRoamer->level = roamer_type.level;
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
    roamerHistories[i].sRoamerLocation.group_number = 3;
    roamerHistories[i].sRoamerLocation.map_number = sRoamerLocations[Random() % (NELEMS(sRoamerLocations) - 1)][0];
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
  u8 i;
  for (i = 0; i < ROAMER_SPECIES_COUNT; i++)
  {
    struct RoamerHistory *roamerHistory = &roamerHistories[i];
    UpdateLocationHistoryForRoamer(roamerHistory->sLocationHistory);
  }
}

/** move far */
static void RoamerMoveToOtherLocationSet(RoamerInfo *roamerInfo)
{
    u8 mapNum = 0;

    if (!roamerInfo->roamer->active)
        return;

    roamerInfo->roamerHistory->sRoamerLocation.group_number = 3;

    // Choose a location set that starts with a map
    // different from the roamer's current map
    while (1)
    {
        mapNum = sRoamerLocations[Random() % (NELEMS(sRoamerLocations) - 1)][0];
        if (roamerInfo->roamerHistory->sRoamerLocation.map_number != mapNum)
        {
            roamerInfo->roamerHistory->sRoamerLocation.map_number = mapNum;
            return;
        }
    }
}

static struct RoamerInfo get_roamer_by_index(u8 index)
{
    RoamerHistory *roamerHistory;
    struct Roamer *roamer;
    RoamerInfo roamerInfo ;

    roamerHistory = &roamerHistories[index];
    roamer = &saveRoamers[index];

    roamerInfo = (RoamerInfo){.roamer = roamer, .roamerHistory = roamerHistory};
    return roamerInfo;
}

void MoveRoamersToOtherLocationSet()
{
  u8 i;
  for (i = 0; i < ROAMER_SPECIES_COUNT; i++)
  {
    struct RoamerInfo roamerInfo = get_roamer_by_index(i);
    RoamerMoveToOtherLocationSet(&roamerInfo);
  }
}

/** move near or far */
static void RoamerMove(RoamerInfo *roamerInfo)
{
    u8 locSet = 0;
    u8 oldestGroup;
    u8 oldestMap;

    if ((Random() % 16) == 0)
    {
        RoamerMoveToOtherLocationSet(roamerInfo);
    }
    else
    {
        if (!roamerInfo->roamer->active)
            return;

        while (locSet < NUM_LOCATION_SETS)
        {
            // Find the location set that starts with the roamer's current map
            if (roamerInfo->roamerHistory->sRoamerLocation.map_number == sRoamerLocations[locSet][0])
            {
                u8 mapNum;
                while (1)
                {
                    // Choose a new map (excluding the first) within this set
                    // Also exclude a map if the roamer was there 2 moves ago
                    mapNum = sRoamerLocations[locSet][(Random() % (NUM_LOCATIONS_PER_SET - 1)) + 1];
                    // mapNum = sRoamerLocations[locSet][(Random() % 6) + 1];
                    oldestGroup = roamerInfo->roamerHistory->sLocationHistory[2].group_number;
                    oldestMap = roamerInfo->roamerHistory->sLocationHistory[2].map_number;
                    //if (!(sLocationHistory[2][MAP_GRP] == ROAMER_MAP_GROUP
                    //   && sLocationHistory[2][MAP_NUM] == mapNum)
                    //   && mapNum != MAP_NUM(UNDEFINED))
                    if (!(oldestGroup == 3 && oldestMap == mapNum) && mapNum != 0xFF)
                    {
                        break;
                    }
                }
                roamerInfo->roamerHistory->sRoamerLocation.map_number = mapNum;
                return;
            }
            locSet++;
        }
    }
}

void MoveRoamers()
{
  u8 i;
  for (i = 0; i < ROAMER_SPECIES_COUNT; i++)
  {
    struct RoamerInfo roamerInfo = get_roamer_by_index(i);
    RoamerMove(&roamerInfo);
  }
}

static bool8 IsRoamerAt(RoamerInfo *roamerInfo, u8 mapGroup, u8 mapNum)
{
  struct Roamer *roamer = roamerInfo->roamer;
  RoamerLocation *roamerLoc = &roamerInfo->roamerHistory->sRoamerLocation;
  if (roamer->active && mapGroup == roamerLoc->group_number && mapNum == roamerLoc->map_number)
    return TRUE;
  else
    return FALSE;
}

static void CreateRoamerMonInstance(struct Roamer *roamer)
{
    //u32 status;
    struct Pokemon *mon;

    mon = &gEnemyParty[0];
    ZeroEnemyPartyMons();
    CreateMonWithIVsPersonality(mon, roamer->species, roamer->level, roamer->ivs, roamer->personality);
// The roamer's status field is u8, but SetMonData expects status to be u32, so will set the roamer's status
// using the status field and the following 3 bytes (cool, beauty, and cute).
#if defined(BUGFIX) || IV_BUGFIX == TRUE
    status = ROAMER->status;
    SetMonData(mon, MON_DATA_STATUS, &status);
#else
    SetMonData(mon, MON_DATA_STATUS, &roamer->status);
#endif
    SetMonData(mon, MON_DATA_HP, &roamer->hp);
    SetMonData(mon, MON_DATA_COOL, &roamer->cool);
    SetMonData(mon, MON_DATA_BEAUTY, &roamer->beauty);
    SetMonData(mon, MON_DATA_CUTE, &roamer->cute);
    SetMonData(mon, MON_DATA_SMART, &roamer->smart);
    SetMonData(mon, MON_DATA_TOUGH, &roamer->tough);
}

static struct RoamerInfo get_roamer_by_species(u16 monSpecies)
{
    RoamerInfo roamerInfo;

    u8 i;
    for (i = 0; i < ROAMER_SPECIES_COUNT; i++){
      if (monSpecies == roamer_types[i].species){
        roamerInfo = get_roamer_by_index(i);
        break;
      }
    }

    return roamerInfo;
}

static struct RoamerInfo get_roamer_by_mon(struct Pokemon *mon)
{
  u16 monSpecies = GetMonData(mon, MON_DATA_SPECIES);
  return get_roamer_by_species(monSpecies);
}

// bool8 TryStartRoamerEncounter()
// {
//   u8 i;

//   for (i = 0; i < ROAMER_SPECIES_COUNT; i++){
//     RoamerInfo roamerInfo = get_roamer_by_index(i);
//     if (IsRoamerAt(&roamerInfo, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) == TRUE)
//     {
//         CreateRoamerMonInstance(roamerInfo.roamer);
//         return TRUE;
//     }
//   }

//   return FALSE;
// }

bool8 TryStartRoamerEncounter()
{
  struct Roamer *randomRoamers[ROAMER_SPECIES_COUNT] = {};
  u8 randCount = 0;
  u8 i;
  u16 rand_index;
  u8 roamer_index;
  struct Roamer *roamer;

  for (i = 0; i < ROAMER_SPECIES_COUNT; i++){
    RoamerInfo roamerInfo = get_roamer_by_index(i);
    if (IsRoamerAt(&roamerInfo, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) == TRUE)
    {
        randomRoamers[randCount] = roamerInfo.roamer;
        randCount++;
    }
  }

  if (randCount == 0){
    return FALSE;
  }
  if ((Random() % 4) != 0){
    return FALSE;
  }

  rand_index = (Random() % randCount);
  roamer = randomRoamers[rand_index];
  CreateRoamerMonInstance(roamer);
  return TRUE;
}

void UpdateRoamerHPStatus(struct Pokemon *mon)
{
   RoamerInfo roamerInfo = get_roamer_by_mon(mon);
   roamerInfo.roamer->hp = GetMonData(mon, MON_DATA_HP);
   roamerInfo.roamer->status = GetMonData(mon, MON_DATA_STATUS);

   RoamerMoveToOtherLocationSet(&roamerInfo);
}

void SetRoamerInactive(struct Pokemon *mon)
{
  RoamerInfo roamerInfo = get_roamer_by_mon(mon);
  roamerInfo.roamer->active = FALSE;
}

u16 GetRoamerLocationMapSectionId(u16 species)
{
    RoamerInfo roamerInfo = get_roamer_by_species(species);
    RoamerLocation *roamerLoc = &roamerInfo.roamerHistory->sRoamerLocation;
    if (!roamerInfo.roamer->active){
        return MAPSEC_NONE;
    }
    return Overworld_GetMapHeaderByGroupAndId(roamerLoc->group_number, roamerLoc->map_number)->regionMapSectionId;
}

u8 GetRoamerMapNumber(u8 index)
{
    return roamerHistories[index].sRoamerLocation.map_number;
}
