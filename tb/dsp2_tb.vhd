library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity dsp2_tb is
end dsp2_tb;

architecture Behavioral of dsp2_tb is
    constant FIXED_SIZE : integer := 48;
    
    -- Signals for the DUT (Device Under Test)
    signal clk : std_logic := '0';
    signal rst : std_logic := '1';
    signal u1_i : signed(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal u2_i : signed(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal res_o : signed(FIXED_SIZE - 1 downto 0);
    
    -- Clock period definition
    constant clk_period : time := 10 ns;

begin
    -- Instantiate the Unit Under Test (UUT)
    uut: entity work.dsp2
        generic map (
            FIXED_SIZE => FIXED_SIZE
        )
        port map (
            clk => clk,
            rst => rst,
            u1_i => u1_i,
            u2_i => u2_i,
            res_o => res_o
        );

    -- Clock process definitions
    clk_process :process
    begin
        clk <= '0';
        wait for clk_period / 2;
        clk <= '1';
        wait for clk_period / 2;
    end process;

    -- Stimulus process
    stim_proc: process
    begin
        -- Hold reset state for 20 ns
        wait for 20 ns;
        rst <= '0';

        -- Test case 1
        u1_i <= to_signed(100, FIXED_SIZE);
        u2_i <= to_signed(50, FIXED_SIZE);
        wait for clk_period * 100;
        report "Value of res_o after Test case 1: " & integer'image(to_integer(res_o));

        -- Test case 2
        u1_i <= to_signed(-100, FIXED_SIZE);
        u2_i <= to_signed(100, FIXED_SIZE);
        wait for clk_period * 10;
        report "Value of res_o after Test case 2: " & integer'image(to_integer(res_o));

--        -- Test case 3
--        u1_i <= to_signed(123456789, FIXED_SIZE);
--        u2_i <= to_signed(-987654321, FIXED_SIZE);
--        wait for clk_period * 10;
--        report "Value of res_o after Test case 3: " & integer'image(to_integer(res_o));

--        -- Test case 4
--        u1_i <= to_signed(500, FIXED_SIZE);
--        u2_i <= to_signed(700, FIXED_SIZE);
--        wait for clk_period * 10;
--        report "Value of res_o after Test case 4: " & integer'image(to_integer(res_o));

        -- End simulation
        wait;
    end process;
end Behavioral;