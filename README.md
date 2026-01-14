# C Raycaster

A simple raycasting engine/thing made with C and SDL3.

## Installation

1. Install SDL3
2. Install GCC
3. Install Make
3. Clone this repository
4. Open the repository and run `make run`.

## Use

When you first open the program you will be in 'World Mode', Press Z to toggle between World Mode and 'Map Mode'.

In map mode, clicking twice will create a line between those two points where you clicked. You can place up to 100 lines.

In world mode, use the WASD keys to move around the world, and the arrows keys to adjust your rotation.

## Plans

1. Change the program to use the lines properly, currently they arent 'grabbed' by the raycaster and are just used as collision points.
2. Add textures to the lines.
3. Add collision to the lines, this should hopefully be quite easy.
4. Add portals
5. Maybe add sectors?? this project may be getting too big at that point.
