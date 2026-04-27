% =========================================================================
% Generování testovacích signálů pro DSP filtrace
% =========================================================================
clear; clc; close all;

%% 1. Parametry signálu
fs = 10000;         % Vzorkovací frekvence (10 kHz)
f1 = 50;   A1 = 1.0; % Nízkofrekvenční složka
f2 = 500;  A2 = 0.5; % Středněfrekvenční složka
f3 = 2500; A3 = 0.2; % Vysokofrekvenční složka
noise_power = 0.05;  % Výkon bílého šumu

%% 2. Definice velikostí datasetů
N_small = 1e3;      % 1 000 vzorků
N_medium = 1e5;     % 100 000 vzorků
N_large = 1e7;      % 10 000 000 vzorků

% Anonymní funkce pro generování signálu (aby se kód neopakoval)
generate_signal = @(N) A1*sin(2*pi*f1*(0:N-1)/fs) + ...
                       A2*sin(2*pi*f2*(0:N-1)/fs) + ...
                       A3*sin(2*pi*f3*(0:N-1)/fs) + ...
                       noise_power * randn(1, N);

%% 3. Generování signálů
disp('Generuji signály...');
sig_small  = generate_signal(N_small)';   % Transpozice na sloupcový vektor
%sig_medium = generate_signal(N_medium)';
%sig_large  = generate_signal(N_large)';

%% 4. Vykreslení malého signálu a jeho spektra pro kontrolu
t_small = (0:N_small-1)/fs;
figure('Name', 'Analýza malého signálu', 'Position', [100 100 800 400]);

% Časová oblast
subplot(1,2,1);
plot(t_small(1:200), sig_small(1:200)); % Zobrazíme jen prvních 200 vzorků
title('Časový průběh (výřez)');
xlabel('Čas [s]'); ylabel('Amplituda');
grid on;

% Frekvenční oblast (FFT)
subplot(1,2,2);
f_axis = linspace(0, fs/2, N_small/2);
sig_fft = abs(fft(sig_small))/N_small;
plot(f_axis, 2*sig_fft(1:N_small/2));
title('Amplitudové spektrum');
xlabel('Frekvence [Hz]'); ylabel('Amplituda');
grid on;

%% 5. Export do CSV
disp('Exportuji do CSV (to může u velkého datasetu chvíli trvat)...');

input_path = '../data/input/';
% writematrix je v novějších verzích Matlabu velmi rychlý
writematrix(sig_small, strcat(input_path, 'signal_small.csv'));
disp('signal_small.csv uložen.');

%writematrix(sig_medium, strcat(input_path, 'signal_medium.csv'));
disp('signal_medium.csv uložen.');

%writematrix(sig_large, strcat(input_path, 'signal_large.csv'));
disp('signal_large.csv uložen.');

disp('Hotovo!');
