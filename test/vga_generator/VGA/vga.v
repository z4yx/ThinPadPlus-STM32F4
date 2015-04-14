module vga(
input wire pixel_clk,
input wire rst,
output wire hsync,
output wire vsync,
output reg[10:0] hcnt,
output reg[10:0] vcnt
);
parameter h_visible = 640;
parameter h_fporch_end = h_visible+16;
parameter h_sync_end = h_fporch_end+96;
parameter h_bporch_end = h_sync_end+48;

parameter v_visible = 480;
parameter v_fporch_end = v_visible+10;
parameter v_sync_end = v_fporch_end+2;
parameter v_bporch_end = v_sync_end+33;

assign hsync = (hcnt>=h_fporch_end&&hcnt<h_sync_end) ? 0 : 1;
assign vsync = (vcnt>=v_fporch_end&&vcnt<v_sync_end) ? 0 : 1;

always @(posedge pixel_clk or negedge rst) begin
    if (!rst) begin
        hcnt<=0;
        vcnt<=0;
    end
    else begin
        if (hcnt == h_bporch_end-1) begin
            hcnt <= 0;
            vcnt <= (vcnt == v_bporch_end-1) ? 0 : vcnt+1;
        end
        else begin
            hcnt<=hcnt+1;
        end
    end
end

endmodule