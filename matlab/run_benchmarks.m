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
filter_coeffs = readmatrix('../data/input/filter_coeffs.csv');
% TODO: Is there a better way to do this?
b1 = filter_coeffs(1, :);
a1 = filter_coeffs(2, :);
b2 = filter_coeffs(3, :);
a2 = filter_coeffs(4, :);


% Impulsní odezva prvního filtru (např. 100 vzorků) pro test konvoluce
h = impz(b1, a1, 100); 

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
y_conv = conv(signal, h, 'same');
time_conv = toc;

% ---------------------------------------------------------
% B. Funkce pro analýzu a manipulaci se systémy (filtry)
% Tyto nezávisí na signálu, pracují jen s b, a koeficienty.
% Pro přesnější měření velmi rychlých operací bychom mohli použít smyčku,
% ale pro základní srovnání s C stačí jedno proběhnutí.
% ---------------------------------------------------------

% 3. Funkce FREQZ (Frekvenční charakteristika, např. 1024 bodů)
tic;
[H, w] = freqz(b1, a1, 1024);
time_freqz = toc;

% 4. Funkce SERIES (Sériové řazení dvou filtrů)
tic;
[b_series, a_series] = series(b1, a1, b2, a2);
time_series = toc;

% 5. Funkce PARALLEL (Paralelní řazení dvou filtrů)
tic;
[b_parallel, a_parallel] = parallel(b1, a1, b2, a2);
time_parallel = toc;

% 6. Funkce FEEDBACK (Zpětnovazební spojení dvou filtrů)
tic;
[b_feedback, a_feedback] = feedback(b1, a1, b2, a2);
time_feedback = toc;


%% 3. VÝPIS VÝSLEDKŮ
% Vytiskneme úhlednou tabulku pro snadné srovnání s C
fprintf('====================================================\n');
fprintf('Funkce       | Doba trvání [s] | Doba trvání [ms] \n');
fprintf('====================================================\n');
fprintf('filter       | %13.6f | %14.3f \n', time_filter, time_filter * 1000);
fprintf('conv         | %13.6f | %14.3f \n', time_conv, time_conv * 1000);
fprintf('----------------------------------------------------\n');
fprintf('freqz        | %13.8f | %14.5f \n', time_freqz, time_freqz * 1000);
fprintf('series       | %13.8f | %14.5f \n', time_series, time_series * 1000);
fprintf('parallel     | %13.8f | %14.5f \n', time_parallel, time_parallel * 1000);
fprintf('feedback     | %13.8f | %14.5f \n', time_feedback, time_feedback * 1000);
fprintf('====================================================\n');