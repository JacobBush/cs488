# CS 488 - Assignment 4
# Jacob Bush - 20558637

## Compilation
This program is compiled in the usual way.

Run
```sh
premake4 gmake
make
./A4 sample.lua
```
in the `/A4` directory.

## Manual
The program runs as expected, with `./A4 FILENAME.lua` running the lua file FILENAME through the
ray tracing, and outputting the result to a .png file.

### Extra Feature
For my extra feature, I implemented Supersampling. Specifically, I used regular sampling, with samples
equal to the constant `NUM_SAMPLES` in the file `A4.cpp` (I take the NxN samples per pixel, where N
is the floor of the squareroot of `NUM_SAMPLES`).

By default, `NUM_SAMPLES = 16` (4x4 = 16 samples per pixel), and the image `sample-aa.png` 
was generated with this number of samples. Compare this image to `sample.png`, which was
generated with `NUM_SAMPLES = 1` and notice the smoothing of edges, especially around the border
of shadows in `sample-aa.png`.

If not specified, images were produced with 1 sample per pixel.

### Images
The following are the set of images included with this assignment. Each was produced with 1 sample
per pixel, unless otherwise specified.
 	- `nonhier.png` : a 512x512px image generated with `./A4 Assets/nonhier.lua`
 	- `macho-cows.png` : a 512x512px image generated with `./A4 Assets/macho-cows.lua`
 	- `simple-cows.png` : a 512x512px image generated with `./A4 Assets/simple-cows.lua`
 	- `macho-cows-bb.png` : a 512x512px image generated with `./A4 Assets/macho-cows.lua`, with the flag
 							`SPECIAL_BOUNDING_BOX_RENDERING` set to true in the file `Mesh.hpp`. Note that
 							the bounding boxes  surround only triangle meshes (and not simpler meshes such 
 							as spheres and cubes).
 	- `sample.png` : a 512x512px image generated with `./A4 sample.lua`
 	- `sample-aa.png` : a 512x512px image generated with `./A4 sample.lua`. This image was generated with
 						16 samples per pixel.
 	- `screenshot.png` : a 1024x1024px image generated with `./A4 sample.lua` and 16 samples per pixel.

### Unique Scene
The unique scene I created is stored in `sample.lua`. It is inspired by the old rhyme about
the cow jumping over the moon. There are three renders of this scene stored in the `A4` directory:
	- `sample.png`
 	- `sample-aa.png`
 	- `screenshot.png`
all of these as described above. Additional renders can be generated using the command
`./A4 sample.lua` (If, for example, a higher or lower resolution image is desired).

NOTE DID NOT HAVE TIME TO CREATE sample WITH AA - NO EXTRA FEATURE in PICTURES
