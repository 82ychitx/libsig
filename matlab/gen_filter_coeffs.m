clear; clc;

[b1, a1] = butter(4, 0.2); % Filtr 1: Butterworth dolní propust
[b2, a2] = cheby1(4, 0.5, 0.4); % Filtr 2: Čebyšev dolní propust

filter_coeffs = [b1; a1; b2; a2;];
input_path = '../data/input/';
writematrix(filter_coeffs, strcat(input_path, 'filter_coeffs.csv'));
disp('filter_coeffs.csv uložen.');