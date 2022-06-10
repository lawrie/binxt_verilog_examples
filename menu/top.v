`default_nettype none
module top
(
	input        clk_25mhz,
	output [5:0] led,
	output       sclk,
	output       cs,
	output       mosi,
      	input        miso,
	output       hsync,
	output       vsync,
	output [2:0] red,
        output [3:0] green,
	output [2:0] blue,
	output       tx,
	input        rx,
	output       gnd
);

	assign gnd = 0;

	// Power-on reset
	reg [5:0] reset_cnt = 0;
        wire resetn = &reset_cnt;

        always @(posedge clk_25mhz) begin
                reset_cnt <= reset_cnt + !resetn;
        end

	wire [23:0] color;
	wire [9:0] x;
	wire [9:0] y;
	wire [6:0] xc = x[9:3];
	wire [5:0] yc = y[9:4];

	wire [7:0] data_out;

	wire [71:0] title = "Game Menu";

	reg [127:0] game[8];

	reg [11:0] addr;

	localparam TITLE_ROW = 5;
	localparam TITLE_START_COL = 34;
	localparam TITLE_END_COL = 43;
	localparam MENU_START_ROW = 8;
	localparam MENU_END_ROW = 16;
	localparam MENU_START_COL = 32;
	localparam MENU_END_COL = 48;

	reg [2:0] sel = 0;
	reg [3:0] index = 0;

	// Set the address 
	always @* begin
		if (yc == TITLE_ROW && (xc >= TITLE_START_COL && xc < TITLE_END_COL)) begin
			addr = {title[((TITLE_END_COL - xc) << 3)-1 -: 8], y[3:0]};
		end else if ((yc >= MENU_START_ROW && yc < MENU_END_ROW) && (xc >= MENU_START_COL && xc < MENU_END_COL))  begin
			addr = {game[yc - MENU_START_ROW][((MENU_END_COL - xc) << 3)-1 -: 8], y[3:0]};
		end else begin
		       addr = 0;
		end	       
	end

	font_rom vga_font(
		.clk(clk_25mhz),
		.addr(addr),
		.data_out(data_out)
	);

	assign color = data_out[7-x[2:0]+1] ? (yc == (MENU_START_ROW + sel) ? 24'hffff00 : 24'hffffff) : 24'h000000; // +1 for sync

	wire vga_blank;

	assign red = vga_blank ? 3'b0 : color[23:21];
	assign green = vga_blank ? 4'b0 : color[15:12];
	assign blue = vga_blank ? 3'b0 : color[7:5];

	vga_video vga(
		.clk(clk_25mhz),
		.resetn(resetn),
		.vga_vsync(vsync),
		.vga_hsync(hsync),
		.vga_blank(vga_blank),
		.h_pos(x),
		.v_pos(y)
	);

	wire [3:0] gx,gy;
	wire [7:0] ch;
	wire set_ch;
	wire [5:0] led_out;

	attosoc soc(
		.clk(clk_25mhz),
		.resetn(resetn),
		.index(index),
		.led(led_out),
		.uart_tx(tx),
		.uart_rx(rx),
		.SD_SCK(sclk),
		.SD_SS(cs),
		.SD_MOSI(mosi),
		.SD_MISO(miso),
		.x(gx),
		.y(gy),
		.ch(ch),
		.set_ch(set_ch)
	);

	// Set game menu character
	always @(posedge clk_25mhz) begin
		if (set_ch) begin
			game[gy][((16 - gx) << 3) - 1 -: 8] <= ch;
		end
	end

	assign led = ~led_out;

endmodule

