%% save_projections.m
function save_projections(optimizedProjections, angles, outputDir)
%SAVE_PROJECTIONS  Write one tab-delimited .txt per angle and save raw .mat
%
%   Each .txt file contains the 2D projection image for one angle:
%       rows = detector ray positions  (numRays)
%       cols = build-axis slices       (numSlices)
%   Values are raw single-precision floats in [0, 1] — no conversion.
%
%   Inputs:
%       optimizedProjections - single array (numRays x numAngles x numSlices)
%                              normalised to [0, 1]
%       angles               - 1-D array of angles (used for count only)
%       outputDir            - Folder where outputs are written
%
%   Outputs on disk:
%       <outputDir>/proj_angle_000.txt  ...  proj_angle_359.txt
%       <outputDir>/projections_data.mat  (raw .mat v7.3)

    numAngles = numel(angles);

    % ── 1. Save raw data ──────────────────────────────────────────────────
    matFile = fullfile(outputDir, 'projections_data.mat');
    fprintf('Saving raw data to: %s\n', matFile);
    save(matFile, 'optimizedProjections', 'angles', '-v7.3');

    % ── 2. Save one .txt per angle ────────────────────────────────────────
    fprintf('Saving %d .txt projection files to: %s\n', numAngles, outputDir);
    tStart = tic;

    for a = 1:numAngles
        % (numRays x numSlices) matrix of floats in [0, 1]
        img = double(squeeze(optimizedProjections(:, a, :)));

        filename = fullfile(outputDir, sprintf('proj_angle_%03d.txt', a - 1));

        fid = fopen(filename, 'w');
        if fid == -1
            error('Could not open file for writing: %s', filename);
        end

        % Write row by row; values tab-separated, newline at end of each row
        for row = 1:size(img, 1)
            fprintf(fid, '%.6f\t', img(row, :));
            fprintf(fid, '\n');
        end

        fclose(fid);

        if mod(a, 60) == 0
            fprintf('  Saved %d / %d files (%.1f s)\n', a, numAngles, toc(tStart));
        end
    end

    fprintf('All .txt files saved in %.1f s.\n', toc(tStart));
end