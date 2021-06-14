# notes

## changes

- fast battle text speed
- messing with starters
- remove trade-based evolution
- allow espeon and umbreon evolutions
- add flag to hide previous quest log on load game

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
- load_save.c
  - continue game setup
- data/maps/
  - scripts and data for levels
- script macros
  - asm/macros/event.inc
  - script_cmd_table.inc
  - src/scrcmd.c

## scripting

- setvar dest_var, constant
- copyvar dest_var, src_var
- addbyte dest_var, constant
- addvar dest_var, constant
- setflag var
  - turn variable to true
- clearflag var
  - set variable to false

## todo

- gary should choose random
- release all 3 dogs as roamers
- release all 3 fr/lg starters as roamers
- release all 3 g/s/c starters as roamers
