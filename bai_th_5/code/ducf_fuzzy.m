function [chance, size_out] = ducf_fuzzy(residual_energy, node_degree, dist_to_BS, params)
%% =========================================================
%  HỆ THỐNG SUY LUẬN MỜ (FIS) CỦA DUCF
%  Mamdani Fuzzy Inference + Centroid (COA) Defuzzification
%% =========================================================
%
%  NGUYÊN LÝ FUZZY LOGIC TRONG DUCF:
%  1. FUZZIFICATION: Chuyển giá trị crisp → fuzzy linguistic variables
%     - Residual energy: Low / Medium / High
%     - Node degree:     Less / Average / Huge  
%     - Distance to BS:  Nearby / Reachable / Far
%
%  2. INFERENCE: Áp dụng 27 luật IF-THEN (Bảng 1 trong bài báo)
%     Dùng phương pháp Mamdani (min cho AND, max cho OR)
%
%  3. DEFUZZIFICATION: Chuyển fuzzy output → crisp output
%     Dùng phương pháp Centroid (Center of Area - COA)
%     - chance : xác suất node trở thành CH
%     - size   : số member tối đa cluster có thể chứa
%
%  INPUT (đã chuẩn hóa):
%    residual_energy : [0, 1]    (tỷ lệ năng lượng còn lại)
%    node_degree     : số hàng xóm trong bán kính R
%    dist_to_BS      : khoảng cách đến BS (m)
%    params          : struct tham số
%
%  OUTPUT:
%    chance   : [0, 100] - xác suất làm CH
%    size_out : [0, 30]  - kích thước cluster tối đa
%% =========================================================

%% ---- BƯỚC 1: FUZZIFICATION ----
% Tính mức độ thuộc (membership degree) cho từng biến đầu vào

% --- (1a) Residual Energy: [0, 1] ---
% Low:    trapezoid (0, 0, 0.2, 0.4)
% Medium: triangle  (0.2, 0.5, 0.8)
% High:   trapezoid (0.6, 0.8, 1, 1)
mu_E_low    = trapMF(residual_energy, 0, 0, 0.2, 0.4);
mu_E_medium = triMF(residual_energy,  0.2, 0.5, 0.8);
mu_E_high   = trapMF(residual_energy, 0.6, 0.8, 1.0, 1.0);

% --- (1b) Node Degree: [0, 30] hàng xóm ---
% Less:    trapezoid (0, 0, 4, 8)
% Average: triangle  (4, 12, 20)
% Huge:    trapezoid (15, 22, 30, 30)
mu_ND_less    = trapMF(node_degree, 0, 0, 4, 8);
mu_ND_average = triMF(node_degree,  4, 12, 20);
mu_ND_huge    = trapMF(node_degree, 15, 22, 30, 30);

% --- (1c) Distance to BS: [0, 300] m ---
% Nearby:    trapezoid (0, 0, 30, 70)
% Reachable: triangle  (40, 100, 160)
% Far:       trapezoid (120, 180, 300, 300)
max_dist = sqrt(params.width^2 + params.height^2);  % ~283m cho 200x200
mu_D_nearby    = trapMF(dist_to_BS, 0,  0,   0.25*max_dist, 0.45*max_dist);
mu_D_reachable = triMF(dist_to_BS,  0.2*max_dist, 0.5*max_dist, 0.8*max_dist);
mu_D_far       = trapMF(dist_to_BS, 0.55*max_dist, 0.75*max_dist, max_dist, max_dist);

%% ---- BƯỚC 2: RULE BASE (27 luật - Bảng 1 bài báo) ----
% Index chance: 1=VL,2=L,3=RL,4=LM,5=M,6=HM,7=RH,8=H,9=VH
% Index size:   1=VS,2=S,3=RS,4=M,5=RB,6=B,7=VB
%
% Mỗi hàng: [mu_E, mu_ND, mu_D, chance_idx, size_idx]
rule_table = [
    mu_E_high,   mu_ND_huge,    mu_D_nearby,    9, 5;   % R1
    mu_E_high,   mu_ND_huge,    mu_D_reachable, 8, 6;   % R2
    mu_E_high,   mu_ND_huge,    mu_D_far,       7, 7;   % R3
    mu_E_high,   mu_ND_average, mu_D_nearby,    9, 4;   % R4
    mu_E_high,   mu_ND_average, mu_D_reachable, 8, 4;   % R5
    mu_E_high,   mu_ND_average, mu_D_far,       7, 4;   % R6
    mu_E_high,   mu_ND_less,    mu_D_nearby,    9, 1;   % R7
    mu_E_high,   mu_ND_less,    mu_D_reachable, 8, 2;   % R8
    mu_E_high,   mu_ND_less,    mu_D_far,       7, 3;   % R9
    mu_E_medium, mu_ND_huge,    mu_D_nearby,    6, 5;   % R10
    mu_E_medium, mu_ND_huge,    mu_D_reachable, 5, 6;   % R11
    mu_E_medium, mu_ND_huge,    mu_D_far,       4, 7;   % R12
    mu_E_medium, mu_ND_average, mu_D_nearby,    6, 4;   % R13
    mu_E_medium, mu_ND_average, mu_D_reachable, 5, 4;   % R14
    mu_E_medium, mu_ND_average, mu_D_far,       4, 4;   % R15
    mu_E_medium, mu_ND_less,    mu_D_nearby,    6, 1;   % R16
    mu_E_medium, mu_ND_less,    mu_D_reachable, 5, 2;   % R17
    mu_E_medium, mu_ND_less,    mu_D_far,       4, 3;   % R18
    mu_E_low,    mu_ND_huge,    mu_D_nearby,    3, 5;   % R19
    mu_E_low,    mu_ND_huge,    mu_D_reachable, 2, 6;   % R20
    mu_E_low,    mu_ND_huge,    mu_D_far,       1, 7;   % R21
    mu_E_low,    mu_ND_average, mu_D_nearby,    3, 4;   % R22
    mu_E_low,    mu_ND_average, mu_D_reachable, 2, 4;   % R23
    mu_E_low,    mu_ND_average, mu_D_far,       1, 4;   % R24
    mu_E_low,    mu_ND_less,    mu_D_nearby,    3, 1;   % R25
    mu_E_low,    mu_ND_less,    mu_D_reachable, 2, 2;   % R26
    mu_E_low,    mu_ND_less,    mu_D_far,       1, 3;   % R27
];

% Tính aggregated strength theo Mamdani (min cho AND, max cho aggregation)
chance_strength = zeros(1, 9);
size_strength   = zeros(1, 7);

for k = 1:27
    strength  = min([rule_table(k,1), rule_table(k,2), rule_table(k,3)]);
    ch_idx    = rule_table(k, 4);
    sz_idx    = rule_table(k, 5);
    chance_strength(ch_idx) = max(chance_strength(ch_idx), strength);
    size_strength(sz_idx)   = max(size_strength(sz_idx),   strength);
end

%% ---- BƯỚC 3: DEFUZZIFICATION (Centroid / COA method) ----

% --- Defuzzify CHANCE ---
chance_range  = 0:0.5:100;
chance_centers = [5, 15, 27, 38, 50, 62, 73, 85, 95];
chance_width   = 11;
agg_chance = zeros(size(chance_range));
for c = 1:9
    if chance_strength(c) > 0
        mf_vals = zeros(size(chance_range));
        for xi = 1:length(chance_range)
            mf_vals(xi) = min(chance_strength(c), ...
                triMF(chance_range(xi), chance_centers(c)-chance_width, ...
                      chance_centers(c), chance_centers(c)+chance_width));
        end
        agg_chance = max(agg_chance, mf_vals);
    end
end
if sum(agg_chance) > 0
    chance = sum(chance_range .* agg_chance) / sum(agg_chance);
else
    chance = 50;
end

% --- Defuzzify SIZE ---
size_range   = 0:0.2:30;
size_centers = [2, 5, 9, 14, 19, 24, 28];
size_width   = 4;
agg_size = zeros(size(size_range));
for s = 1:7
    if size_strength(s) > 0
        mf_vals = zeros(size(size_range));
        for xi = 1:length(size_range)
            mf_vals(xi) = min(size_strength(s), ...
                triMF(size_range(xi), size_centers(s)-size_width, ...
                      size_centers(s), size_centers(s)+size_width));
        end
        agg_size = max(agg_size, mf_vals);
    end
end
if sum(agg_size) > 0
    size_out = sum(size_range .* agg_size) / sum(agg_size);
    size_out = max(2, round(size_out));
else
    size_out = 5;
end

end  % end function ducf_fuzzy

%% ======================================================
%  HÀM MEMBERSHIP (Membership Functions)
%% ======================================================

function mu = trapMF(x, a, b, c, d)
% Hàm membership hình thang: a≤b≤c≤d
% Bằng 0 ngoài [a,d], tăng từ a đến b, bằng 1 trong [b,c], giảm từ c đến d
if x <= a || x >= d
    mu = 0;
elseif x >= b && x <= c
    mu = 1;
elseif x > a && x < b
    mu = (x - a) / (b - a);
else
    mu = (d - x) / (d - c);
end
mu = max(0, min(1, mu));
end

function mu = triMF(x, a, b, c)
% Hàm membership hình tam giác: a≤b≤c
% Bằng 0 ngoài [a,c], đỉnh = 1 tại b
if x <= a || x >= c
    mu = 0;
elseif x < b
    mu = (x - a) / (b - a);
else
    mu = (c - x) / (c - b);
end
mu = max(0, min(1, mu));
end