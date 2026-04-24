%% visualize_results.m
function visualize_results(targetVolume, optimizedProjections, angles, gridSize)
%VISUALIZE_RESULTS  Compare target slice with simulated dose reconstruction.
%
%   visualize_results(targetVolume, optimizedProjections, angles, gridSize)

    numSlices = size(targetVolume, 3);
    midSlice  = round(numSlices / 2);

    % Reconstruct the mid-slice from its optimised sinogram
    midSinogram   = double(squeeze(optimizedProjections(:, :, midSlice)));
    simulatedDose = iradon(midSinogram, angles, 'none', gridSize);

    figure('Name', 'VAM Quality Check', 'NumberTitle', 'off');

    subplot(1,3,1);
    imagesc(targetVolume(:,:,midSlice));
    title(sprintf('Target (slice %d)', midSlice));
    axis image; colormap(gca, gray); colorbar;

    subplot(1,3,2);
    imagesc(simulatedDose);
    title('Simulated Dose (optimised)');
    axis image; colormap(gca, gray); colorbar;

    subplot(1,3,3);
    diff = targetVolume(:,:,midSlice) - simulatedDose;
    imagesc(diff);
    title('Difference (target − dose)');
    axis image; colormap(gca, jet); colorbar;

    sgtitle('VAM Reconstruction Quality Check');
end