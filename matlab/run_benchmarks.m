% =========================================================================
% Benchmark DSP funkcí (Měření času běhu)
% =========================================================================
clear; clc;

%% 1. PŘÍPRAVNÁ FÁZE (NEMĚŘÍ SE)
disp('--- PŘÍPRAVNÁ FÁZE ---');

% Výběr souboru uživatelem
[file, path] = uigetfile('*.csv', 'Vyberte CSV soubor s testovacím signálem');
if isequal(file, 0)
    disp('Výběr souboru byl zrušen.');
    return;
end

filename = fullfile(path, file);
disp(['Načítám soubor: ', file, ' ... (čekejte)']);
signal = readmatrix(filename);
N_samples = length(signal);
disp(['Načteno ', num2str(N_samples), ' vzorků.']);

% Příprava parametrů filtrů pro testování
% (Vytvoříme dva jednoduché filtry 4. řádu, abychom měli co spojovat)

disp('Načítám koeficienty filtru a generuji impulsní odezvu...');
[b1, a1] = butter(10, 0.2); % Filtr 1: Butterworth dolní propust
[b2, a2] = cheby1(10, 0.5, 0.4); % Filtr 2: Čebyšev dolní propust

filter_coeffs = [b1; a1; b2; a2;];
input_path = '../data/input/';
writematrix(filter_coeffs, strcat(input_path, 'filter_coeffs.csv'));
disp('filter_coeffs.csv uložen.');

% Impulsní odezva prvního filtru (např. 1000 vzorků) pro test konvoluce
tic;
h = impz(b1, a1, 1000);
time_impz = toc;

writematrix(h, '../data/output/impz_result.csv');

disp('Příprava dokončena. Spouštím benchmark...');
disp(' ');
%% 2. MĚŘENÍ ČASU (BENCHMARK)
disp('--- VÝSLEDKY MĚŘENÍ ---');

% ---------------------------------------------------------
% A. Funkce zpracovávající samotný signál
% ---------------------------------------------------------

% 1. Funkce FILTER (IIR/FIR filtrace)
tic;
y_filter = filter(b1, a1, signal);
time_filter = toc;

writematrix(y_filter, '../data/output/filter_result.csv');

% 2. Funkce CONV (Konvoluce signálu s impulsní odezvou)
% (Používáme parametr 'same', aby měl výstup stejnou délku jako vstup)
tic;
y_conv = conv(signal, h, 'full');
time_conv = toc;

writematrix(y_conv, '../data/output/conv_result.csv');

% ---------------------------------------------------------
% B. Funkce pro analýzu a manipulaci se systémy (filtry)
% Tyto nezávisí na signálu, pracují jen s b, a koeficienty.
% Pro přesnější měření velmi rychlých operací bychom mohli použít smyčku,
% ale pro základní srovnání s C stačí jedno proběhnutí.
% ---------------------------------------------------------

% 3. Funkce FREQZ (Frekvenční charakteristika, např. 1024 bodů)
omega = linspace(0, pi, 1000);
tic;
H = freqz(b1, a1, omega);
time_freqz = toc;

writematrix(omega', '../data/input/normalized_freqs.csv');
writematrix(H.', '../data/output/freqz_result.csv');

% 4. Funkce SERIES (Sériové řazení dvou filtrů)
tic;
[b_series, a_series] = series(b1, a1, b2, a2);
time_series = toc;
writematrix(b_series', '../data/output/series_result_n.csv');
writematrix(a_series', '../data/output/series_result_d.csv');


% 5. Funkce PARALLEL (Paralelní řazení dvou filtrů)
tic;
[b_parallel, a_parallel] = parallel(b1, a1, b2, a2);
time_parallel = toc;
writematrix(b_parallel', '../data/output/parallel_result_n.csv');
writematrix(a_parallel', '../data/output/parallel_result_d.csv');

% 6. Funkce FEEDBACK (Zpětnovazební spojení dvou filtrů)
tic;
[b_feedback, a_feedback] = feedback(b1, a1, b2, a2);
time_feedback = toc;
writematrix(b_feedback', '../data/output/feedback_result_n.csv');
writematrix(a_feedback', '../data/output/feedback_result_d.csv');


%% 3. VÝPIS VÝSLEDKŮ
% Vytiskneme úhlednou tabulku pro snadné srovnání s C
fprintf('====================================================\n');
fprintf('Funkce       | Doba trvání [s] | Doba trvání [ms] \n');
fprintf('====================================================\n');
fprintf('filter       | %13.6f | %14.3f \n', time_filter, time_filter * 1000);
fprintf('impz         | %13.6f | %14.3f \n', time_impz, time_impz * 1000);
fprintf('conv         | %13.6f | %14.3f \n', time_conv, time_conv * 1000);
fprintf('----------------------------------------------------\n');
fprintf('freqz        | %13.8f | %14.5f \n', time_freqz, time_freqz * 1000);
fprintf('series       | %13.8f | %14.5f \n', time_series, time_series * 1000);
fprintf('parallel     | %13.8f | %14.5f \n', time_parallel, time_parallel * 1000);
fprintf('feedback     | %13.8f | %14.5f \n', time_feedback, time_feedback * 1000);
fprintf('====================================================\n');
