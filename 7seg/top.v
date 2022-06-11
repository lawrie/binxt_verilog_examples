module top(
  input clk_25mhz,
  output [2:0] ca,
  output a,
  output b,
  output c,
  output d,
  output e,
  output f,
  output g
);

 reg [36:0] timer;

 wire [11:0] val = timer[36:25];

 always @(posedge clk_25mhz) timer <= timer + 1;
 
 wire [3:0] dig = (ca == 'b011 ? val[11:8] : ca == 'b101 ? val[7:4] : val[3:0]);

 h27seg hex (
   .hex(dig),
   .s7({g, f, e, d, c, b, a})
 );

 seven_seg_display seg7 (
   .clk(clk_25mhz),
   .ca(ca)
 );

endmodule

