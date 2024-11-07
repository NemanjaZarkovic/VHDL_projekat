library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity dsp8_tb is
end dsp8_tb;

architecture Behavioral of dsp8_tb is
    constant FIXED_SIZE : integer := 48;
    constant clk_period : time := 10 ns;

    -- Signals for the DUT (Device Under Test)
    signal clk : std_logic := '0';
    signal rst : std_logic := '1';
    signal u1_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal u2_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal u3_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal res_o : std_logic_vector(FIXED_SIZE - 1 downto 0);

begin
    -- Instantiate the Unit Under Test (UUT)
    uut: entity work.dsp8
        generic map (
            FIXED_SIZE => FIXED_SIZE
        )
        port map (
            clk => clk,
            rst => rst,
            u1_i => u1_i,
            u2_i => u2_i,
            u3_i => u3_i,
            res_o => res_o
        );

    -- Clock process definition
    clk_process : process
    begin
        while true loop
            clk <= '0';
            wait for clk_period / 2;
            clk <= '1';
            wait for clk_period / 2;
        end loop;
    end process;

    -- Stimulus process
    stim_proc: process
    begin
        -- Hold reset state for 20 ns
        wait for 20 ns;
        rst <= '0';

        -- Test case 1: Set specific values for u1_i, u2_i, and u3_i
        u1_i <= "000000000000000000000000001001101011111000001110";  -- dxx1
        u2_i <= "000000000000000000000000000000011100110110110110";  -- dxx2
        u3_i <= "000000000000000000000000000000001010000011000010";  -- weight
        wait for clk_period * 3;  -- Wait for 3 clock cycles to allow computation

        -- Report the output value
       -- report "Value of res_o after computation: " & integer'image(to_integer(signed(res_o)));

        -- End simulation
        wait;
    end process;

end Behavioral;
