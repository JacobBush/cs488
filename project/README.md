# CS 488 - Project
# Jacob Bush - 20558637

## Compilation
This program is compiled in the usual way.

Run
```sh
premake4 gmake
make
./A4 Assets/torus.lua
```
in the `/project` directory. tours.lua can be replaced by other lua files.
Flags 

### Flags:
The following flags can be specified in `A4.cpp` to change renderring mode.
Change these flags and then re-run `make`.

	- NUM_CAMERA_POSITIONS: Number of samples taken for depth of field
	- MAX_HITS: Max number of hits allowed when tracing reflections / refractions
	- NUM_SAMPLES: Number of samples per pixel (antialiasing)
	- JITTERING: If true, randomly jitter each sample to reduce reduce regular errors
	- SPACE_PARTITIONING: If true, partition tree in auxiliarry octree, and ray trace through
						  this data structure. Will speed up renderring on scenes with many
						  nodes. Required for caustics.
	- PHOTON_MAPPING: If true, enable caustics.

### Optional
Run `make config=release` to speed up renderring times.

## Manual
The program runs as expected, with `./A4 FILENAME.lua` running the lua file FILENAME through the
ray tracing, and outputting the result to a .png file. The program is compiled to `A4`, despite
this no longer being Assignment 4.

## Objectives:
	1.  Primitives:  Torus & Cylinder
	2.  Texture Mapping
	3.  Bump Mapping
	4.  Reflection
	5.  Refraction
	6.  Caustics via Photon Mapping
	7.  Acceleration via Octree
	8.  Antialiasing
	9.  Depth of Field
	10. Final Scene
