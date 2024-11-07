library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

-- Operation : rfrac=rx-ri 

entity dsp6 is
    generic (
          FIXED_SIZE : integer:= 48;
          WIDTH : integer:= 11
          );
    port (clk: in std_logic;
          rst: in std_logic;
          u1_i: in std_logic_vector(FIXED_SIZE - 1 downto 0); --rx
          u2_i: in std_logic_vector(WIDTH - 1 downto 0); -- ri, iy, ix
          ADD_SUB : in std_logic; -- '0' for add, '1' for subtract
          res_o: out std_logic_vector(FIXED_SIZE - 1 downto 0));-- rfrac
end dsp6;

architecture Behavioral of dsp6 is
    attribute use_dsp : string;
    attribute use_dsp of Behavioral : architecture is "yes";
    constant zero_vector1 : unsigned(19-1 downto 0) := (others => '0');
    constant zero_vector2 : unsigned(18-1 downto 0) := (others => '0');
    signal u1_reg: signed(FIXED_SIZE - 1 downto 0);  
    signal u2_reg: unsigned(FIXED_SIZE - 1 downto 0);
    signal res_reg: signed(FIXED_SIZE - 1 downto 0);
begin
    process(clk) is
    begin
        if (rising_edge(clk)) then
            if (rst = '1') then
                u1_reg <= (others => '0');
                u2_reg <= (others => '0');
                res_reg <= (others => '0');
            else
                u1_reg <= signed(u1_i);
                u2_reg <= zero_vector1 & unsigned(u2_i) & zero_vector2;
                if (ADD_SUB = '0') then
                    res_reg <= u1_reg + signed(u2_reg);
                else
                    res_reg <= u1_reg - signed(u2_reg);
                end if;                
            end if;
        end if;
    end process;
    res_o <= std_logic_vector(res_reg);
end Behavioral;