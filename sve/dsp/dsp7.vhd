-- Operation : iy + (i - iradius) *step
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all; 

entity dsp7 is
  generic (
           WIDTH : integer := 11
          );
  port (
        clk : in  std_logic;     -- Clock
        rst : in std_logic;
        u1_i : in std_logic_vector(WIDTH - 1 downto 0); -- i/j
        u2_i : in std_logic_vector(WIDTH - 1 downto 0); --iradius
        u3_i : in std_logic_vector(WIDTH - 1 downto 0); -- step
        u4_i : in std_logic_vector(WIDTH - 1 downto 0);
        res_o : out std_logic_vector(WIDTH -1 downto 0) -- Output
       );
end dsp7;

architecture Behavioral of dsp7 is
    attribute use_dsp : string;
    attribute use_dsp of Behavioral : architecture is "yes";

    signal u1_reg : unsigned(WIDTH - 1 downto 0);
    signal u2_reg : unsigned(WIDTH - 1 downto 0);
    signal u3_1_reg, u3_2_reg : unsigned(WIDTH - 1 downto 0);
    signal u4_1_reg, u4_2_reg, u4_3_reg : unsigned(WIDTH - 1 downto 0);
    signal sub : signed(WIDTH - 1  downto 0); -- oduzimanje je signed zato sto moze da bude negativno, i to posle povlaci da mnozenje i sabiranje mogu biti negativni
    signal mult, res_reg : signed(2 * WIDTH - 1 downto 0);

    begin

    process(clk)
     begin
      if rising_edge(clk) then
         if (rst = '1') then
             u1_reg <= (others => '0');
             u2_reg <= (others => '0');
             u3_1_reg <= (others => '0');
             u3_2_reg <= (others => '0');
             u4_1_reg <= (others => '0');
             u4_2_reg <= (others => '0');
             u4_3_reg <= (others => '0');
             sub <= (others => '0');
             mult <= (others => '0');
             res_reg <= (others => '0');
         else
             u1_reg <= unsigned(u1_i);
             u2_reg <= unsigned(u2_i);
             u3_1_reg <= unsigned(u3_i);
             u3_2_reg <= u3_1_reg;
             u4_1_reg <= unsigned(u4_i);
             u4_2_reg <= u4_1_reg;
             u4_3_reg <= u4_2_reg;
             sub <= signed(u1_reg) - signed(u2_reg);
             mult <= sub * signed(u3_2_reg);
             res_reg <= mult + signed(u4_3_reg);
         end if;
      end if;
    end process;
    res_o <= std_logic_vector(res_reg(WIDTH- 1 downto 0));

    end Behavioral;
