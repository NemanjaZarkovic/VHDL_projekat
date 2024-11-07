library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;
-- Operation : temp_3*spacing
entity dsp4 is
    generic ( 
          FIXED_SIZE : integer := 48);          
    port (clk: in std_logic;
          rst: in std_logic;
          u1_i: in signed(FIXED_SIZE - 1 downto 0);
          spacing : in std_logic_vector(FIXED_SIZE - 1 downto 0);
          res_o: out std_logic_vector(FIXED_SIZE - 1 downto 0));
end dsp4;

architecture Behavioral of dsp4 is
    attribute use_dsp : string;
    attribute use_dsp of Behavioral : architecture is "yes";
    
    signal u1_reg: signed(FIXED_SIZE - 1 downto 0);
    signal u2_reg: signed(FIXED_SIZE - 1 downto 0);
    signal res_reg: signed(2*FIXED_SIZE - 1 downto 0);
begin
    process(clk) is
    begin
        if (rising_edge(clk)) then
            if (rst = '1') then
                u1_reg <= (others => '0');
                u2_reg <= (others => '0');
                res_reg <= (others => '0');
            else
                u1_reg <= u1_i;
                u2_reg <= signed(spacing);
                res_reg <= u1_reg * u2_reg;
            end if;
        end if;
    end process;
    --res_o <= std_logic_vector(res_reg);
    res_o <= std_logic_vector(res_reg(2*FIXED_SIZE - 30 - 1 downto 18));
end Behavioral;