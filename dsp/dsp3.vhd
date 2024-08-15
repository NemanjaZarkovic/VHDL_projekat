-- Operation : step * temp_3 - frac
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all; 

entity dsp3 is
  generic (
           WIDTH : integer := 11;  
           FIXED_SIZE : integer := 48  
          );
  port (
        clk : in  std_logic;    
        rst : in std_logic;
        u1_i : in std_logic_vector(WIDTH - 1 downto 0); -- step
        u2_i : in signed (FIXED_SIZE -1 downto 0); -- temp_3
        u3_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); -- frac
        res_o : out signed(FIXED_SIZE -1 downto 0) -- temp_4
       );
end dsp3;

architecture Behavioral of dsp3 is
    attribute use_dsp : string;
    attribute use_dsp of Behavioral : architecture is "yes";

    signal u1_reg : signed(WIDTH - 1 downto 0);
    signal u2_reg : signed(FIXED_SIZE - 1 downto 0);
    signal u3_reg : signed(FIXED_SIZE - 1 downto 0);
    signal mult, res_reg : signed(FIXED_SIZE + WIDTH - 1 downto 0);
    signal sub : signed(FIXED_SIZE + WIDTH - 1  downto 0);
    --signal reg_res : signed(FIXED_SIZE - 1 downto 0);


    begin

    process(clk)
     begin
      if rising_edge(clk) then
         if (rst = '1') then
             u1_reg <= (others => '0');
             u2_reg <= (others => '0');
             u3_reg <= (others => '0');
             mult <= (others => '0');
             res_reg <= (others => '0');
             sub <= (others => '0');
             --reg_res <=(others => '0');
         else
             u1_reg <= signed(u1_i);
             u2_reg <= u2_i;
             u3_reg <= signed(u3_i);
             mult <= u1_reg * u2_reg;
             res_reg <= mult;
              sub <= res_reg - u3_reg;
             --sub <= p - resize(frac,FIXED_SIZE + WIDTH);
             --reg_res <= resize(sub, FIXED_SIZE);
         end if;
      end if;
    end process;

    -- Type conversion for output
    res_o <= resize(sub, FIXED_SIZE);

    end Behavioral;