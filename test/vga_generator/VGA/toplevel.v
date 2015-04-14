module toplevel(
input wire gclk,
input wire rst,
output wire hsync,
output wire vsync,
output wire[2:0] pixel_r_out,
output wire[2:0] pixel_g_out,
output wire[2:0] pixel_b_out
);


parameter h_visible = 800;
parameter h_fporch_end = h_visible+56;
parameter h_sync_end = h_fporch_end+120;
parameter h_bporch_end = h_sync_end+64;
parameter v_visible = 600;
parameter v_fporch_end = v_visible+37;
parameter v_sync_end = v_fporch_end+6;
parameter v_bporch_end = v_sync_end+23;

/*
parameter h_visible = 1280;
parameter h_fporch_end = h_visible+72;
parameter h_sync_end = h_fporch_end+80;
parameter h_bporch_end = h_sync_end+216;
parameter v_visible = 720;
parameter v_fporch_end = v_visible+3;
parameter v_sync_end = v_fporch_end+5;
parameter v_bporch_end = v_sync_end+22;
*/

// wire clk75M;

// dcm clk_dcm (
//     .CLKIN_IN(gclk), 
//     .RST_IN(!rst), 
//     .CLKFX_OUT(clk75M), 
//     .CLKIN_IBUFG_OUT(), 
//     .CLK0_OUT(), 
//     .LOCKED_OUT()
//     );

wire clk_50M;
assign clk_50M = gclk;

reg clk_25M;
always @(posedge clk_50M or negedge rst) begin
    if (!rst) begin
        clk_25M <= 0;
    end
    else begin
        clk_25M <= ~clk_25M;
    end
end

wire [10:0] h,v;

vga disp1(clk_50M, rst, hsync, vsync, h, v);
defparam disp1.h_visible = h_visible;
defparam disp1.h_fporch_end = h_fporch_end;
defparam disp1.h_sync_end = h_sync_end;
defparam disp1.h_bporch_end = h_bporch_end;
defparam disp1.v_visible = v_visible;
defparam disp1.v_fporch_end = v_fporch_end;
defparam disp1.v_sync_end = v_sync_end;
defparam disp1.v_bporch_end = v_bporch_end;

image img1(clk_50M, rst, h, v, pixel_r_out, pixel_g_out, pixel_b_out);
defparam img1.WIDTH = h_visible;
defparam img1.HEIGHT = v_visible;


endmodule
