library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all; 
--Operation step * (i_cose * (i - iradius) + i_sine * (j - iradius)) - fracr
--step * temp_3 -frac         
entity dsp3 is
  generic (
           WIDTH : integer := 11;  
           FIXED_SIZE : integer := 48  
          );
  port (
        clk : in  std_logic;    
        rst : in std_logic;
        u1_i : in std_logic_vector(WIDTH - 1 downto 0); -- step
        u2_i : in std_logic_vector(FIXED_SIZE -1 downto 0); -- temp_3
        u3_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); -- frac
        res_o : out std_logic_vector(FIXED_SIZE -1 downto 0) -- temp_4
       );
end dsp3;

architecture Behavioral of dsp3 is
    attribute use_dsp : string;
    attribute use_dsp of Behavioral : architecture is "yes";

    signal u1_reg : signed(WIDTH - 1 downto 0);
    signal u2_reg : signed(FIXED_SIZE - 1 downto 0);
    signal u3_reg : signed(FIXED_SIZE - 1 downto 0);
    signal u4_reg : signed(FIXED_SIZE + WIDTH - 1 downto 0); 
    signal res_reg : signed(FIXED_SIZE + WIDTH - 1 downto 0);
    signal sub : signed(FIXED_SIZE + WIDTH - 1  downto 0);

begin
    
    process(clk)
    begin
        if rising_edge(clk) then
            if (rst = '1') then
                -- Reset all registers to zero with correct width
                u1_reg <= (others => '0');
                u2_reg <= (others => '0');
                u3_reg <= (others => '0');
                u4_reg <= (others => '0');
                res_reg <= (others => '0');
                sub <= (others => '0');
            else
                u1_reg <= signed(u1_i);
                u2_reg <= signed(u2_i);
                u3_reg <= signed(u3_i);
                u4_reg <= resize(u3_reg, FIXED_SIZE + WIDTH); 
                res_reg <= u1_reg * u2_reg;
                sub <= res_reg - u4_reg;
            end if;
        end if;
    end process;
    
    res_o <= std_logic_vector(sub(FIXED_SIZE- 1 downto 0));
    
end Behavioral;
