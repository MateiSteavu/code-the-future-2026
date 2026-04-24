%% voxelize_stl.m
function [targetVolume, outputDir] = voxelize_stl(gridSize)
%VOXELIZE_STL  Load an STL file and voxelize it into a binary volume.
%
%   [targetVolume, outputDir] = voxelize_stl(gridSize)
%
%   Inputs:
%       gridSize    - Scalar; number of voxels along each axis (e.g. 512)
%
%   Outputs:
%       targetVolume - gridSize^3 binary double array
%       outputDir    - Path to the 'projections' subfolder next to the STL

    [file, path] = uigetfile('*.stl', 'Select STL file');
    if isequal(file, 0)
        error('No file selected. Aborting.');
    end

    fprintf('Loading STL: %s\n', file);
    model = stlread(fullfile(path, file));

    % Build uniform grid spanning the model bounding box
    xRange = linspace(min(model.Points(:,1)), max(model.Points(:,1)), gridSize);
    [X, Y, Z] = meshgrid(xRange, xRange, xRange);

    % Alpha-shape based inside test
    shp          = alphaShape(model.Points(:,1), ...
                               model.Points(:,2), ...
                               model.Points(:,3));
    targetVolume = double(inShape(shp, X, Y, Z));

    fprintf('Voxelization complete. Volume size: %dx%dx%d\n', ...
            gridSize, gridSize, size(targetVolume,3));

    % Create output directory alongside the STL
    outputDir = fullfile(path, 'projections');
    if ~exist(outputDir, 'dir')
        mkdir(outputDir);
    end
end