/******************************************************************************
*                                                                             *
* Copyright 2016 myStorm Copyright and related                                *
* rights are licensed under the Solderpad Hardware License, Version 0.51      *
* (the “License”); you may not use this file except in compliance with        *
* the License. You may obtain a copy of the License at                        *
* http://solderpad.org/licenses/SHL-0.51. Unless required by applicable       *
* law or agreed to in writing, software, hardware and materials               *
* distributed under this License is distributed on an “AS IS” BASIS,          *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or             *
* implied. See the License for the specific language governing                *
* permissions and limitations under the License.                              *
*                                                                             *
******************************************************************************/

/* 7 Segments - Excludes DP **************************************************
*
*  a  _
*  f | | b
*  g  -
*  e |_| c
*  d  
*    
* a:s7[0], g = s7[6]
 *****************************************************************************/

module h27seg (
    input wire [3:0] hex,
    output reg [6:0] s7
    );

    always @(*)
        case (hex)
            // Segments - gfedcba
            4'h0: s7 = ~7'b0111111;
            4'h1: s7 = ~7'b0000110;
            4'h2: s7 = ~7'b1011011;
            4'h3: s7 = ~7'b1001111;
            4'h4: s7 = ~7'b1100110;
            4'h5: s7 = ~7'b1101101;
            4'h6: s7 = ~7'b1111101;
            4'h7: s7 = ~7'b0000111;
            4'h8: s7 = ~7'b1111111;
            4'h9: s7 = ~7'b1101111;
            4'hA: s7 = ~7'b1110111;
            4'hB: s7 = ~7'b1111100;
            4'hC: s7 = ~7'b0111001;
            4'hD: s7 = ~7'b1011110;
            4'hE: s7 = ~7'b1111001;
            4'hF: s7 = ~7'b1110001;
        endcase 

endmodule

