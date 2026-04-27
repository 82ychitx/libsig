% plot_benchmarks.m
% Visualizes benchmark performance between MATLAB and libsig (C)

% Data structure: [Filter, Impz, Conv, Freqz] (in milliseconds)
functions = categorical({'Filter', 'Impz', 'Conv', 'Freqz'});
functions = reordercats(functions, {'Filter', 'Impz', 'Conv', 'Freqz'});

% --- Signal Small Data ---
small_1k_matlab = [0.218, 0.517, 0.329, 1.239];
small_1k_c = [0.080, 0.041, 0.485, 0.329];

small_100k_matlab = [0.246, 10.731, 6.834, 47.201];
small_100k_c = [0.080, 3.967, 31.325, 49.455];

small_10m_matlab = [0.196, 710.258, 489.864, 4443.446];
small_10m_c = [0.084, 392.028, 13795.482, 21230.282];

% --- Signal Medium Data ---
medium_1k_matlab = [10.069, 0.369, 4.875, 1.139];
medium_1k_c = [5.141, 0.041, 28.763, 0.183];

medium_100k_matlab = [8.838, 5.913, 537.304, 47.121];
medium_100k_c = [4.616, 3.940, 73.471, 46.772];

medium_10m_matlab = [7.176, 566.866, 40542.599, 4512.007];
medium_10m_c = [4.608, 393.481, 14639.145, 22500.915];

% Plotting Function Setup
figure('Position', [100, 100, 1200, 800]);

% Plot 1: Signal Small (100k)
subplot(2, 2, 1);
b = bar(functions, [small_100k_matlab; small_100k_c]', 'grouped');
title('Signal Small - 100k Samples');
ylabel('Time [ms]');
legend('MATLAB', 'libsig (C)', 'Location', 'northeast');
set(gca, 'YScale', 'log'); % Log scale is useful for huge variances
grid on;

% Plot 2: Signal Small (10M)
subplot(2, 2, 2);
b = bar(functions, [small_10m_matlab; small_10m_c]', 'grouped');
title('Signal Small - 10M Samples');
ylabel('Time [ms]');
legend('MATLAB', 'libsig (C)', 'Location', 'northeast');
set(gca, 'YScale', 'log');
grid on;

% Plot 3: Signal Medium (100k)
subplot(2, 2, 3);
b = bar(functions, [medium_100k_matlab; medium_100k_c]', 'grouped');
title('Signal Medium - 100k Samples');
ylabel('Time [ms]');
legend('MATLAB', 'libsig (C)', 'Location', 'northeast');
set(gca, 'YScale', 'log');
grid on;

% Plot 4: Signal Medium (10M)
subplot(2, 2, 4);
b = bar(functions, [medium_10m_matlab; medium_10m_c]', 'grouped');
title('Signal Medium - 10M Samples');
ylabel('Time [ms]');
legend('MATLAB', 'libsig (C)', 'Location', 'northeast');
set(gca, 'YScale', 'log');
grid on;

sgtitle('Performance Benchmark: MATLAB vs libsig (Logarithmic Scale)');