library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity dsp3_tb is
end dsp3_tb;

architecture Behavioral of dsp3_tb is
    constant WIDTH : integer := 11;
    constant FIXED_SIZE : integer := 48;

    -- Signals for the DUT (Device Under Test)
    signal clk : std_logic := '0';
    signal rst : std_logic := '1';
    signal u1_i : std_logic_vector(WIDTH - 1 downto 0) := (others => '0');
    signal u2_i : signed(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal u3_i : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal res_o : signed(FIXED_SIZE - 1 downto 0);

    -- Clock period definition
    constant clk_period : time := 10 ns;

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

        -- Test case 1: step = 10, temp_3 = 20, frac = 15
        u1_i <= std_logic_vector(to_signed(1, WIDTH));  -- step = 10
        u2_i <= to_signed(2, FIXED_SIZE);               -- temp_3 = 20
        u3_i <=std_logic_vector(to_signed(3, FIXED_SIZE)); -- frac = 15
        wait for clk_period * 2;
        report "Value of res_o after Test case 1: " & integer'image(to_integer(res_o));

--        -- Test case 2: step = 5, temp_3 = 30, frac = 10
--        u1_i <= std_logic_vector(to_signed(5, WIDTH));   -- step = 5
--        u2_i <= to_signed(30, FIXED_SIZE);               -- temp_3 = 30
--        u3_i <= std_logic_vector(to_signed(10, FIXED_SIZE)); -- frac = 10
--        wait for clk_period * 2;
--        report "Value of res_o after Test case 2: " & integer'image(to_integer(res_o));

--        -- Test case 3: step = 7, temp_3 = -25, frac = 35
--        u1_i <= std_logic_vector(to_signed(7, WIDTH));   -- step = 7
--        u2_i <= to_signed(-25, FIXED_SIZE);              -- temp_3 = -25
--        u3_i <= std_logic_vector(to_signed(35, FIXED_SIZE)); -- frac = 35
--        wait for clk_period * 2;
--        report "Value of res_o after Test case 3: " & integer'image(to_integer(res_o));

--        -- Test case 4: step = -3, temp_3 = 15, frac = 5
--        u1_i <= std_logic_vector(to_signed(-3, WIDTH));  -- step = -3
--        u2_i <= to_signed(15, FIXED_SIZE);               -- temp_3 = 15
--        u3_i <= std_logic_vector(to_signed(5, FIXED_SIZE)); -- frac = 5
--        wait for clk_period * 2;
--        report "Value of res_o after Test case 4: " & integer'image(to_integer(res_o));

        -- End simulation
        wait;
    end process;

end Behavioral;
