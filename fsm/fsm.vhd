library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.numeric_std.all;
--use work.ip_pkg.all; 

entity fsm is
    generic (
        WIDTH : integer := 11; -- Bit width for various unsigned signals
        FIXED_SIZE : integer := 48 -- Bit width for fixed-point operations    
    );
    port (
        clk : in std_logic;
        reset : in std_logic;
        iradius : in unsigned(WIDTH - 1 downto 0);
        fracr : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        --fracc : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        i_cose : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        --i_sine : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        
        ---------------KOMANDNI INTERFEJS------------------------
        start_i : in std_logic;
        ---------------STATUSNI INTERFEJS------------------------
        ready_o : out std_logic
    );
end fsm;

architecture Behavioral of fsm is
    type state_type is (
        idle, StartLoop, InnerLoop, NextSample,IncrementI, 
        ComputeRPos1, WaitForResult1,     
        Finish
    );
    signal state_reg, state_next : state_type;
    
    signal i_reg, i_next : unsigned(WIDTH - 1 downto 0);
    signal j_reg, j_next : unsigned(WIDTH - 1 downto 0);
    
    signal temp1_rpos_reg, temp1_rpos_next : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal temp1_rpos_delayed, temp1_rpos_delayed1 : std_logic_vector(FIXED_SIZE - 1 downto 0);
    

    signal done : std_logic;
    component dsp1 is
    generic (
        WIDTH : integer := 11; 
        FIXED_SIZE : integer := 48    
    );
    port (
        clk : in  std_logic;   
        rst : in std_logic;
        u1_i : in std_logic_vector(WIDTH - 1 downto 0); 
        u2_i : in std_logic_vector(WIDTH - 1 downto 0); 
        u3_i : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        res_o : out std_logic_vector(FIXED_SIZE -1 downto 0)
        );
    end component;
    
    


begin

-- rpos = (step * ((i_cose * (i - iradius)) + i_sine * (j - iradius))) - fracr) * spacing;
                --temp1= i_cose*(i-iradius)
                --temp2= i_sine*(j-iradius)
                --temp3 = temp1+temp2
                --temp4 = (step*temp3)-fracr 
                --rpos = temp4*spacing
    temp1_rpos_inc_dsp: dsp1
    generic map ( 
        WIDTH => WIDTH,
        FIXED_SIZE => FIXED_SIZE)
    port map(
        clk => clk,
        rst => reset,
        u1_i => std_logic_vector(i_reg),
        u2_i => std_logic_vector(iradius),
        u3_i => i_cose,
        res_o => temp1_rpos_delayed);
    
    delay_temp1_rpos: entity work.delay
    generic map (
        DELAY_CYCLES => 3,
        SIGNAL_WIDTH => FIXED_SIZE
    )
    port map (
        clk => clk,
        rst => reset,
        din => temp1_rpos_delayed,  -- Signal sa DSP-a
        dout => temp1_rpos_delayed1  -- Signal nakon kasnjenja
    ); 

    -- Sekvencijalni proces za registre
    process (clk)
    begin
        if (rising_edge(clk)) then
            if (reset ='1') then
                state_reg <= idle;
                -- Resetovanje svih signala na po?etne vrednosti
                i_reg <= (others =>'0');
                j_reg <= (others => '0');
                temp1_rpos_reg <= (others => '0');
            else
                if (state_reg = ComputeRPos1 and (temp1_rpos_reg /= temp1_rpos_delayed1 or temp1_rpos_reg = (temp1_rpos_reg'range => '0'))) then
                    temp1_rpos_reg <= temp1_rpos_delayed1;
                else
                    state_reg <= state_next;
                    -- Ažuriranje registara sa internim signalima
                    i_reg <= i_next;
                    j_reg <= j_next;
                    temp1_rpos_reg <= temp1_rpos_next;
                end if;
            end if;            
        end if;
    end process;

    -- Kombinacioni proces za odredjivanje sledecih stanja i vrednosti signala
    process(state_reg, start_i, i_reg,j_reg, temp1_rpos_reg, iradius, fracr, i_cose, temp1_rpos_delayed1)
    begin
        --Default assigments
        state_next <= state_reg;
        i_next <= i_reg;
        j_next <= j_reg;
        
        temp1_rpos_next <= temp1_rpos_delayed1;
        
        ready_o <= '0';  -- Default ready_o to '0' unless in idle state
        
        --Logika za FSM
        case state_reg is
            when idle =>
                ready_o <= '1';
            if start_i = '1' then 
                i_next <= to_unsigned(0, WIDTH);
                state_next <= StartLoop;
            else
                state_next <= idle;
            end if;    
            
            when StartLoop =>
                j_next <= TO_UNSIGNED (0, WIDTH);
                state_next <= InnerLoop;
                
            when InnerLoop =>
                state_next <= ComputeRPos1;
            
            when ComputeRPos1 =>
                temp1_rpos_next <= temp1_rpos_reg;
                state_next <= NextSample;
                
            when NextSample =>
                j_next <= j_reg + 1;
            if (j_next >= to_unsigned(2 * to_integer(iradius) - 1, WIDTH)) then
                state_next <= IncrementI;
            else
                state_next <= InnerLoop;
            end if;
            
            when IncrementI =>
                i_next <= i_reg + 1;
            if (i_next >= to_unsigned(2 * to_integer(iradius) - 1, WIDTH)) then
                state_next <= Finish;
            else
                state_next <= StartLoop;
            end if;
            
            when Finish =>
            done <= '1';
            state_next <= idle;
            
            when others =>
            state_next <= idle;     
        end case;
    end process;


end Behavioral;
