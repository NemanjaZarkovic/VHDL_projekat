library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity dsp7_tb is
end dsp7_tb;

architecture Behavioral of dsp7_tb is
    constant WIDTH : integer := 11;
    constant clk_period : time := 10 ns;

    -- Signals for the DUT (Device Under Test)
    signal clk : std_logic := '0';
    signal rst : std_logic := '1';
    signal u1_i : std_logic_vector(WIDTH - 1 downto 0) := (others => '0');
    signal u2_i : std_logic_vector(WIDTH - 1 downto 0) := (others => '0');
    signal u3_i : std_logic_vector(WIDTH - 1 downto 0) := (others => '0');
    signal u4_i : std_logic_vector(WIDTH - 1 downto 0) := (others => '0');
    signal res_o : std_logic_vector(WIDTH - 1 downto 0);

begin
    -- Instantiate the Unit Under Test (UUT)
    uut: entity work.dsp7
        generic map (
            WIDTH => WIDTH
        )
        port map (
            clk => clk,
            rst => rst,
            u1_i => u1_i,
            u2_i => u2_i,
            u3_i => u3_i,
            u4_i => u4_i,
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

        -- Test case 1: Basic operation
        u1_i <= std_logic_vector(to_unsigned(10, WIDTH));  -- i/j = 10
        u2_i <= std_logic_vector(to_unsigned(5, WIDTH));   -- iradius = 5
        u3_i <= std_logic_vector(to_unsigned(2, WIDTH));   -- step = 2
        u4_i <= std_logic_vector(to_unsigned(3, WIDTH));   -- iy = 3
        wait for clk_period * 5;  -- Wait for 5 clock cycles to allow pipeline delay

        -- Check output after pipeline delay
--        wait for clk_period;
--        report "Test case 1: res_o = " & integer'image(to_integer(unsigned(res_o)));

        -- Test case 2: Negative result test (subtraction)
        u1_i <= std_logic_vector(to_unsigned(3, WIDTH));   -- i/j = 3
        u2_i <= std_logic_vector(to_unsigned(8, WIDTH));   -- iradius = 8
        u3_i <= std_logic_vector(to_unsigned(4, WIDTH));   -- step = 4
        u4_i <= std_logic_vector(to_unsigned(1, WIDTH));   -- iy = 1
        wait for clk_period * 5;  -- Wait for 5 clock cycles to allow pipeline delay

--        -- Check output after 1 clock cycle
--        wait for clk_period;
--        report "Test case 2: res_o = " & integer'image(to_integer(unsigned(res_o)));

--        -- Test case 3: Zero result test
--        u1_i <= (others => '0');  -- i/j = 0
--        u2_i <= (others => '0');  -- iradius = 0
--        u3_i <= (others => '0');  -- step = 0
--        u4_i <= (others => '0');  -- iy = 0
--        wait for clk_period;  -- Wait for 1 clock cycle

--        -- Check output after 1 clock cycle
--        wait for clk_period;
--        report "Test case 3: res_o = " & integer'image(to_integer(unsigned(res_o)));

        -- End simulation
        wait;
    end process;

end Behavioral;
