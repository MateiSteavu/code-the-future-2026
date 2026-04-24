%% main_vam.m  —  VAM Projection Pipeline Entry Point

clear; clc;

% ── Parameters ────────────────────────────────────────────────────────────
gridSize     = 128;
numAngles    = 360;
learningRate = 0.05;
iterations   = 10;
angles       = linspace(0, 359, numAngles);

% ── Pipeline ──────────────────────────────────────────────────────────────
[targetVolume, outputDir] = voxelize_stl(gridSize);

optimizedProjections = optimize_projections(targetVolume, angles, ...
                                            gridSize, learningRate, iterations);

save_projections(optimizedProjections, angles, outputDir);

visualize_results(targetVolume, optimizedProjections, angles, gridSize);