library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity dsp3_tb is
end dsp3_tb;

architecture Behavioral of dsp3_tb is
    -- Constants for the testbench
    constant WIDTH : integer := 11;
    constant FIXED_SIZE : integer := 48;
    constant clk_period : time := 10 ns;

    -- Signals for the DUT (Device Under Test)
    signal clk : std_logic := '0';
    signal rst : std_logic := '1';
    signal u1_i : std_logic_vector(WIDTH - 1 downto 0) := (others => '0');
    signal u2_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal u3_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal res_o : std_logic_vector(FIXED_SIZE - 1 downto 0);

begin
    -- Instantiate the Unit Under Test (UUT)
    uut: entity work.dsp3
        generic map (
            WIDTH => WIDTH,
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

    -- Clock generation process
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
        -- Hold reset for 20 ns
        wait for 20 ns;
        rst <= '0';

        -- Test case 1
        -- Example values for u1_i (step), u2_i (temp_3), and u3_i (frac)
        u1_i <= "00000000010";   -- 2 in binary
        u2_i <= std_logic_vector(to_signed(131072, FIXED_SIZE)); -- 0.5 in binary format (fixed point)
        u3_i <= "000000000000000000000000000000011100110110110110"; --  0.45089018167010253 in binary format  

        -- Wait for a few clock cycles to allow processing
        wait for clk_period * 5;

        -- Report the output value
        report "Value of res_o after Test case 1: " & integer'image(to_integer(signed(res_o)));

        -- Test case 2: Different input values
        u1_i <= "00000000100";   -- 4 in binary
        u2_i <= std_logic_vector(to_signed(65536, FIXED_SIZE)); -- 0.25 in binary format (fixed point)
        u3_i <= "000000000000000000000000000000001111100110011001"; -- 0.123456 in binary format  

        -- Wait for a few clock cycles to allow processing
        wait for clk_period * 5;

        -- Report the output value
        report "Value of res_o after Test case 2: " & integer'image(to_integer(signed(res_o)));

        -- Test case 3: Edge case with zero input
        u1_i <= (others => '0');  -- step = 0
        u2_i <= (others => '0');  -- temp_3 = 0
        u3_i <= "000000000000000000000000000000000000000000000000";  -- frac = 0

        -- Wait for a few clock cycles to allow processing
        wait for clk_period * 5;

        -- Report the output value
        report "Value of res_o after Test case 3: " & integer'image(to_integer(signed(res_o)));

        -- End simulation
        wait;
    end process;

end Behavioral;
