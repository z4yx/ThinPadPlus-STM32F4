module image(
input wire pixel_clk,
input wire rst,
input wire[10:0] hcnt,
input wire[10:0] vcnt,
output reg[2:0] pixel_r_out,
output reg[2:0] pixel_g_out,
output reg[2:0] pixel_b_out
);

parameter WIDTH=800;
parameter HEIGHT=600;


reg clk_move;
reg[18:0] cnt_clk;

always @(posedge pixel_clk or negedge rst) begin
    if (!rst) begin
        clk_move<=1;
        cnt_clk<=0;
    end
    else if (cnt_clk == 100000-1) begin
        cnt_clk<=0;
        clk_move<=~clk_move;
    end
    else begin
        cnt_clk<=cnt_clk+1;
    end
end

reg [10:0] x,y;

reg [2:0] color;
wire [2:0] r_val, g_val, b_val;
assign r_val = color[2]?3'b111:0;
assign g_val = color[1]?3'b111:0;
assign b_val = color[0]?3'b111:0;

assign in_area = (x>=vcnt?x-vcnt<=2:vcnt-x<=2)&&(y>=hcnt?y-hcnt<=2:hcnt-y<=2);

always @(posedge pixel_clk or negedge rst) begin
    if (!rst) begin
        pixel_r_out<=0;
        pixel_g_out<=0;
        pixel_b_out<=0;
    end
    else begin
        {pixel_r_out,pixel_g_out,pixel_b_out} <= 
            in_area/*(vcnt==x&&hcnt==y)*/ ? {r_val,g_val,b_val} : 0;
    end
end

reg dir_x, dir_y;
wire rev_dir_x, rev_dir_y;

assign rev_dir_x = ~dir_x;
assign rev_dir_y = ~dir_y;

always @(posedge clk_move or negedge rst) begin
    if (!rst) begin
        x<=0;
        y<=0;
        dir_x<=0;
        dir_y<=0;
        color<=0;
    end
    else begin
        if (x==HEIGHT-1 || x==0 || y==WIDTH-1 || y==0) begin
            color<=(color==3'b111) ? 1 : color+1;
        end
        if (x==HEIGHT-1 || x==0) begin
            dir_x<=rev_dir_x;
            x<=rev_dir_x?x+1:x-1;
        end
        else begin
            x<=dir_x?x+1:x-1;
        end
        if (y==WIDTH-1 || y==0) begin
            dir_y<=rev_dir_y;
            y<=rev_dir_y?y+1:y-1;
        end
        else begin
            y<=dir_y?y+1:y-1;
        end
    end
end

endmodule