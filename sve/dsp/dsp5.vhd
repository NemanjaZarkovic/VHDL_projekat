-- Operation : rx = rpos + _IndexSize / 2.0 - 0.5;
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all; 

entity dsp5 is
  generic ( 
           FIXED_SIZE : integer := 48
          );
  port (
        clk : in  std_logic; 
        rst : in std_logic;
        u1_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); -- rpos
        u2_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); --_IndexSize / 2.0
        u3_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); -- 0.5
        res_o : out std_logic_vector(FIXED_SIZE -1 downto 0) -- rx
       );
end dsp5;

architecture Behavioral of dsp5 is
    attribute use_dsp : string;
    attribute use_dsp of Behavioral : architecture is "yes";

    signal u1_reg : signed(FIXED_SIZE - 1 downto 0);
    signal u2_reg : signed(FIXED_SIZE - 1 downto 0);
    signal u3_reg : signed(FIXED_SIZE - 1 downto 0);
    signal u4_reg : signed(FIXED_SIZE - 1 downto 0);
    signal add: signed(FIXED_SIZE - 1  downto 0);
    signal sub : signed(FIXED_SIZE - 1 downto 0);

    begin
    
    process(clk)
     begin
      if rising_edge(clk) then
         if (rst = '1') then
             u1_reg <= (others => '0');
             u2_reg <= (others => '0');
             u3_reg <= (others => '0');
             u4_reg <= (others => '0');
             add <= (others => '0');
             sub <= (others => '0');
         else
             u1_reg <= signed(u1_i);
             u2_reg <= signed(u2_i);
             u3_reg <= signed(u3_i);
             u4_reg <= u3_reg;
             add <= u1_reg + u2_reg;
             sub <= add - u4_reg;
         end if;
      end if;
    end process;
    
    -- Type conversion for output

    res_o <= std_logic_vector(sub);

    
    end Behavioral;