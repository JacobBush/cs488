#!/bin/sh
echo "Small space partitioning (10^3 = 1000 nodes):"
time ./A4 Assets/space_partitioning_small.lua
echo "Medium space partitioning (20^3 = 8000 nodes):"
time ./A4 Assets/space_partitioning_medium.lua
echo "Large space partitioning (50^3 = 125000 nodes):"
time ./A4 Assets/space_partitioning_large.lua 
echo "End of Test."
