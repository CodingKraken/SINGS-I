# Cosmology-Simulation
An pretty scuffed N-body physics simulation, using the Barnes-Hut Algorithm.

Requires Raylib, and is made to compile on linux. NOTE: Has only been tested on Ubuntu 22.04 and Manjaro 21.2.6!

To create a new simulation, run ./cosmology -p=#1 -f=#2, where #1 is the number of particles and #2 is the number of timesteps you want to simulate
If you wish to write the file to binary, add the flag -w= along with a name. The program will append .bin so doing so manually is not necessary
If you wish to read a file after compilation, use the -r= flag, include the .bin in the file name, and do not pass any other arguments.

This code is very bad, do not come screaming to me if you find something that triggers you, believe me, it urks me too. If you want to add on to it, feel free to do so.
