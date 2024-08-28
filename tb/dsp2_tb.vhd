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
    signal u1_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal u2_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal ADD_SUB : std_logic := '0';  -- '0' for add, '1' for subtract
    signal res_o : std_logic_vector(FIXED_SIZE - 1 downto 0);
    
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
            ADD_SUB => ADD_SUB,
            res_o => res_o
        );

    -- Clock process definitions
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

        -- Test case 1: Addition
        ADD_SUB <= '0';  -- Perform addition
        u1_i <= "000000000000000000000000000000001010000011000010"; --  0.15699263069083616 u binarnom format
        u2_i <= "000000000000000000000000000011001110101000000100"; -- 3.2285336566057454 u binarnom formatu
        wait for clk_period * 5;
        report "Value of res_o after Test case 1 (Addition): " & integer'image(to_integer(signed(res_o)));

        -- Test case 2: Subtraction
        ADD_SUB <= '1';  -- Perform subtraction
        u1_i <= "000000000000000000000000000000111001110101110010"; -- 0.90375518798828125 u binarnom formatu
        u2_i <= "000000000000000000000000000000011100110110110110"; --  0.45089018167010253 u binarnom formatu
        wait for clk_period * 5;
        report "Value of res_o after Test case 2 (Subtraction): " & integer'image(to_integer(signed(res_o)));

        -- End simulation
        wait;
    end process;

end Behavioral;
