library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity dsp5_tb is
end dsp5_tb;

architecture Behavioral of dsp5_tb is
    -- Constants for the testbench
    constant FIXED_SIZE : integer := 48;
    constant clk_period : time := 10 ns;

    -- Signals for the DUT (Device Under Test)
    signal clk : std_logic := '0';
    signal rst : std_logic := '1';
    signal u1_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0'); -- rpos
    signal u2_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0'); -- _IndexSize / 2.0
    signal u3_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0'); -- 0.5
    signal res_o : std_logic_vector(FIXED_SIZE - 1 downto 0);

begin
    -- Instantiate the Unit Under Test (UUT)
    uut: entity work.dsp5
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

    -- Clock generation process
    clk_process : process
    begin
        clk <= '0';
        wait for clk_period / 2;
        clk <= '1';
        wait for clk_period / 2;
    end process;

    -- Stimulus process
    stim_proc: process
    begin
        -- Hold reset for 20 ns
        wait for 20 ns;
        rst <= '0';

        -- Test case 1
        -- Example values for u1_i (rpos), u2_i (_IndexSize / 2.0), and u3_i (0.5)
        u1_i <= "000000000000000000000000000000001010000011000010"; --  0.15699263069083616 u binarnom format
        u2_i <= std_logic_vector(to_signed(4*131072, FIXED_SIZE));
        u3_i <=std_logic_vector(to_signed(131072, FIXED_SIZE));     

        -- Wait for a few clock cycles to allow processing
        wait for clk_period * 5;
        
        -- Test case 1
        -- Example values for u1_i (rpos), u2_i (_IndexSize / 2.0), and u3_i (0.5)
        u1_i <= "000000000000000000000000000000001010000011000000"; --  0.15699263069083616 u binarnom format
        u2_i <= std_logic_vector(to_signed(8*131072, FIXED_SIZE));
        u3_i <=std_logic_vector(to_signed(4*131072, FIXED_SIZE));     

        -- Wait for a few clock cycles to allow processing
        wait for clk_period * 5;

        -- Report the output value
        report "Value of res_o after Test case 1: " & integer'image(to_integer(signed(res_o)));

        -- End simulation
        wait;
    end process;

end Behavioral;