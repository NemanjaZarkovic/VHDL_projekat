library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity dsp6_tb is
end dsp6_tb;

architecture Behavioral of dsp6_tb is
   constant FIXED_SIZE : integer := 48;
   constant WIDTH : integer := 11;
   
    -- Signals for the DUT (Device Under Test)
    signal clk : std_logic := '0';
    signal rst : std_logic := '1';
    signal u1_i : std_logic_vector(FIXED_SIZE - 1 downto 0):= (others => '0');
    signal u2_i : std_logic_vector(WIDTH - 1 downto 0):= (others => '0');
    signal res_o : std_logic_vector(FIXED_SIZE - 1 downto 0);
    -- Clock period definition
    constant clk_period : time := 10 ns;

begin

    -- Instantiate the Unit Under Test (UUT)
    uut: entity work.dsp6
        generic map (
            FIXED_SIZE => FIXED_SIZE,
            WIDTH => WIDTH
        )
        port map (
            clk => clk,
            rst => rst,
            u1_i => u1_i,
            u2_i => u2_i,
            res_o => res_o
        );
    -- Clock generation
    clk_process : process
    begin
        clk <= '0';
        wait for clk_period / 2;
        clk <= '1';
        wait for clk_period / 2;
    end process;

   

    -- Testbench process
    stimulus_process : process
    begin
         -- Hold reset state for 20 ns
        wait for 20 ns;
        rst <= '0';

          -- Test case 1: u1_i = 1000, u2_i = 50
        u1_i <= "000000000000000000000000001001101011111000001110"; -- 1000000000000000 in binary
        u2_i <= "00000000001"; -- 50 in binary
        wait for CLK_PERIOD;

--        -- Test case 2
--        u1_i <= std_logic_vector(to_signed(-30000, 48)); -- Example value
--        u2_i <= std_logic_vector(to_unsigned(2000, 11)); -- Example value
--        wait for CLK_PERIOD;

--        -- Test case 3
--        u1_i <= std_logic_vector(to_signed(150000, 48)); -- Example value
--        u2_i <= std_logic_vector(to_unsigned(500, 11)); -- Example value
--        wait for CLK_PERIOD;

        -- Additional test cases as needed...

        -- Stop the simulation
        wait for 100 ns;
        wait;
    end process;

end Behavioral;
