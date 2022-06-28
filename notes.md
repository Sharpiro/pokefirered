# notes

## compiler quirks

- all function variables must be declared before modifying any of them
- `#define saveRoamers (*(&gSaveBlock1Ptr->roamers)`
  - magic macro that somehow prevents roamer pointer from dying
  - a normal global pointer pointing at roamer(s) will go bad somehow
  - possible to use a double pointer instead?
- initialize non-save block memory!!!

## points of interest

- starter zone script
  - data/maps/PalletTown_ProfessorOaksLab/scripts.inc
- include/constants/vars.h
  - variables
- main_menu.c
  - follow code from game start
- evolution.h
  - evolution table
- species.h
  - species list
- new_game.c
  - new game setup
  - default settings
- load_save.c
  - continue game setup
- data/maps/
  - scripts and data for levels
- script macros
  - asm/macros/event.inc
  - script_cmd_table.inc
  - src/scrcmd.c
- script specials
  - data/specials.inc
  - src/field_specials.c
- roaming pokemon
  - src/roamer.c
- wild encounters
  - src/data/wild_encounters.json

## scripting

- setvar dest_var, constant
  - set dest_var to constant
- copyvar dest_var, src_var
  - set dest_var to src_var
- addvar dest_var, constant
  - add constant to dest_var
- setflag var
  - turn variable to true
- clearflag var
  - set variable to false

## text

- `\p` creates a whole new text window
- `\n` creates a new line within an existing text window

## questions

- how can important data be saved in temporary flags?
  - probably can't...

## changes

- fast battle text speed
- sane shiny encounter rate
- first starter pokeball pulls from a random list of starters
- remove trade-based evolution
- allow espeon and umbreon evolutions
- add flag to hide previous quest log on load game
- release roamer after first rival battle

## hacks to try

- pokemon adventure red chapter
  - follows original manga
- pokemon ash gray
  - follows original anime
- pokemon liquid crystal
  - crystal remake w/ extra zone
- pokemon theta emerald ex (renev)
  - follows emerald w/ all pokemon
- pokemon rocket edition
  - play as team rocket
- pokemon radical red
  - fire red hard mode
- pokemon unbound
  - fire red hack w/ new content
- pokemon gaia
  - ...

## todo

- release all 3 g/s/c starters as roamers
- L2 view temp vars in memory
- gary should choose random
- remove time played

## done

- ~~release all 3 dogs as roamers~~
  - ~~test `UpdateRoamerHPStatus`~~
  - ~~test `SetRoamerInactive`~~
  - ~~add raikou's current location to a sign~~
  - ~~test removing original roamer prop from save block~~
- add static save block size assertion
- ~~release all 3 fr/lg starters as roamers~~