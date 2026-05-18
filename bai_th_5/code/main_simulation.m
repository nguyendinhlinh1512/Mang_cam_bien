clear; clc; close all;

fprintf('==============================================\n');
fprintf('  MO PHONG: DUCF vs LEACH\n');
fprintf('==============================================\n\n');

%% THAM SO MO PHONG
params.N        = 100;          % So sensor nodes
params.width    = 200;          % Chieu rong vung ROI (m)
params.height   = 200;          % Chieu cao vung ROI (m)
params.E0       = 1;            % Nang luong ban dau moi node (Joules)
params.Eelec    = 50e-9;        % Nang luong dien tu: 50 nJ/bit
params.Efs      = 10e-12;       % He so khuech dai free-space: 10 pJ/bit/m^2
params.Emp      = 0.0013e-12;   % He so khuech dai multipath: 0.0013 pJ/bit/m^4
params.d0       = sqrt(params.Efs / params.Emp);  % Khoang cach nguong ~87m
params.EDA      = 5e-9;         % Nang luong tong hop du lieu: 5 nJ/bit
params.l        = 4000;         % So bit moi goi tin (500 bytes)
params.l_ctrl   = 200;          % So bit goi control (25 bytes)
params.p        = 0.1;          % Ty le CH mong muon trong LEACH
params.R_comm   = 40;           % Ban kinh truyen thong (m)
params.max_rounds = 1000;       % So vong toi da
params.num_sims = 5;            % So lan mo phong de lay trung binh

%% 3 KICH BAN BS 
% 1: BS o giua ROI
% 2: BS o goc ROI  
BS_positions = [100, 100;    % Scenario 1
                200, 200;    % Scenario 2
                100, 250];   % Scenario 3

scenario_names = {'Scenario 1: BS o giua ROI', ...
                  'Scenario 2: BS o goc ROI', ...
                  'Scenario 3: BS ngoai ROI'};

%% ---- LUU KET QUA ----
results_leach = struct();
results_ducf  = struct();

for sc = 1:3
    fprintf('\n--- Dang chay %s ---\n', scenario_names{sc});
    params.BS = BS_positions(sc, :);
    
    % Khoi tao mang tich luy ket qua
    leach_dead_all  = zeros(params.num_sims, params.max_rounds);
    ducf_dead_all   = zeros(params.num_sims, params.max_rounds);
    leach_energy_all = zeros(params.num_sims, params.max_rounds);
    ducf_energy_all  = zeros(params.num_sims, params.max_rounds);
    leach_msgs_all  = zeros(params.num_sims, 1);
    ducf_msgs_all   = zeros(params.num_sims, 1);
    
    for sim = 1:params.num_sims
        fprintf('  Simulation %d/%d...', sim, params.num_sims);
        
        % Tao cung topology node cho ca hai thuat toan
        rng(sim * sc * 42);  % Seed co dinh de tai lap
        node_positions = rand(params.N, 2) .* [params.width, params.height];
        
        % Chay LEACH
        [ld, le, lm] = run_leach(node_positions, params);
        leach_dead_all(sim, :)   = ld;
        leach_energy_all(sim, :) = le;
        leach_msgs_all(sim)      = lm;
        
        % Chay DUCF
        [dd, de, dm] = run_ducf(node_positions, params);
        ducf_dead_all(sim, :)   = dd;
        ducf_energy_all(sim, :) = de;
        ducf_msgs_all(sim)      = dm;
        
        fprintf(' Xong.\n');
    end
    
    % Lay trung binh qua cac lan mo phong
    results_leach(sc).dead   = mean(leach_dead_all, 1);
    results_leach(sc).energy = mean(leach_energy_all, 1);
    results_leach(sc).msgs   = mean(leach_msgs_all);
    
    results_ducf(sc).dead    = mean(ducf_dead_all, 1);
    results_ducf(sc).energy  = mean(ducf_energy_all, 1);
    results_ducf(sc).msgs    = mean(ducf_msgs_all);
    
    % Tinh FND va HND
    results_leach(sc).FND = find(results_leach(sc).dead >= 1, 1, 'first');
    results_leach(sc).HND = find(results_leach(sc).dead >= params.N/2, 1, 'first');
    results_ducf(sc).FND  = find(results_ducf(sc).dead >= 1, 1, 'first');
    results_ducf(sc).HND  = find(results_ducf(sc).dead >= params.N/2, 1, 'first');
    
    if isempty(results_leach(sc).FND), results_leach(sc).FND = params.max_rounds; end
    if isempty(results_leach(sc).HND), results_leach(sc).HND = params.max_rounds; end
    if isempty(results_ducf(sc).FND),  results_ducf(sc).FND = params.max_rounds; end
    if isempty(results_ducf(sc).HND),  results_ducf(sc).HND = params.max_rounds; end
end

%% ---- IN KET QUA TONG HOP ----
fprintf('\n\n========== KET QUA TONG HOP ==========\n');
fprintf('%-25s %-12s %-12s %-12s %-12s\n', ...
    'Thuat toan', 'Scenario', 'FND', 'HND', 'Msgs to BS');
fprintf('%s\n', repmat('-', 1, 73));
for sc = 1:3
    fprintf('%-25s %-12d %-12d %-12d %-12.0f\n', ...
        'LEACH', sc, results_leach(sc).FND, results_leach(sc).HND, results_leach(sc).msgs);
    fprintf('%-25s %-12d %-12d %-12d %-12.0f\n', ...
        'DUCF', sc, results_ducf(sc).FND, results_ducf(sc).HND, results_ducf(sc).msgs);
    fprintf('%s\n', repmat('-', 1, 73));
end

%% ---- VE DO THI ----
plot_results(results_leach, results_ducf, params, scenario_names, BS_positions);

fprintf('\n✓ Mo phong hoan tat! Cac do thi da duoc luu.\n');