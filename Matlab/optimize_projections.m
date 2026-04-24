%% optimize_projections.m
function optimizedProjections = optimize_projections(targetVolume, angles, ...
                                                      gridSize, learningRate, ...
                                                      iterations)
%OPTIMIZE_PROJECTIONS  Iteratively refine sinogram projections per slice.
%
%   optimizedProjections = optimize_projections(targetVolume, angles,
%                              gridSize, learningRate, iterations)
%
%   Inputs:
%       targetVolume  - Binary voxel volume (gridSize x gridSize x numSlices)
%       angles        - 1-D array of projection angles in degrees
%       gridSize      - Reconstruction grid size (pixels)
%       learningRate  - Gradient descent step size (e.g. 0.05)
%       iterations    - Number of refinement iterations per slice (e.g. 10)
%
%   Output:
%       optimizedProjections - single array (numRays x numAngles x numSlices)
%                              normalised to [0, 1]

    numSlices = size(targetVolume, 3);
    numAngles = numel(angles);

    % Probe radon output size on a blank slice
    tempR   = radon(zeros(gridSize), angles);
    numRays = size(tempR, 1);

    optimizedProjections = zeros(numRays, numAngles, numSlices, 'single');

    fprintf('Starting optimization: %d slices, %d iterations each...\n', ...
            numSlices, iterations);
    tStart = tic;

    for z = 1:numSlices
        targetSlice = targetVolume(:,:,z);

        % Skip empty slices
        if ~any(targetSlice(:)), continue; end

        % Initial guess via forward Radon transform
        currentProj = radon(targetSlice, angles);

        % Gradient-descent refinement
        for iter = 1:iterations
            simulatedDose = iradon(currentProj, angles, 'none', gridSize);
            errorUpdate   = radon(targetSlice - simulatedDose, angles);
            currentProj   = max(currentProj + learningRate * errorUpdate, 0);
        end

        optimizedProjections(:,:,z) = single(currentProj);

        if mod(z, 20) == 0
            elapsed = toc(tStart);
            fprintf('  Slice %4d / %d  (%.1f s elapsed)\n', z, numSlices, elapsed);
        end
    end

    % Global normalisation to [0, 1]
    maxVal = max(optimizedProjections(:));
    if maxVal > 0
        optimizedProjections = optimizedProjections / maxVal;
    end

    fprintf('Optimization complete in %.1f s.\n', toc(tStart));
end