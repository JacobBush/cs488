#!/bin/sh
echo "Start of Test."
echo "Tiny space partitioning (10^3 = 1000 nodes):"
time ./A4 Assets/space_partitioning_tiny.lua
echo "Small space partitioning (50^3 = 125000 nodes):"
time ./A4 Assets/space_partitioning_small.lua
echo "Medium space partitioning (100^3 = 1000000 nodes):"
time ./A4 Assets/space_partitioning_medium.lua
echo "Large space partitioning (200^3 = 8000000 nodes):"
time ./A4 Assets/space_partitioning_large.lua 
echo "End of Test."
