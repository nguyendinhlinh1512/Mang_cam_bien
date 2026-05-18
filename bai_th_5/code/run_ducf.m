function [dead_per_round, energy_per_round, total_msgs] = run_ducf(node_pos, params)
%% =========================================================
%  THUẬT TOÁN DUCF
%  Distributed Unequal Clustering using Fuzzy logic
%  Baranidharan & Santhi, Applied Soft Computing, 2016
%% =========================================================
%
%  NGUYÊN LÝ DUCF:
%  1. Mọi node đều là "probationary CH" ban đầu
%  2. Mỗi node tính (chance, size) qua FIS với 3 đầu vào:
%       - Residual energy, Node degree, Distance to BS
%  3. Node có chance cao nhất trong vùng bán kính R → Final CH
%  4. CH chỉ nhận tối đa 'size' members (unequal clustering)
%  5. CH gần BS → size nhỏ (dành energy relay đa chặng)
%     CH xa BS  → size lớn (tổng hợp nhiều dữ liệu)
%  6. Truyền dữ liệu đến BS theo multi-hop
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

N          = params.N;
BS         = params.BS;
max_rounds = params.max_rounds;
R          = params.R_comm;   % Bán kính truyền thông 40m

%% --- Khởi tạo mạng ---
energy = ones(N, 1) * params.E0;
alive  = true(N, 1);
dead_per_round   = zeros(1, max_rounds);
energy_per_round = zeros(1, max_rounds);
total_msgs  = 0;
HND_reached = false;

% Tính khoảng cách từ mỗi node đến BS
dist_to_BS = sqrt((node_pos(:,1)-BS(1)).^2 + (node_pos(:,2)-BS(2)).^2);

% Tính ma trận khoảng cách node-node (tính 1 lần cho nhanh)
dist_matrix = zeros(N, N);
for i = 1:N
    for j = i+1:N
        d = sqrt((node_pos(i,1)-node_pos(j,1))^2 + (node_pos(i,2)-node_pos(j,2))^2);
        dist_matrix(i,j) = d;
        dist_matrix(j,i) = d;
    end
end

%% --- Vòng lặp chính ---
for r = 1:max_rounds
    alive_nodes = find(alive);
    n_alive = length(alive_nodes);
    if n_alive == 0, break; end

    %% ---- SUB-PHASE 1: CH ELECTION ----
    % Bước 1: Tính node degree (số hàng xóm còn sống trong bán kính R)
    node_deg = zeros(N, 1);
    for i = alive_nodes'
        mask = (dist_matrix(i, alive_nodes) <= R) & (alive_nodes ~= i)';
        neighbors = alive_nodes(mask);
        node_deg(i) = length(neighbors);
    end

    % Bước 2: Mỗi node chạy FIS để tính chance và size
    chance_val = zeros(N, 1);
    size_val   = zeros(N, 1);
    for i = alive_nodes'
        re = energy(i) / params.E0;   % Chuẩn hóa [0,1]
        nd = node_deg(i);
        db = dist_to_BS(i);
        [chance_val(i), size_val(i)] = ducf_fuzzy(re, nd, db, params);
    end

    % Bước 3: Mỗi node broadcast CH_CANDIDATE và so sánh
    % Node nào có chance cao nhất trong vùng R của nó → Final CH
    is_CH = false(N, 1);
    for i = alive_nodes'
        % Tìm các node trong bán kính R
        mask_R = (dist_matrix(i, alive_nodes) <= R);
        neighbors_in_R = alive_nodes(mask_R);
        % Nếu node i có chance cao nhất trong vùng → trở thành CH
        if chance_val(i) >= max(chance_val(neighbors_in_R))
            is_CH(i) = true;
        end
    end

    CH_list = find(is_CH & alive);

    % Đảm bảo có ít nhất 1 CH
    if isempty(CH_list)
        [~, best_node] = max(chance_val .* alive);
        is_CH(best_node) = true;
        CH_list = best_node;
    end

    %% ---- SUB-PHASE 2: CLUSTER BUILDING ----
    % Mỗi non-CH node gửi CM_JOIN đến CH gần nhất
    % CH chấp nhận nếu count < size (CM_ACCEPTANCE), ngược lại CM_REJECTION
    cluster_of  = zeros(N, 1);   % cluster_of(i) = CH index node i thuộc về
    ch_count    = zeros(N, 1);   % Số member hiện tại của mỗi CH

    for i = alive_nodes'
        if is_CH(i)
            cluster_of(i) = i;
            continue;
        end

        % Sắp xếp CH theo khoảng cách tăng dần
        if isempty(CH_list), continue; end
        d_to_CHs = dist_matrix(i, CH_list);
        [sorted_d, sort_idx] = sort(d_to_CHs);
        sorted_CHs = CH_list(sort_idx);

        joined = false;
        for k = 1:length(sorted_CHs)
            ch = sorted_CHs(k);
            % Kiểm tra size: CH nhận nếu chưa đầy
            if ch_count(ch) < size_val(ch)
                cluster_of(i) = ch;
                ch_count(ch)  = ch_count(ch) + 1;
                joined = true;
                break;
            end
        end

        % Nếu không join được CH nào → tự trở thành CH (worst case)
        if ~joined
            is_CH(i) = true;
            cluster_of(i) = i;
            CH_list = [CH_list; i];
        end
    end

    %% ---- DATA COLLECTION PHASE ----
    round_energy = 0;

    % Chi phí control messages (CH_CANDIDATE, CH_WON, CM_JOIN, CM_ACCEPTANCE)
    for i = alive_nodes'
        % Mỗi node broadcast CH_CANDIDATE trong bán kính R
        e_ctrl = energy_tx(params.l_ctrl, R, params);
        energy(i) = energy(i) - e_ctrl;
        round_energy = round_energy + e_ctrl;
    end

    % (A) Members gửi data đến CH
    for i = alive_nodes'
        if is_CH(i) || cluster_of(i) == 0, continue; end
        ch = cluster_of(i);
        if ~alive(ch), continue; end

        d = dist_matrix(i, ch);
        % Năng lượng phát của member node
        e_tx = energy_tx(params.l, d, params);
        energy(i) = energy(i) - e_tx;
        round_energy = round_energy + e_tx;

        % Năng lượng nhận của CH
        e_rx = params.l * params.Eelec;
        energy(ch) = energy(ch) - e_rx;
        round_energy = round_energy + e_rx;
    end

    % (B) CH tổng hợp dữ liệu và gửi lên BS qua multi-hop
    % Sắp xếp CH theo khoảng cách đến BS (gần nhất trước)
    if ~isempty(CH_list)
        valid_CH = CH_list(logical(alive(CH_list)));
        if ~isempty(valid_CH)
            [~, sort_idx] = sort(dist_to_BS(valid_CH));
            sorted_CH = valid_CH(sort_idx);

            for k = 1:length(sorted_CH)
                ch = sorted_CH(k);
                if ~alive(ch), continue; end

                % Data aggregation energy
                members = find(cluster_of == ch & alive & ~is_CH);
                n_m = length(members);
                e_da = params.EDA * params.l * (n_m + 1);
                energy(ch) = energy(ch) - e_da;
                round_energy = round_energy + e_da;

                % Multi-hop: tìm next hop CH hoặc gửi thẳng đến BS
                d_bs = dist_to_BS(ch);
                if d_bs <= R
                    % Gửi thẳng đến BS
                    e_tx = energy_tx(params.l, d_bs, params);
                    energy(ch) = energy(ch) - e_tx;
                    round_energy = round_energy + e_tx;
                else
                    % Tìm CH khác gần BS hơn để relay
                    other_CHs = sorted_CH(1:k-1);  % CHs đã được sort (gần BS hơn)
                    if isempty(other_CHs)
                        % Không có relay → gửi thẳng
                        e_tx = energy_tx(params.l, d_bs, params);
                        energy(ch) = energy(ch) - e_tx;
                        round_energy = round_energy + e_tx;
                    else
                        % Tìm CH relay gần nhất
                        d_to_relays = dist_matrix(ch, other_CHs);
                        [min_d, relay_idx] = min(d_to_relays);
                        relay_ch = other_CHs(relay_idx);

                        % Gửi đến relay CH
                        e_tx = energy_tx(params.l, min_d, params);
                        energy(ch) = energy(ch) - e_tx;
                        round_energy = round_energy + e_tx;

                        % Relay CH nhận
                        if alive(relay_ch)
                            e_rx = params.l * params.Eelec;
                            energy(relay_ch) = energy(relay_ch) - e_rx;
                            round_energy = round_energy + e_rx;
                        end
                    end
                end

                % Đếm messages đến BS
                if ~HND_reached
                    total_msgs = total_msgs + 1;
                end
            end
        end
    end

    %% Cập nhật trạng thái
    newly_dead = alive & (energy <= 0);
    alive(newly_dead) = false;
    energy(newly_dead) = 0;

    dead_count = sum(~alive);
    dead_per_round(r) = dead_count;
    energy_per_round(r) = round_energy;

    if dead_count >= N/2 && ~HND_reached
        HND_reached = true;
    end
end

% Điền đầy mảng dead sau khi mạng chết
for r = 2:max_rounds
    if dead_per_round(r) == 0 && dead_per_round(r-1) > 0
        dead_per_round(r) = dead_per_round(r-1);
    end
end

end

%% ---- Hàm tính năng lượng phát ----
function E = energy_tx(l, d, params)
if d < params.d0
    E = l * params.Eelec + l * params.Efs * d^2;
else
    E = l * params.Eelec + l * params.Emp * d^4;
end
end