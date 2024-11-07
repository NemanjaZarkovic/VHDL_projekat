library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity fsm_tb is
    -- No ports in testbench
end fsm_tb;

architecture Behavioral of fsm_tb is
    constant WIDTH : integer := 11; -- Bit width for various unsigned signals
    constant FIXED_SIZE : integer := 48; -- Bit width for fixed-point operations    
    -- Component declaration for the Unit Under Test (UUT)
    component fsm is
        port (
            clk : in std_logic;
            reset : in std_logic;
            iradius : in unsigned(WIDTH - 1 downto 0);
            fracr : in std_logic_vector(FIXED_SIZE - 1 downto 0);
            i_cose : in std_logic_vector(FIXED_SIZE - 1 downto 0);
            start_i : in std_logic;
            ready_o : out std_logic
        );
    end component;

    -- Testbench signals
    signal clk : std_logic := '0';
    signal reset : std_logic := '0';
    signal iradius : unsigned(WIDTH - 1 downto 0) := (others => '0');
    signal fracr : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal i_cose : std_logic_vector(FIXED_SIZE - 1 downto 0) := (others => '0');
    signal start_i : std_logic := '0';
    signal ready_o : std_logic;

    -- Clock period definition
    constant clk_period : time := 10 ns;

begin

    -- Instantiate the Unit Under Test (UUT)
    uut: fsm

        port map (
            clk => clk,
            reset => reset,
            iradius => iradius,
            fracr => fracr,
            i_cose => i_cose,
            start_i => start_i,
            ready_o => ready_o
        );

    -- Clock process definitions
    clk_process :process
    begin
        clk <= '0';
        wait for clk_period/2;
        clk <= '1';
        wait for clk_period/2;
    end process;

    -- Stimulus process
    stim_proc: process
    begin		
        -- Initialize inputs
        reset <= '1';
        wait for clk_period*2;
        reset <= '0';
        
        -- Apply test stimulus
        iradius <= to_unsigned(5, WIDTH);
        fracr <= (others => '0');
        i_cose <= (others => '0');
        start_i <= '1';
        
        wait for clk_period;
        start_i <= '0';

        -- Wait for the FSM to process and become ready
        wait until ready_o = '1';

        -- Check further transitions by modifying input signals
        fracr <= (others => '1');
        i_cose <= (others => '1');
        start_i <= '1';

        wait for clk_period;
        start_i <= '0';

        -- Wait for the FSM to complete the processing
        wait until ready_o = '1';

        -- Stop simulation
        wait;
    end process;

end Behavioral;
