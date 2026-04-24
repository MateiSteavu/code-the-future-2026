%% visualize_projection.m
function visualize_projection(outputDir, angleIndex)
%VISUALIZE_PROJECTION  Load and display a single projection .txt file.
%
%   visualize_projection(outputDir, angleIndex)
%
%   Inputs:
%       outputDir   - Folder containing the proj_angle_XXX.txt files
%       angleIndex  - Zero-based angle index to display (0–359)
%
%   Example:
%       visualize_projection('C:/my_print/projections', 45)

    if nargin < 2
        angleIndex = 0;   % default to first angle
    end

    % ── Load ──────────────────────────────────────────────────────────────
    filename = fullfile(outputDir, sprintf('proj_angle_%03d.txt', angleIndex));
    if ~isfile(filename)
        error('File not found: %s', filename);
    end

    fprintf('Loading: %s\n', filename);
    img = readmatrix(filename);

    % ── Plot ──────────────────────────────────────────────────────────────
    figure('Name', sprintf('Projection — Angle %d°', angleIndex), ...
           'NumberTitle', 'off');

    subplot(1,2,1);
    imagesc(img);
    axis image;
    colormap(gca, gray);
    colorbar;
    title(sprintf('Projection image  (angle %d°)', angleIndex));
    xlabel('Slice index (z)');
    ylabel('Detector ray position');

    subplot(1,2,2);
    histogram(img(:), 256, 'FaceColor', [0.2 0.5 0.8], 'EdgeColor', 'none');
    xlabel('Intensity value [0–1]');
    ylabel('Pixel count');
    title('Intensity distribution');
    grid on;

    sgtitle(sprintf('proj\\_angle\\_%03d.txt', angleIndex));

    fprintf('Size: %d rays x %d slices  |  min: %.4f  max: %.4f\n', ...
            size(img,1), size(img,2), min(img(:)), max(img(:)));
end