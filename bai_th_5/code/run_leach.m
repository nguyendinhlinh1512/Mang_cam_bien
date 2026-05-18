function [dead_per_round, energy_per_round, total_msgs] = run_leach(node_pos, params)
%% =========================================================
%  THUẬT TOÁN LEACH
%  Low Energy Adaptive Clustering Hierarchy
%  Heizelman et al., IEEE Trans. Wireless Commun., 2002
%% =========================================================
%
%  NGUYÊN LÝ LEACH:
%  - Mỗi node tự bầu mình làm CH với xác suất p/(1 - p*(r mod 1/p))
%  - CH truyền dữ liệu trực tiếp đến BS (single-hop)
%  - Mỗi vòng: (1) Setup phase: bầu CH, thành lập cluster
%              (2) Steady-state phase: thu thập và gửi dữ liệu
%
%  INPUT:
%    node_pos : [N x 2] tọa độ các nodes
%    params   : struct chứa tham số mô phỏng
%
%  OUTPUT:
%    dead_per_round  : số node chết theo từng vòng
%    energy_per_round: tổng năng lượng tiêu thụ mỗi vòng
%    total_msgs      : tổng số gói dữ liệu gửi đến BS trước HND
%% =========================================================

N = params.N;
BS = params.BS;
p = params.p;
max_rounds = params.max_rounds;

%% --- Khởi tạo mạng ---
energy = ones(N, 1) * params.E0;   % Năng lượng hiện tại mỗi node
alive  = true(N, 1);               % Trạng thái sống/chết
dead_per_round   = zeros(1, max_rounds);
energy_per_round = zeros(1, max_rounds);
total_msgs = 0;
HND_reached = false;

% Tính khoảng cách từ mỗi node đến BS
dist_to_BS = sqrt((node_pos(:,1) - BS(1)).^2 + (node_pos(:,2) - BS(2)).^2);

% Biến lưu round cuối mỗi node được chọn làm CH
last_ch_round = -ones(N, 1) * (1/p);  % Cho phép bầu ngay từ đầu

%% --- Vòng lặp chính ---
for r = 1:max_rounds
    alive_nodes = find(alive);
    n_alive = length(alive_nodes);
    
    if n_alive == 0, break; end
    
    %% PHASE 1: SETUP - Bầu chọn Cluster Head
    % Mỗi node tính xác suất T(n) theo công thức LEACH (Eq.1 trong bài báo):
    %   T(n) = p / (1 - p * (r mod (1/p)))  nếu node chưa làm CH trong 1/p vòng gần nhất
    %        = 0 nếu đã làm CH rồi
    
    is_CH = false(N, 1);
    
    for i = alive_nodes'
        % Kiểm tra node có đủ điều kiện bầu không
        rounds_since_ch = r - last_ch_round(i);
        if rounds_since_ch >= ceil(1/p)
            % Tính ngưỡng T(n)
            Tn = p / (1 - p * mod(r-1, ceil(1/p)));
            % So sánh với giá trị ngẫu nhiên
            if rand() < Tn
                is_CH(i) = true;
                last_ch_round(i) = r;
            end
        end
    end
    
    CH_list = find(is_CH & alive);
    
    % Nếu không có CH nào được bầu, chọn ngẫu nhiên 1 node
    if isempty(CH_list)
        idx = alive_nodes(randi(n_alive));
        is_CH(idx) = true;
        CH_list = idx;
    end
    
    %% PHASE 1: Gán member nodes vào CH gần nhất
    cluster_of = zeros(N, 1);  % cluster_of(i) = CH mà node i thuộc về
    
    for i = alive_nodes'
        if ~is_CH(i)
            % Node không phải CH → tìm CH gần nhất (theo RSSI)
            if isempty(CH_list)
                cluster_of(i) = 0;
            else
                d_to_CH = sqrt((node_pos(i,1) - node_pos(CH_list,1)).^2 + ...
                               (node_pos(i,2) - node_pos(CH_list,2)).^2);
                [~, best] = min(d_to_CH);
                cluster_of(i) = CH_list(best);
            end
        else
            cluster_of(i) = i;  % CH tự quản lý mình
        end
    end
    
    %% PHASE 2: STEADY STATE - Thu thập và truyền dữ liệu
    round_energy = 0;
    
    % (A) Chi phí Control messages trong Setup phase
    % CH quảng bá CH_ADV, members gửi JOIN_REQ
    for ch = CH_list'
        if ~alive(ch), continue; end
        % CH gửi quảng bá (1 gói control)
        d_ch_bs = dist_to_BS(ch);
        e_adv = energy_tx(params.l_ctrl, d_ch_bs, params);
        energy(ch) = energy(ch) - e_adv;
        round_energy = round_energy + e_adv;
    end
    
    for i = alive_nodes'
        if is_CH(i), continue; end
        if cluster_of(i) == 0, continue; end
        ch = cluster_of(i);
        d = sqrt((node_pos(i,1)-node_pos(ch,1))^2 + (node_pos(i,2)-node_pos(ch,2))^2);
        e_join = energy_tx(params.l_ctrl, d, params);
        energy(i) = energy(i) - e_join;
        round_energy = round_energy + e_join;
    end
    
    % (B) Members gửi data đến CH của mình
    for i = alive_nodes'
        if is_CH(i) || cluster_of(i) == 0, continue; end
        if ~alive(i), continue; end
        
        ch = cluster_of(i);
        d = sqrt((node_pos(i,1)-node_pos(ch,1))^2 + (node_pos(i,2)-node_pos(ch,2))^2);
        
        % Năng lượng phát của member
        e_tx = energy_tx(params.l, d, params);
        energy(i) = energy(i) - e_tx;
        round_energy = round_energy + e_tx;
        
        % Năng lượng thu của CH
        if alive(ch)
            e_rx = params.l * params.Eelec;
            energy(ch) = energy(ch) - e_rx;
            round_energy = round_energy + e_rx;
        end
    end
    
    % (C) CH tổng hợp dữ liệu (Data Aggregation)
    for ch = CH_list'
        if ~alive(ch), continue; end
        
        % Đếm số members
        members = find(cluster_of == ch & alive & ~is_CH);
        n_members = length(members);
        
        % Năng lượng tổng hợp dữ liệu
        e_da = params.EDA * params.l * (n_members + 1);
        energy(ch) = energy(ch) - e_da;
        round_energy = round_energy + e_da;
        
        % (D) CH truyền dữ liệu đến BS (LEACH dùng single-hop)
        d_bs = dist_to_BS(ch);
        e_tx_bs = energy_tx(params.l, d_bs, params);
        energy(ch) = energy(ch) - e_tx_bs;
        round_energy = round_energy + e_tx_bs;
        
        % Đếm số tin nhắn gửi đến BS (trước HND)
        if ~HND_reached
            total_msgs = total_msgs + 1;
        end
    end
    
    %% Cập nhật trạng thái node chết
    newly_dead = alive & (energy <= 0);
    alive(newly_dead) = false;
    energy(newly_dead) = 0;
    
    dead_count = sum(~alive);
    dead_per_round(r) = dead_count;
    energy_per_round(r) = round_energy;
    
    % Kiểm tra điều kiện HND
    if dead_count >= N/2 && ~HND_reached
        HND_reached = true;
    end
end

% Điền đầy mảng (nếu mạng chết trước max_rounds)
for r = 2:max_rounds
    if dead_per_round(r) == 0 && dead_per_round(r-1) > 0
        dead_per_round(r) = dead_per_round(r-1);
    end
end

end

%% ---- HÀM PHỤ TRỢ: Tính năng lượng phát ----
function E = energy_tx(l, d, params)
% Mô hình năng lượng truyền (Eq. 11-12 trong bài báo):
%   Etx = l*Eelec + l*eps_fs*d^2  nếu d < d0 (free space)
%   Etx = l*Eelec + l*eps_mp*d^4  nếu d >= d0 (multipath)
if d < params.d0
    E = l * params.Eelec + l * params.Efs * d^2;
else
    E = l * params.Eelec + l * params.Emp * d^4;
end
end