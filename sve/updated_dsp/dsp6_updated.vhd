library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

-- Operation : temp_1 + temp_2

entity dsp6 is
    generic (
          FIXED_SIZE : integer:= 48;
          WIDTH : integer:= 11
          );
    port (clk: in std_logic;
          rst: in std_logic;
          u1_i: in std_logic_vector(FIXED_SIZE - 1 downto 0); --rx
          u2_i: in std_logic_vector(WIDTH - 1 downto 0); -- i_sine * (j - iradius), temp_2
          res_o: out std_logic_vector(FIXED_SIZE - 1 downto 0));-- temp_3
end dsp6;

architecture Behavioral of dsp6 is
    attribute use_dsp : string;
    attribute use_dsp of Behavioral : architecture is "yes";
    -- ako treba dodaj 2 registra za ulaze
    constant zero_vector : unsigned(18-1 downto 0) := (others => '0');
    signal u1_reg: signed(FIXED_SIZE - 1 downto 0);  
    signal u2_reg: unsigned(WIDTH - 1 downto 0);
    signal u3_reg: unsigned(FIXED_SIZE - 1 downto 0);
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
                u2_reg <= unsigned(u2_i);
                u3_reg <= resize(u2_reg & zero_vector, FIXED_SIZE);
                res_reg <= u1_reg - signed(u3_reg);
            end if;
        end if;
    end process;
    res_o <= std_logic_vector(res_reg);
end Behavioral;