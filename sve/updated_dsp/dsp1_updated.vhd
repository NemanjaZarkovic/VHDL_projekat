
-- This code implements a parameterizable subtractor followed by multiplier which will be packed into DSP Block. Widths must be less than or equal to what is supported by the DSP block else exta logic will be inferred
-- Operation : i_cose * (i - iradius) / i_sine * (j - iradius)
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all; 

entity dsp1 is
  generic (
           WIDTH : integer := 11; 
           FIXED_SIZE : integer := 48
          -- ADD_SUB : string:= "add" 
          );
  port (
        clk : in  std_logic;     -- Clock
        rst : in std_logic;
        u1_i : in std_logic_vector(WIDTH - 1 downto 0); -- i/j
        u2_i : in std_logic_vector(WIDTH - 1 downto 0); --iradius
        u3_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); -- i_cose/i_sine
        res_o : out std_logic_vector(FIXED_SIZE -1 downto 0) -- Output
       );
end dsp1;

architecture Behavioral of dsp1 is
    attribute use_dsp : string;
    attribute use_dsp of Behavioral : architecture is "yes";
    
    constant zero_vector : unsigned(18-1 downto 0) := (others => '0');
    signal u1_reg : unsigned(WIDTH - 1 downto 0);
    signal u2_reg : unsigned(WIDTH - 1 downto 0);
    signal u3_reg : signed(FIXED_SIZE - 1 downto 0);
    signal sub : signed(WIDTH - 1  downto 0);
    signal mult : signed(2 * FIXED_SIZE - 1 downto 0);
    signal res_reg : signed(2*FIXED_SIZE - 30 - 1 downto 18);
    begin
    
    process(clk)
     begin
      if rising_edge(clk) then
         if (rst = '1') then
             u1_reg <= (others => '0');
             u2_reg <= (others => '0');
             u3_reg <= (others => '0');
             sub <= (others => '0');
             mult <= (others => '0');
             res_reg <= (others => '0');
         else
             --if(ADD_SUB = "add")then
                 u1_reg <= unsigned(u1_i);
                 u2_reg <= unsigned(u2_i);
                 u3_reg <= signed(u3_i);
                 sub <= signed(u1_reg - u2_reg);
                 mult <= resize(sub & signed(zero_vector), FIXED_SIZE) * u3_reg;
                 res_reg <= mult(2*FIXED_SIZE - 30 - 1 downto 18);
         end if;
      end if;
    end process;
    -- Type conversion for output
    res_o <= std_logic_vector(res_reg);

    
    end Behavioral;
