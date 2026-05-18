function plot_results(results_leach, results_ducf, params, scenario_names, BS_positions)
%% =========================================================
%  VẼ ĐỒ THỊ KẾT QUẢ MÔ PHỎNG
%  So sánh DUCF vs LEACH trên 3 kịch bản
%% =========================================================

colors = struct('leach', [0.85 0.33 0.10], 'ducf', [0.00 0.45 0.74]);
lw = 2.0;

%% ======================================================
%  HÌNH 1: Network lifetime - Số node chết theo vòng
%  (Tương đương Figs. 17-19 trong bài báo)
%% ======================================================
fig1 = figure('Name','Network Lifetime','Position',[50 50 1400 420]);
set(fig1,'Color','white');

for sc = 1:3
    subplot(1, 3, sc);
    r_leach = results_leach(sc).dead;
    r_ducf  = results_ducf(sc).dead;

    % Tìm điểm HND để cắt trục x
    hnd_leach = results_leach(sc).HND;
    hnd_ducf  = results_ducf(sc).HND;
    x_max = min(max(hnd_leach, hnd_ducf) + 100, params.max_rounds);

    rounds = 1:x_max;
    plot(rounds, r_leach(1:x_max), '-', 'Color', colors.leach, ...
        'LineWidth', lw, 'DisplayName', 'LEACH'); hold on;
    plot(rounds, r_ducf(1:x_max),  '-', 'Color', colors.ducf,  ...
        'LineWidth', lw, 'DisplayName', 'DUCF');

    % Đánh dấu FND và HND
    fnd_l = results_leach(sc).FND; fnd_d = results_ducf(sc).FND;
    hnd_l = results_leach(sc).HND; hnd_d = results_ducf(sc).HND;

    plot(fnd_l, 1,           'v', 'Color', colors.leach, 'MarkerSize', 8, 'MarkerFaceColor', colors.leach);
    plot(fnd_d, 1,           'v', 'Color', colors.ducf,  'MarkerSize', 8, 'MarkerFaceColor', colors.ducf);
    plot(hnd_l, params.N/2,  's', 'Color', colors.leach, 'MarkerSize', 8, 'MarkerFaceColor', colors.leach);
    plot(hnd_d, params.N/2,  's', 'Color', colors.ducf,  'MarkerSize', 8, 'MarkerFaceColor', colors.ducf);

    % Chú thích FND/HND
    text(fnd_l+5, 3,  sprintf('FND=%d',fnd_l), 'Color',colors.leach,'FontSize',8);
    text(fnd_d+5, 7,  sprintf('FND=%d',fnd_d), 'Color',colors.ducf, 'FontSize',8);
    text(hnd_l+5, params.N/2+2, sprintf('HND=%d',hnd_l), 'Color',colors.leach,'FontSize',8);
    text(hnd_d+5, params.N/2-5, sprintf('HND=%d',hnd_d), 'Color',colors.ducf, 'FontSize',8);

    xlabel('Số vòng (Rounds)', 'FontSize', 11);
    ylabel('Số node chết', 'FontSize', 11);
    title(scenario_names{sc}, 'FontSize', 11, 'FontWeight', 'bold');
    legend('LEACH','DUCF','Location','northwest','FontSize',9);
    xlim([0 x_max]); ylim([0 params.N+5]);
    grid on; box on;
    set(gca,'FontSize',10);
end
sgtitle('Vòng đời mạng: Số node chết theo vòng (▼=FND, ■=HND)', ...
    'FontSize', 13, 'FontWeight', 'bold');

%% ======================================================
%  HÌNH 2: FND và HND bar chart (Figs. 20-21 bài báo)
%% ======================================================
fig2 = figure('Name','FND và HND','Position',[50 520 900 420]);
set(fig2,'Color','white');

FND_data = [results_leach(1).FND, results_ducf(1).FND;
            results_leach(2).FND, results_ducf(2).FND;
            results_leach(3).FND, results_ducf(3).FND];

HND_data = [results_leach(1).HND, results_ducf(1).HND;
            results_leach(2).HND, results_ducf(2).HND;
            results_leach(3).HND, results_ducf(3).HND];

subplot(1,2,1);
b = bar(FND_data, 'grouped');
b(1).FaceColor = colors.leach;
b(2).FaceColor = colors.ducf;
set(gca,'XTickLabel',{'SC1','SC2','SC3'},'FontSize',11);
xlabel('Kịch bản','FontSize',11); ylabel('Số vòng','FontSize',11);
title('First Node Die (FND)','FontSize',12,'FontWeight','bold');
legend('LEACH','DUCF','Location','northeast','FontSize',10);
grid on; box on;
% Thêm giá trị trên thanh bar
for sc=1:3
    text(sc-0.15, FND_data(sc,1)+8, num2str(FND_data(sc,1)), ...
        'HorizontalAlignment','center','FontSize',9,'Color',colors.leach,'FontWeight','bold');
    text(sc+0.15, FND_data(sc,2)+8, num2str(FND_data(sc,2)), ...
        'HorizontalAlignment','center','FontSize',9,'Color',colors.ducf,'FontWeight','bold');
end

subplot(1,2,2);
b2 = bar(HND_data, 'grouped');
b2(1).FaceColor = colors.leach;
b2(2).FaceColor = colors.ducf;
set(gca,'XTickLabel',{'SC1','SC2','SC3'},'FontSize',11);
xlabel('Kịch bản','FontSize',11); ylabel('Số vòng','FontSize',11);
title('Half Node Die (HND)','FontSize',12,'FontWeight','bold');
legend('LEACH','DUCF','Location','northeast','FontSize',10);
grid on; box on;
for sc=1:3
    text(sc-0.15, HND_data(sc,1)+8, num2str(HND_data(sc,1)), ...
        'HorizontalAlignment','center','FontSize',9,'Color',colors.leach,'FontWeight','bold');
    text(sc+0.15, HND_data(sc,2)+8, num2str(HND_data(sc,2)), ...
        'HorizontalAlignment','center','FontSize',9,'Color',colors.ducf,'FontWeight','bold');
end
sgtitle('So sánh FND và HND: DUCF vs LEACH', 'FontSize',13,'FontWeight','bold');

%% ======================================================
%  HÌNH 3: Năng lượng tiêu thụ mỗi vòng
%  (Tương đương Fig. 15 bài báo)
%% ======================================================
fig3 = figure('Name','Energy per Round','Position',[980 520 900 420]);
set(fig3,'Color','white');

energy_leach_avg = zeros(1,3);
energy_ducf_avg  = zeros(1,3);
for sc = 1:3
    % Lấy trung bình 50 vòng đầu khi mạng còn đầy đủ
    n_sample = min(50, params.max_rounds);
    energy_leach_avg(sc) = mean(results_leach(sc).energy(1:n_sample));
    energy_ducf_avg(sc)  = mean(results_ducf(sc).energy(1:n_sample));
end

subplot(1,2,1);
b3 = bar([energy_leach_avg', energy_ducf_avg'], 'grouped');
b3(1).FaceColor = colors.leach;
b3(2).FaceColor = colors.ducf;
set(gca,'XTickLabel',{'SC1','SC2','SC3'},'FontSize',11);
xlabel('Kịch bản','FontSize',11);
ylabel('Năng lượng (J/vòng)','FontSize',11);
title('Năng lượng TB mỗi vòng (50 vòng đầu)','FontSize',11,'FontWeight','bold');
legend('LEACH','DUCF','Location','northeast','FontSize',10);
grid on; box on;

subplot(1,2,2);
% Energy theo thời gian - Scenario 1
e_l = results_leach(1).energy;
e_d = results_ducf(1).energy;
x_max2 = min(results_leach(1).HND + 50, params.max_rounds);
rounds2 = 1:x_max2;
% Làm mượt bằng moving average
window = 10;
e_l_smooth = movmean(e_l(1:x_max2), window);
e_d_smooth = movmean(e_d(1:x_max2), window);
plot(rounds2, e_l_smooth, '-', 'Color', colors.leach, 'LineWidth', lw); hold on;
plot(rounds2, e_d_smooth, '-', 'Color', colors.ducf,  'LineWidth', lw);
xlabel('Số vòng','FontSize',11); ylabel('Năng lượng (J)','FontSize',11);
title('Năng lượng/vòng theo thời gian (SC1, smoothed)','FontSize',10,'FontWeight','bold');
legend('LEACH','DUCF','Location','northeast','FontSize',10);
grid on; box on;
sgtitle('Năng lượng tiêu thụ: DUCF vs LEACH', 'FontSize',13,'FontWeight','bold');

%% ======================================================
%  HÌNH 4: Số tin nhắn gửi đến BS trước HND
%  (Tương đương Fig. 16 bài báo)
%% ======================================================
fig4 = figure('Name','Messages to BS','Position',[50 50 600 450]);
set(fig4,'Color','white');

msgs_data = [results_leach(1).msgs, results_ducf(1).msgs;
             results_leach(2).msgs, results_ducf(2).msgs;
             results_leach(3).msgs, results_ducf(3).msgs];

b4 = bar(msgs_data, 'grouped');
b4(1).FaceColor = colors.leach;
b4(2).FaceColor = colors.ducf;
set(gca,'XTickLabel',{'SC1','SC2','SC3'},'FontSize',12);
xlabel('Kịch bản', 'FontSize', 12);
ylabel('Số gói tin đến BS', 'FontSize', 12);
title('Tổng số tin nhắn đến BS trước HND', 'FontSize', 13, 'FontWeight','bold');
legend('LEACH','DUCF','Location','northwest','FontSize',11);
grid on; box on;
for sc=1:3
    text(sc-0.15, msgs_data(sc,1)*1.02, sprintf('%.0f',msgs_data(sc,1)), ...
        'HorizontalAlignment','center','FontSize',9,'Color',colors.leach,'FontWeight','bold');
    text(sc+0.15, msgs_data(sc,2)*1.02, sprintf('%.0f',msgs_data(sc,2)), ...
        'HorizontalAlignment','center','FontSize',9,'Color',colors.ducf,'FontWeight','bold');
end

%% ======================================================
%  HÌNH 5: Topology mạng với clustering (Scenario 1)
%  Minh hoạ sự khác biệt Equal vs Unequal clustering
%% ======================================================
fig5 = figure('Name','Network Topology - Cluster Visualization','Position',[700 50 1100 450]);
set(fig5,'Color','white');

% Tạo 1 snapshot topology để visualize
rng(42);
node_pos_demo = rand(params.N, 2) .* [params.width, params.height];

for sc_demo = 1:2  % Chỉ SC1 và SC2
    subplot(1,2,sc_demo);
    BS_demo = BS_positions(sc_demo,:);
    dist_BS_demo = sqrt((node_pos_demo(:,1)-BS_demo(1)).^2 + (node_pos_demo(:,2)-BS_demo(2)).^2);

    % Tính clustering DUCF đơn giản cho visualization
    energy_demo = ones(params.N,1);
    dist_mat_demo = zeros(params.N,params.N);
    for i=1:params.N
        for j=i+1:params.N
            d=sqrt((node_pos_demo(i,1)-node_pos_demo(j,1))^2+(node_pos_demo(i,2)-node_pos_demo(j,2))^2);
            dist_mat_demo(i,j)=d; dist_mat_demo(j,i)=d;
        end
    end
    nd_demo=zeros(params.N,1);
    for i=1:params.N
        nd_demo(i)=sum(dist_mat_demo(i,:)<=params.R_comm)-1;
    end
    chance_demo=zeros(params.N,1); size_demo=zeros(params.N,1);
    for i=1:params.N
        [chance_demo(i),size_demo(i)]=ducf_fuzzy(1,nd_demo(i),dist_BS_demo(i),params);
    end
    is_CH_demo=false(params.N,1);
    for i=1:params.N
        nb=find(dist_mat_demo(i,:)<=params.R_comm);
        if chance_demo(i)>=max(chance_demo(nb)), is_CH_demo(i)=true; end
    end
    CH_demo=find(is_CH_demo);

    % Vẽ links member → CH
    cluster_demo=zeros(params.N,1);
    for i=1:params.N
        if is_CH_demo(i), cluster_demo(i)=i; continue; end
        if isempty(CH_demo), continue; end
        [~,bi]=min(dist_mat_demo(i,CH_demo));
        cluster_demo(i)=CH_demo(bi);
    end

    % Màu sắc cho từng cluster
    cmap = lines(length(CH_demo));
    hold on;
    for ki=1:length(CH_demo)
        ch=CH_demo(ki);
        mems=find(cluster_demo==ch & ~is_CH_demo);
        clr=cmap(ki,:);
        % Vẽ đường nối member-CH
        for m=mems'
            plot([node_pos_demo(m,1),node_pos_demo(ch,1)], ...
                 [node_pos_demo(m,2),node_pos_demo(ch,2)], ...
                 '-','Color',[clr 0.3],'LineWidth',0.8);
        end
        % Vẽ members
        scatter(node_pos_demo(mems,1),node_pos_demo(mems,2),40,...
            clr,'o','filled','MarkerFaceAlpha',0.7);
        % Vẽ CH (hình vuông lớn hơn)
        scatter(node_pos_demo(ch,1),node_pos_demo(ch,2),150,...
            clr,'s','filled','MarkerEdgeColor','k','LineWidth',1.5);
        % Vẽ đường CH → BS (multi-hop)
        plot([node_pos_demo(ch,1),BS_demo(1)],[node_pos_demo(ch,2),BS_demo(2)],...
            '--','Color',[0 0 0 0.2],'LineWidth',0.8);
    end
    % BS
    scatter(BS_demo(1),BS_demo(2),300,'r','p','filled','MarkerEdgeColor','k','LineWidth',2);
    text(BS_demo(1)+3,BS_demo(2)+5,'BS','FontSize',11,'FontWeight','bold','Color','r');

    xlabel('x (m)','FontSize',11); ylabel('y (m)','FontSize',11);
    title(sprintf('DUCF Clustering - %s\n(■=CH, ●=Member, ★=BS, %d CHs)', ...
        scenario_names{sc_demo}, length(CH_demo)),'FontSize',10,'FontWeight','bold');
    xlim([-10 params.width+10]); ylim([-10 max(params.height,BS_positions(sc_demo,2))+15]);
    grid on; box on; axis equal;
end
sgtitle('Minh hoạ Unequal Clustering của DUCF', 'FontSize',13,'FontWeight','bold');

%% ======================================================
%  HÌNH 6: % Cải thiện DUCF so với LEACH
%% ======================================================
fig6 = figure('Name','Improvement %','Position',[700 520 600 420]);
set(fig6,'Color','white');

improve_FND = zeros(1,3); improve_HND = zeros(1,3); improve_msgs = zeros(1,3);
for sc=1:3
    improve_FND(sc)  = (results_ducf(sc).FND  - results_leach(sc).FND)  / results_leach(sc).FND  * 100;
    improve_HND(sc)  = (results_ducf(sc).HND  - results_leach(sc).HND)  / results_leach(sc).HND  * 100;
    improve_msgs(sc) = (results_ducf(sc).msgs  - results_leach(sc).msgs) / results_leach(sc).msgs * 100;
end

b6 = bar([improve_FND; improve_HND; improve_msgs]', 'grouped');
b6(1).FaceColor = [0.2 0.7 0.3];
b6(2).FaceColor = [0.2 0.5 0.9];
b6(3).FaceColor = [0.9 0.7 0.1];
set(gca,'XTickLabel',{'SC1','SC2','SC3'},'FontSize',12);
xlabel('Kịch bản','FontSize',12);
ylabel('Cải thiện so với LEACH (%)','FontSize',12);
title('DUCF cải thiện bao nhiêu % so với LEACH?','FontSize',12,'FontWeight','bold');
legend('FND','HND','Msgs to BS','Location','northwest','FontSize',11);
yline(0,'k--','LineWidth',1);
grid on; box on;

%% ---- Lưu tất cả figures ----
output_dir = '/mnt/user-data/outputs/';
saveas(fig1, [output_dir 'Fig1_NetworkLifetime.png']);
saveas(fig2, [output_dir 'Fig2_FND_HND_bar.png']);
saveas(fig3, [output_dir 'Fig3_EnergyPerRound.png']);
saveas(fig4, [output_dir 'Fig4_MessagesToBS.png']);
saveas(fig5, [output_dir 'Fig5_ClusterVisualization.png']);
saveas(fig6, [output_dir 'Fig6_ImprovementPercent.png']);
fprintf('\n✓ Đã lưu 6 đồ thị vào /mnt/user-data/outputs/\n');

end