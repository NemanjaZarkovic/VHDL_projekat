library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity dsp4_tb is
end dsp4_tb;

architecture Behavioral of dsp4_tb is
    constant FIXED_SIZE : integer := 48;

    -- Signals for the DUT (Device Under Test)
    signal clk : std_logic := '0';
    signal rst : std_logic := '1';
    signal u1_i : signed(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal spacing : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    --signal res_o : std_logic_vector(2*FIXED_SIZE-1 downto 0);
    signal res_o : std_logic_vector(FIXED_SIZE - 1 downto 0);

    -- Clock period definition
    constant clk_period : time := 10 ns;

begin
    -- Instantiate the Unit Under Test (UUT)
    uut: entity work.dsp4
        generic map (
            FIXED_SIZE => FIXED_SIZE
        )
        port map (
            clk => clk,
            rst => rst,
            u1_i => u1_i,
            spacing => spacing,
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
       -- u1_i <= "000000000000000000000000001001101011111000001110";
         u1_i <= "000000000000000000000000001000101011111000001110";
        spacing <= std_logic_vector(to_signed(4*131072, FIXED_SIZE));
        wait for clk_period * 2;

--        -- End simulation
        wait;
    end process;

end Behavioral;
