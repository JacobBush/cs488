# CS 488 - Assignment 2
# Jacob Bush - 20558637

## Compilation
This program is compiled in the usual way.

Run
```sh
premake4 gmake
make
./A3 puppet.lua
```
in the `/A3` directory.

## Manual
This assignment was completed to be in line with the assignment specifications.

## Changes to Data Structures
There were small changes made to GeometryNode, JointNode and SceneNode.
These changes were small, and there should be little to no interface changes
required.

FragmentShader.fs was modified to allow for picking

## Puppet Overview
The puppet I built was very close to the provided puppet.lua file.
The puppet is humanoid, and can be interacted with at:
 - Neck
 - Head (nodding and rotating)
 - Shoulder (x2)
 - Elbow (x2)
 - Wrist (x2)
 - Hip (x2)
 - Knee (x2)
 - Ankle (x2)
This gives a total of 15 degrees of freedom, as per the assignment spec
