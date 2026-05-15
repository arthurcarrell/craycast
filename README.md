# C Raycaster with Portals

This was my A-Level Coursework (NEA) for Computer Science. Its recommended to do your project in C#, but I did it in C.
I got 84% for the actual coursework (my conclusion/analysis should've been better), but the project itself got full marks.
This isnt really a game, but more of a tech demo.

The project allows you to load and edit custom maps/run them. You can create sectors, lines and portals to connect lines and sectors together.

## Installation

1. Install SDL3
2. Install GCC
3. Install Make
3. Clone this repository
4. Open the repository and run `make run`.

## Use

(There isnt actually a way to create maps from scratch in the project, so create an empty .map file in the maps directory in the project)

Press the 'Z' key to move between World Mode (walking around the map) and Map Mode (the map editor)

## Running the Map
Use WASD to move and the left and right arrow keys to look around.

## Editing a Map
At any time, while doing anything, pressing 'ESC' will cancel the task. Once you do something, you cannot undo it.

Clicking somewhere will enter sector creation mode. Click to define points on the map and then press S to join them all together to create a sector.

Press B and then click inside of a sector to set the spawn point of the player.

Pressing L and then clicking twice will create a standalone line in a sector. The line must be in bounds.

Clicking on a line will change its color.

### Portals
Press X and then click on a line to set it as the entrance for a portal. Click on another line to set that line as the exit. Portals are one-way, but you can make them two-way by doing the same process but in reverse.

Portals can connect to any line at any rotation, but they might start to look weird if one of the portals is larger than the other.


## Plans

Probably none, might come back to this once my A-Levels are completely finished.
