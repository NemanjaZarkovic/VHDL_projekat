library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all; 
-- Operation : dxx = weight * (dxx1 - dxx2);
entity dsp8 is
  generic (
           FIXED_SIZE : integer := 48
          );
  port (
        clk : in  std_logic;     -- Clock
        rst : in std_logic;
        u1_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); -- dxx1
        u2_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); -- dxx2
        u3_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); -- weight
        res_o : out std_logic_vector(FIXED_SIZE -1 downto 0) -- dxx
       );
end dsp8;

architecture Behavioral of dsp8 is
    attribute use_dsp : string;
    attribute use_dsp of Behavioral : architecture is "yes";

    signal u1_reg : signed(FIXED_SIZE - 1 downto 0);
    signal u2_reg : signed(FIXED_SIZE - 1 downto 0);
    signal u3_1_reg, u3_2_reg  : signed(FIXED_SIZE - 1 downto 0);
    signal sub : signed(FIXED_SIZE - 1 downto 0);
    signal res_reg : signed(2* FIXED_SIZE - 1 downto 0);


begin

    process(clk)
    begin
        if rising_edge(clk) then
            if (rst = '1') then
                u1_reg <= (others => '0');
                u2_reg <= (others => '0');
                u3_1_reg <= (others => '0');
                u3_2_reg <= (others => '0');
                sub <= (others => '0');
                res_reg <= (others => '0');
            else
                u1_reg <= signed(u1_i);
                u2_reg <= signed(u2_i);
                u3_1_reg <= signed(u3_i);
                u3_2_reg <= u3_1_reg; 
                sub <= u1_reg - u2_reg;
                res_reg <= sub * u3_2_reg;
            end if;
        end if;
    end process;

    res_o <= std_logic_vector(res_reg(2*FIXED_SIZE - 30 - 1 downto 18));
    

end Behavioral;
