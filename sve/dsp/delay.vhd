library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.numeric_std.all;

entity delay is
    generic (
        DELAY_CYCLES : integer := 3;
        SIGNAL_WIDTH : integer := 48
    );
    port (
        clk   : in  std_logic;
        rst   : in  std_logic;
        din   : in  std_logic_vector(SIGNAL_WIDTH - 1 downto 0);
        dout  : out std_logic_vector(SIGNAL_WIDTH - 1 downto 0)
    );
end entity delay;

architecture Behavioral of delay is
    type pipeline_type is array (0 to DELAY_CYCLES-1) of std_logic_vector(SIGNAL_WIDTH - 1 downto 0);
    signal pipeline : pipeline_type := (others => (others => '0'));
begin
    process (clk)
    begin
        if rising_edge(clk) then
            if rst = '1' then
                pipeline <= (others => (others => '0'));
            else
                pipeline(0) <= din;
                for i in 1 to DELAY_CYCLES-1 loop
                    pipeline(i) <= pipeline(i-1);
                end loop;
            end if;
        end if;
    end process;

    dout <= pipeline(DELAY_CYCLES-1);
end architecture Behavioral;