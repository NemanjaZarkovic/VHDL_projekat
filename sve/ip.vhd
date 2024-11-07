----------------------------------------------------------------------------------
-- Company:
-- Engineer:
--
-- Create Date: 04/16/2024 14:10:51 PM
-- Design Name:
-- Module Name: ip - Behavioral
-- Project Name:
-- Target Devices:
-- Tool Versions:
-- Description:
--
-- Dependencies:
--
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.numeric_std.all;
use work.ip_pkg.all; 

entity ip is
    generic (
        WIDTH : integer := 11;            -- Bit width for various unsigned signals
        PIXEL_SIZE : integer := 15;       -- 129 x 129 pixels
        INDEX_ADDRESS_SIZE : integer := 6;
        FIXED_SIZE : integer := 48;       -- Bit width for fixed-point operations
        INDEX_SIZE : integer := 4;        -- Dimension size for the index array
        IMG_WIDTH : integer := 129;       -- Width of the image
        IMG_HEIGHT : integer := 129       -- Height of the image
    );
    port (
        clk : in std_logic;
        reset : in std_logic;
        iradius : in unsigned(WIDTH - 1 downto 0);
        fracr : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        fracc : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        spacing : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        iy : in unsigned(WIDTH - 1 downto 0);
        ix : in unsigned(WIDTH - 1 downto 0);
        step : in unsigned(WIDTH - 1 downto 0);
        i_cose : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        i_sine : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        scale : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        ---------------MEM INTERFEJS ZA SLIKU--------------------
        bram_addr1_o : out std_logic_vector(PIXEL_SIZE-1 downto 0);
        bram_data_i : in std_logic_vector(FIXED_SIZE-1 downto 0);
        bram_en1_o : out std_logic;
        ---------------MEM INTERFEJS ZA IZLAZ--------------------
        addr_do1_o : out std_logic_vector (5 downto 0);
        data1_o : out std_logic_vector (10*FIXED_SIZE + 4*WIDTH - 1 downto 0);          
        c1_data_o : out std_logic;
        bram_we1_o : out std_logic;
        ---------------INTERFEJS ZA ROM--------------------
        rom_data : in std_logic_vector(FIXED_SIZE - 1 downto 0);
        rom_addr : out std_logic_vector(5 downto 0);  
        ---------------KOMANDNI INTERFEJS------------------------
        start_i : in std_logic;
        ---------------STATUSNI INTERFEJS------------------------
        ready_o : out std_logic
    );
end ip;

architecture Behavioral of ip is
    signal state_reg, state_next : state_type;

         component dsp1 is
      generic (
               WIDTH : integer := 11; 
               FIXED_SIZE : integer := 48
              -- ADD_SUB : string:= "add" 
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
        
         component dsp2 is
             generic (
              FIXED_SIZE : integer:= 48;
              ADD_SUB : string:= "add"
              );
        port (clk : in std_logic;
              rst : in std_logic;
              u1_i : in std_logic_vector(FIXED_SIZE - 1 downto 0);
              u2_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); 
              res_o : out std_logic_vector(FIXED_SIZE - 1 downto 0));
        end component;
        
         component dsp3 is
      generic (
               WIDTH : integer := 11;  
               FIXED_SIZE : integer := 48  
              );
      port (
            clk : in  std_logic;    
            rst : in std_logic;
            u1_i : in std_logic_vector(WIDTH - 1 downto 0); 
            u2_i : in signed (FIXED_SIZE -1 downto 0);
            u3_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); 
            res_o : out signed(FIXED_SIZE -1 downto 0) 
           );
        end component;
        
         component dsp4 is
        generic ( 
              FIXED_SIZE : integer := 48);          
        port (clk: in std_logic;
              rst: in std_logic;
              u1_i: in signed(FIXED_SIZE - 1 downto 0);
              spacing : in std_logic_vector(FIXED_SIZE - 1 downto 0);
              res_o: out std_logic_vector(FIXED_SIZE - 1 downto 0));
        end component;
        
         component dsp5 is
      generic ( 
               FIXED_SIZE : integer := 48
              );
      port (
            clk : in  std_logic; 
            rst : in std_logic;
            u1_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); 
            u2_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); 
            u3_i : in std_logic_vector(FIXED_SIZE - 1 downto 0); 
            res_o : out std_logic_vector(FIXED_SIZE -1 downto 0) 
           );
        end component;
        
             component dsp6 is
        generic (
              FIXED_SIZE : integer:= 48;
              WIDTH : integer:= 11
              );
        port (clk: in std_logic;
              rst: in std_logic;
              u1_i: in std_logic_vector(FIXED_SIZE - 1 downto 0); 
              u2_i: in std_logic_vector(WIDTH - 1 downto 0);
              res_o: out std_logic_vector(FIXED_SIZE - 1 downto 0));
        end component;
        
        
        component rom
            generic (
                WIDTH: positive := 48;  
                SIZE: positive := 40;   
                SIZE_WIDTH: positive := 6  
            );
            port (
                clk_a : in std_logic;
                en_a : in std_logic;
                addr_a : in std_logic_vector(SIZE_WIDTH - 1 downto 0);
                data_a_o : out std_logic_vector(WIDTH - 1 downto 0)
            );
        end component;


 type state_type is (
            idle, StartLoop, InnerLoop, 
            ComputeRPos1, ComputeRPos2, ComputeRPos3, ComputeRPos4, ComputeRPos5,
            ComputeCPos1, ComputeCPos2, ComputeCPos3, ComputeCPos4, ComputeCPos5,
            SetRXandCX, BoundaryCheck, PositionValidation, ComputePosition, ProcessSample,
            ComputeDerivatives,
            WaitForData1,WaitForData2,WaitForData3,WaitForData4,
            WaitForData5,WaitForData6,WaitForData7,WaitForData8,
            WaitForData9,WaitForData10,WaitForData11,WaitForData12,
            WaitForData13,WaitForData14,WaitForData15,WaitForData16,
            FetchDXX1_1, FetchDXX1_2, FetchDXX1_3, FetchDXX1_4, ComputeDXX1,
            FetchDXX2_1, FetchDXX2_2, FetchDXX2_3, FetchDXX2_4, ComputeDXX2, 
            FetchDYY1_1, FetchDYY1_2, FetchDYY1_3, FetchDYY1_4, ComputeDYY1,
            FetchDYY2_1, FetchDYY2_2, FetchDYY2_3, FetchDYY2_4, ComputeDYY2, 
        CalculateDerivatives, ApplyOrientationTransform,
        SetOrientations, UpdateIndex, ComputeFractionalComponents, ValidateIndices, 
        ComputeWeightsR, ComputeWeightsC, UpdateIndexArray0, UpdateIndexArray1, CheckNextColumn0, CheckNextColumn1, CheckNextRow0, CheckNextRow1,
        NextSample, IncrementI, Finish
    );

    constant INDEX_SIZE_FP : std_logic_vector(FIXED_SIZE - 1 downto 0) := std_logic_vector(to_unsigned(8*131072, FIXED_SIZE));
    constant HALF_INDEX_SIZE_FP : std_logic_vector(FIXED_SIZE - 1 downto 0) := std_logic_vector(to_unsigned(4*131072, FIXED_SIZE));
    constant HALF_FP : std_logic_vector(FIXED_SIZE - 1 downto 0) := std_logic_vector(to_unsigned(131072, FIXED_SIZE));  -- 0.5

    signal i_reg, i_next : unsigned(WIDTH - 1 downto 0);
    signal j_reg, j_next : unsigned(WIDTH - 1 downto 0);
    signal neg_i_sine : std_logic_vector(FIXED_SIZE - 1 downto 0); 

    signal temp1_rpos_reg, temp1_rpos_next, temp2_rpos_reg, temp2_rpos_next : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal temp3_rpos_reg, temp3_rpos_next, temp4_rpos_reg, temp4_rpos_next : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal temp5_rpos_reg, temp5_rpos_next : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal temp1_cpos_reg, temp1_cpos_next, temp2_cpos_reg, temp2_cpos_next : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal temp3_cpos_reg, temp3_cpos_next, temp4_cpos_reg, temp4_cpos_next : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal temp5_cpos_reg, temp5_cpos_next : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal rpos_reg, cpos_reg : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal rpos_next, cpos_next : std_logic_vector(FIXED_SIZE - 1 downto 0);
    
    signal rx, cx, rx_next, cx_next : std_logic_vector( 2*WIDTH + 2*FIXED_SIZE - 1 downto 0);
    signal addSampleStep, addSampleStep_next : unsigned(WIDTH - 1 downto 0);
    signal r, c, r_next, c_next : signed(2 * WIDTH + 1 - 1 downto 0);
    signal weight, weight_next : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal dxx1_sum_next, dxx2_sum_next, dyy1_sum_next, dyy2_sum_next : std_logic_vector(FIXED_SIZE + 2 - 1 downto 0); -- Accumulators for sum of BRAM data
    signal dxx1_sum_reg, dxx2_sum_reg, dyy1_sum_reg, dyy2_sum_reg : std_logic_vector(FIXED_SIZE + 2 - 1 downto 0);
    signal dxx1, dxx2, dyy1, dyy2 : std_logic_vector(FIXED_SIZE + 2 - 1 downto 0);
    signal dxx1_next, dxx2_next, dyy1_next, dyy2_next : std_logic_vector(FIXED_SIZE + 2 - 1 downto 0);
    signal dxx, dyy, dxx_next, dyy_next :  std_logic_vector(2*FIXED_SIZE + 2 -1 downto 0);
    signal dx, dy, dx_next, dy_next :  std_logic_vector(3*FIXED_SIZE + 3 - 1 downto 0);
    signal ori1, ori2 : unsigned(WIDTH - 1 downto 0);
    signal ori1_next, ori2_next : unsigned(WIDTH - 1 downto 0);
    signal ri, ci, ri_next, ci_next : unsigned(WIDTH - 1 downto 0);
    signal rfrac, cfrac :  std_logic_vector(2*WIDTH + 2*FIXED_SIZE - 1 downto 0);
    signal rfrac_next, cfrac_next : std_logic_vector(2*WIDTH + 2*FIXED_SIZE - 1 downto 0);
    signal rweight1, rweight2, rweight1_next, rweight2_next : std_logic_vector(5*FIXED_SIZE + 2*WIDTH + 4 - 1 downto 0);
    signal cweight1, cweight2, cweight1_next, cweight2_next : std_logic_vector(7*FIXED_SIZE + 4*WIDTH + 5 - 1 downto 0);

    signal done : std_logic;

    -- Definisanje internog signala za kombinatornu logiku
    signal rom_data_reg : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal rom_data_internal : std_logic_vector(FIXED_SIZE - 1 downto 0);
    signal rom_enable : std_logic;
    signal rom_addr_int, rom_addr_next : std_logic_vector(5 downto 0);  -- Dodato za internu adresu

    -- Definisanje internog signala za adrese i podatke za ULAZNI bram
    signal bram_addr1_o_next : std_logic_vector(PIXEL_SIZE-1 downto 0);
    signal bram_data_i_reg : std_logic_vector(FIXED_SIZE-1 downto 0);

    -- Definisanje internog signala za adrese IZLAZNI bram
    signal data1_o_reg, data2_o_reg : std_logic_vector (10*FIXED_SIZE + 4*WIDTH - 1 downto 0);  
    signal bram2_phase : integer range 0 to 1 := 0;  -- Faza pristupa BRAM-u
    signal bram2_phase_next : integer range 0 to 1;  -- Pomocni signal za fazu pristupa BRAM-u   
    signal bram_addr_int : std_logic_vector(INDEX_ADDRESS_SIZE-1 downto 0);
    signal bram_data_out : std_logic_vector(10*FIXED_SIZE + 4*WIDTH - 1 downto 0);     
    signal bram_data_out_next : std_logic_vector(10*FIXED_SIZE + 4*WIDTH - 1 downto 0);  -- Pomocni signal za bram_data_out
    signal bram_en_int : std_logic := '0';
    signal bram_we_int : std_logic := '0';

    constant ONE_FP : signed(rfrac'length - 1 downto 0) := to_signed(2**18, rfrac'length); -- 2^18 predstavlja 1 u fiksnoj ta?ki



begin

    neg_i_sine <= std_logic_vector(-signed(i_sine));

-- rpos = (step * ((i_cose * (i - iradius)) + i_sine * (j - iradius))) - fracr) * spacing;
                --temp1= i_cose*(i-iradius)
                --temp2= i_sine*(j-iradius)
                --temp3 = temp1+temp2
                --temp4 = step*temp3
                --temp5 = temp4-fracr
                --rpos = temp5*spacing
    temp1_rpos_inc_dsp: dsp1
     generic map ( WIDTH => WIDTH,
           FIXED_SIZE => FIXED_SIZE)
    port map(clk => clk,
             rst => reset,
             u1_i => std_logic_vector(i_reg),
             u2_i => std_logic_vector(iradius),
             u3_i => i_cose,
            res_o => temp1_rpos_reg);
            
    temp2_rpos_inc_dsp: dsp1
     generic map ( WIDTH => WIDTH,
           FIXED_SIZE => FIXED_SIZE)
    port map(clk => clk,
             rst => reset,
             u1_i => std_logic_vector(j_reg),
             u2_i => std_logic_vector(iradius),
             u3_i => i_sine,
            res_o => temp2_rpos_reg);
            
     temp3_rpos_inc_dsp: dsp2
     generic map (
           FIXED_SIZE => FIXED_SIZE,
             ADD_SUB => "add")
    port map(clk => clk,
             rst => reset,
             u1_i => temp1_rpos_reg,
             u2_i => temp2_rpos_reg,
            res_o => temp3_rpos_reg);
            
 --cpos = (step * ((- i_sine * (i - iradius)) + i_cose * (j - iradius))) - fracc) * spacing;
          --temp1=  - i_sine*(i-iradius)
          --temp2= i_cose*(j-iradius)
          --temp3 = temp1+temp2
          --temp4 = step*temp3
          --temp5 = temp4-fracc
          --cpos = temp5*spacing
          
     temp1_cpos_inc_dsp: dsp1
     generic map ( WIDTH => WIDTH,
           FIXED_SIZE => FIXED_SIZE)
    port map(clk => clk,
             rst => reset,
             u1_i => std_logic_vector(i_reg),
             u2_i => std_logic_vector(iradius),
             u3_i => neg_i_sine,
            res_o => temp1_cpos_reg);
            
     temp2_cpos_inc_dsp: dsp1
     generic map ( WIDTH => WIDTH,
           FIXED_SIZE => FIXED_SIZE)
    port map(clk => clk,
             rst => reset,
             u1_i => std_logic_vector(j_reg),
             u2_i => std_logic_vector(iradius),
             u3_i => i_cose,
            res_o => temp1_cpos_reg);
            
     temp3_cpos_inc_dsp: dsp2
     generic map (
           FIXED_SIZE => FIXED_SIZE,
             ADD_SUB => "add")
    port map(clk => clk,
             rst => reset,
             u1_i => temp1_cpos_reg,
             u2_i => temp2_cpos_reg,
            res_o => temp3_cpos_reg);
            
    -- Instanciranje ROM-a
    ROM_inst : rom
        generic map (
            WIDTH => FIXED_SIZE,
            SIZE => 40,
            SIZE_WIDTH => 6
        )
        port map (
            clk_a => clk,
            en_a => rom_enable,
            addr_a => rom_addr_int,
            data_a_o => rom_data_internal
        );

    -- Povezivanje signala za ROM
    rom_enable <= '1' when state_reg = ProcessSample else '0';

    -- Sekvencijalni proces za registre
    process (clk)
    begin
        if (rising_edge(clk)) then
            if (reset = '1') then
                state_reg <= idle;
                -- Resetovanje svih signala na pocetne vrednosti
                i_reg <= (others => '0');
                j_reg <= (others => '0');
                ri <= (others => '0');
                ci <= (others => '0');
                addSampleStep <= (others => '0');
                r <= (others => '0');
                c <= (others => '0');
                rpos_reg <= (others => '0');
                cpos_reg <= (others => '0');
                rx <= (others => '0');
                cx <= (others => '0');
                rfrac <= (others => '0');
                cfrac <= (others => '0');
                dx <= (others => '0');
                dy <= (others => '0');
                dxx <= (others => '0');
                dyy <= (others => '0');
                weight <= (others => '0');
                rweight1 <= (others => '0');
                rweight2 <= (others => '0');
                cweight1 <= (others => '0');
                cweight2 <= (others => '0');
                ori1 <= (others => '0');
                ori2 <= (others => '0');
                dxx1 <= (others => '0');
                dxx2 <= (others => '0');
                dyy1 <= (others => '0');
                dyy2 <= (others => '0');
                
                dxx1_sum_reg <= (others => '0');
                dxx2_sum_reg <= (others => '0');
                dyy1_sum_reg <= (others => '0');
                dyy2_sum_reg <= (others => '0');
                
                
                temp1_rpos_reg <= (others => '0');
                temp2_rpos_reg <= (others => '0');
                temp3_rpos_reg <= (others => '0');
                temp4_rpos_reg <= (others => '0');
                temp5_rpos_reg <= (others => '0');

                temp1_cpos_reg <= (others => '0');
                temp2_cpos_reg <= (others => '0');
                temp3_cpos_reg <= (others => '0');
                temp4_cpos_reg <= (others => '0');
                temp5_cpos_reg <= (others => '0');
                
                rom_addr_int <= (others => '0');
                rom_data_reg <= (others => '0'); -- Resetovanje signala za zadrzavanje podataka    
                
                bram2_phase <= 0;         
        
                bram_addr1_o <= (others => '0');  
                bram_data_out <= (others => '0');
                data1_o_reg <= (others => '0');
                data2_o_reg <= (others => '0');
            else
                state_reg <= state_next;
                -- Azuriranje registara sa internim signalima
                i_reg <= i_next;
                j_reg <= j_next;
                ri <= ri_next;
                ci <= ci_next;
                addSampleStep <= addSampleStep_next;
                r <= r_next;
                c <= c_next;
                rpos_reg <= rpos_next;
                cpos_reg <= cpos_next;
                rx <= rx_next;
                cx <= cx_next;
                rfrac <= rfrac_next;
                cfrac <= cfrac_next;
                dx <= dx_next;
                dy <= dy_next;
                dxx <= dxx_next;
                dyy <= dyy_next;
                weight <= weight_next;
                rweight1 <= rweight1_next;
                rweight2 <= rweight2_next;
                cweight1 <= cweight1_next;
                cweight2 <= cweight2_next;
                ori1 <= ori1_next;
                ori2 <= ori2_next;
                dxx1 <= dxx1_next;
                dxx2 <= dxx2_next;
                dyy1 <= dyy1_next;
                dyy2 <= dyy2_next;
                
                dxx1_sum_reg <= dxx1_sum_next;
                dxx2_sum_reg <= dxx2_sum_next;
                dyy1_sum_reg <= dyy1_sum_next;
                dyy2_sum_reg <= dyy2_sum_next;
                
                bram2_phase <= bram2_phase_next;  

                bram_data_out <= bram_data_out_next;
                
                bram_addr1_o <= bram_addr1_o_next;  -- Azuriranje internog signala za adrese

                temp1_rpos_reg <= temp1_rpos_next;
                temp2_rpos_reg <= temp2_rpos_next;
                temp3_rpos_reg <= temp3_rpos_next;
                temp4_rpos_reg <= temp4_rpos_next;
                temp5_rpos_reg <= temp5_rpos_next;
               
                temp1_cpos_reg <= temp1_cpos_next;
                temp2_cpos_reg <= temp2_cpos_next;
                temp3_cpos_reg <= temp3_cpos_next;
                temp4_cpos_reg <= temp4_cpos_next;
                temp5_cpos_reg <= temp5_cpos_next;

                
                
                if rom_enable = '1' then
                    rom_data_reg <= rom_data;
                    rom_addr_int <= rom_addr_next;               
                end if;
                
                if bram2_phase = 0 then
                    data1_o_reg <= bram_data_out;
                elsif bram2_phase = 1 then
                    data2_o_reg <= bram_data_out;
                end if;
            end if;
        end if;
    end process;

    -- Kombinacioni proces za odredjivanje sledecih stanja i vrednosti signala
    process (bram_data_i, bram2_phase, state_reg, start_i, i_reg, j_reg, temp1_rpos_reg, temp2_rpos_reg, temp3_rpos_reg, temp4_rpos_reg, temp5_rpos_reg, temp1_cpos_reg, temp2_cpos_reg, temp3_cpos_reg, temp4_cpos_reg, temp5_cpos_reg, rpos_reg, cpos_reg, iradius, fracr, fracc, spacing, iy, ix, step, i_cose, i_sine, scale, ri, ci, r, c, rx, cx, rfrac, cfrac, dx, dy, dxx, dyy, weight, rweight1, rweight2, cweight1, cweight2, ori1, ori2, dxx1, dxx2, dyy1, dyy2, dxx1_sum_reg, dxx2_sum_reg, dyy1_sum_reg, dyy2_sum_reg, addSampleStep, rom_data_reg, rom_addr_int, data1_o_reg, data2_o_reg, bram_data_out, bram_addr1_o_next)
    begin
        -- Default assignments
        state_next <= state_reg;
        i_next <= i_reg;
        j_next <= j_reg;
        ri_next <= ri;
        ci_next <= ci;
        addSampleStep_next <= addSampleStep;
        r_next <= r;
        c_next <= c;
        rx_next <= rx;
        cx_next <= cx;
        rfrac_next <= rfrac;
        cfrac_next <= cfrac;
        dx_next <= dx;
        dy_next <= dy;
        dxx_next <= dxx;
        dyy_next <= dyy;
        weight_next <= weight;
        rweight1_next <= rweight1;
        rweight2_next <= rweight2;
        cweight1_next <= cweight1;
        cweight2_next <= cweight2;
        ori1_next <= ori1;
        ori2_next <= ori2;
        dxx1_next <= dxx1;
        dxx2_next <= dxx2;
        dyy1_next <= dyy1;
        dyy2_next <= dyy2;
        rpos_next <= rpos_reg;
        cpos_next <= cpos_reg;
        
        temp1_rpos_next <= temp1_rpos_reg;
        temp2_rpos_next <= temp2_rpos_reg;
        temp3_rpos_next <= temp3_rpos_reg;
        temp4_rpos_next <= temp4_rpos_reg;
        temp5_rpos_next <= temp5_rpos_reg;

        temp1_cpos_next <= temp1_cpos_reg;
        temp2_cpos_next <= temp2_cpos_reg;
        temp3_cpos_next <= temp3_cpos_reg;
        temp4_cpos_next <= temp4_cpos_reg;
        temp5_cpos_next <= temp5_cpos_reg;


        dxx1_sum_next <= dxx1_sum_reg;
        dxx2_sum_next <= dxx2_sum_reg;
        dyy1_sum_next <= dyy1_sum_reg;
        dyy2_sum_next <= dyy2_sum_reg;
        
        
        bram_en_int <= '0'; -- Defaultna vrednost za bram_en1_o
        bram_we_int <= '0'; -- Defaultna vrednost za bram_we1_o
        
        rom_addr_next <= rom_addr_int; -- Defaultna vrednost za rom_addr_next
        
        bram2_phase_next <= bram2_phase;  

        data1_o <= (others => '0');
        bram_addr_int <= (others => '0');
        bram_data_out_next <= bram_data_out;
        c1_data_o <= '0';
        ready_o <= '0';


        -- Logika FSM-a
        case state_reg is
            when idle =>
                ready_o <= '1';
                bram_we_int <= '0';
                bram_en_int <= '0';
                if start_i = '1' then
                    i_next <= TO_UNSIGNED (0, WIDTH);
                    state_next <= StartLoop;
                else
                    state_next <= idle;
                end if;

            when StartLoop =>
                    j_next <= TO_UNSIGNED (0, WIDTH);
                state_next <= InnerLoop;

            when InnerLoop =>
                state_next <= ComputeRPos1;
                dxx1_sum_next <= (others => '0');
                dxx2_sum_next <= (others => '0');
                dyy1_sum_next <= (others => '0');
                dyy2_sum_next <= (others => '0');
                
            when ComputeRPos1 =>
                
                temp1_rpos_next <= temp1_rpos_reg;
                state_next <= ComputeRPos2;
            
            when ComputeRPos2 =>
                temp2_rpos_next <= temp2_rpos_reg;
                state_next <= ComputeRPos3;
            
            when ComputeRPos3 =>
                temp3_rpos_next <= temp3_rpos_reg;
                state_next <= ComputeRPos4;
            
            when ComputeRPos4 =>
                temp4_rpos_next <= temp4_rpos_reg;
                state_next <= ComputeRPos5;
            
            when ComputeRPos5 =>
                rpos_next <= rpos_reg;
                state_next <= ComputeCPos1;
            
            when ComputeCPos1 =>
             
                temp1_cpos_next <= temp1_cpos_reg;
                state_next <= ComputeCPos2;
            
            when ComputeCPos2 =>
                temp2_cpos_next <= temp2_cpos_reg;
                state_next <= ComputeCPos3;
            
            when ComputeCPos3 =>
                temp3_cpos_next <= temp3_cpos_reg;
                state_next <= ComputeCPos4;
            
            when ComputeCPos4 =>
                temp4_cpos_next <= temp4_cpos_reg;
                state_next <= ComputeCPos5;
            
            when ComputeCPos5 =>
                cpos_next <= cpos_reg;
                state_next <= SetRXandCX;

            when SetRXandCX =>
                rx_next <= std_logic_vector(
                    to_signed(
                        to_integer(unsigned(rpos_reg)) +
                        to_integer(unsigned(HALF_INDEX_SIZE_FP)) -
                        to_integer(unsigned(HALF_FP)),
                         2*WIDTH + 2*FIXED_SIZE
                    )
                );
                cx_next <= std_logic_vector(
                    to_signed(
                        to_integer(unsigned(cpos_reg)) +
                        to_integer(unsigned(HALF_INDEX_SIZE_FP)) -
                        to_integer(unsigned(HALF_FP)),
                         2*WIDTH + 2*FIXED_SIZE
                    )
                );
                state_next <= BoundaryCheck;

            when BoundaryCheck =>
                if (signed(rx) > -1 and signed(rx) < to_signed(INDEX_SIZE, rx'length)) and
                   (signed(cx) > -1 and signed(cx) < to_signed(INDEX_SIZE, cx'length)) then
                    state_next <= NextSample;
                else
                    state_next <= PositionValidation;
                end if;

            when PositionValidation =>
                addSampleStep_next <= to_unsigned(to_integer(unsigned(scale(FIXED_SIZE - 1 downto 18))), WIDTH);
                
                r_next <= signed(
                            to_signed(
                                to_integer(signed(iy)) + (to_integer(signed(i_reg)) - to_integer(signed(iradius))) * to_integer(signed(step)),
                                2 * WIDTH + 1
                            )
                        );
                c_next <= signed(
                             to_signed(
                                to_integer(signed(ix)) + (to_integer(signed(j_reg)) - to_integer(signed(iradius))) * to_integer(signed(step)),
                                2 * WIDTH + 1
                            )
                        );
                
                state_next <= ComputePosition; 

            when ComputePosition =>
                if (r < 1 + signed(addSampleStep) or r >= IMG_HEIGHT - 1 - signed(addSampleStep) or
                    c < 1 + signed(addSampleStep) or c >= IMG_WIDTH - 1 - signed(addSampleStep)) then
                    state_next <= NextSample;
                else
                    state_next <= ProcessSample;
                end if;

            when ProcessSample =>
                -- Ensure the address is always non-negative
                rom_addr_next <= std_logic_vector(to_unsigned(
                    abs((to_integer(unsigned(rpos_reg)) * to_integer(unsigned(rpos_reg)) + 
                         to_integer(unsigned(cpos_reg)) * to_integer(unsigned(cpos_reg))) + 100000) mod 40, 
                    rom_addr_next'length));
                weight_next <= std_logic_vector(resize(signed(rom_data_reg), FIXED_SIZE));
                state_next <= ComputeDerivatives;

             when ComputeDerivatives =>
                -- Set BRAM addresses for the first pixel for dxx1
                bram_en1_o <= '1';  -- Enable BRAM port
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r) + to_integer(addSampleStep) + 1) * IMG_WIDTH + (to_integer(c) + to_integer(addSampleStep) + 1), PIXEL_SIZE));
                state_next <= WaitForData1;
                
 when WaitForData1 =>
  state_next <= FetchDXX1_1;
  
            when FetchDXX1_1 =>
                -- Capture the data from BRAM for the first pixel of dxx1
                dxx1_sum_next <= std_logic_vector(resize(signed(bram_data_i), FIXED_SIZE + 2));  -- Pro?irenje signala na FIXED_SIZE + 2
                -- Set BRAM addresses for the second pixel for dxx1
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r) - to_integer(addSampleStep)) * IMG_WIDTH + to_integer(c), PIXEL_SIZE));
                state_next <= WaitForData2;
            
             when WaitForData2 =>
  state_next <= FetchDXX1_2;
  
            when FetchDXX1_2 =>
                -- Capture the data from BRAM for the second pixel of dxx1
                dxx1_sum_next <= std_logic_vector(signed(dxx1_sum_reg) + resize(signed(bram_data_i), FIXED_SIZE + 2));                
                -- Set BRAM addresses for the third pixel for dxx1
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r) - to_integer(addSampleStep)) * IMG_WIDTH + (to_integer(c) + to_integer(addSampleStep) + 1), PIXEL_SIZE));
                state_next <= WaitForData3;
                
 when WaitForData3 =>
  state_next <= FetchDXX1_3;
  
            when FetchDXX1_3 =>
                -- Capture the data from BRAM for the third pixel of dxx1
                dxx1_sum_next <= std_logic_vector(signed(dxx1_sum_reg) - resize(signed(bram_data_i), FIXED_SIZE + 2));                
                -- Set BRAM addresses for the fourth pixel for dxx1
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r) + to_integer(addSampleStep) + 1) * IMG_WIDTH + to_integer(c), PIXEL_SIZE));
                state_next <= WaitForData4;
                
  when WaitForData4 =>           
   state_next <= FetchDXX1_4; 
             when FetchDXX1_4 =>
                  -- Capture the data from BRAM for the fourth pixel of dxx1
                dxx1_sum_next <= std_logic_vector(signed(dxx1_sum_reg) - resize(signed(bram_data_i), FIXED_SIZE + 2));                
                state_next <= ComputeDXX1;
            
            when ComputeDXX1 =>
                -- Final computation for dxx1
                dxx1_next <= dxx1_sum_reg;
                -- Set BRAM addresses for the first pixel for dxx2
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r) + to_integer(addSampleStep) + 1) * IMG_WIDTH + to_integer(c + 1), PIXEL_SIZE));
                state_next <= WaitForData5;
                
  when WaitForData5 =>
  state_next <= FetchDXX2_1;
  
            when FetchDXX2_1 =>
                -- Capture the data from BRAM for the first pixel of dxx2
                dxx2_sum_next <= std_logic_vector(resize(signed(bram_data_i), FIXED_SIZE + 2));  -- Pro?irenje signala na FIXED_SIZE + 2
                -- Set BRAM addresses for the second pixel for dxx2
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r) - to_integer(addSampleStep)) * IMG_WIDTH + (to_integer(c) - to_integer(addSampleStep)), PIXEL_SIZE));
                state_next <= WaitForData6;
                
  when WaitForData6 =>
  state_next <= FetchDXX2_2;
  
            when FetchDXX2_2 =>
                -- Capture the data from BRAM for the second pixel of dxx2
                dxx2_sum_next <= std_logic_vector(signed(dxx2_sum_reg) + resize(signed(bram_data_i), FIXED_SIZE + 2)); 
                -- Set BRAM addresses for the third pixel for dxx2
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r) - to_integer(addSampleStep)) * IMG_WIDTH + (to_integer(c) + 1), PIXEL_SIZE));
                state_next <= WaitForData7;
                
     when WaitForData7 =>
  state_next <= FetchDXX2_3; 
        
            when FetchDXX2_3 =>
                -- Capture the data from BRAM for the third pixel of dxx2
                dxx2_sum_next <= std_logic_vector(signed(dxx2_sum_reg) - resize(signed(bram_data_i), FIXED_SIZE + 2)); 
                -- Set BRAM addresses for the fourth pixel for dxx2
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r) + to_integer(addSampleStep) + 1) * IMG_WIDTH + to_integer(c - to_integer(addSampleStep)), PIXEL_SIZE));
                state_next <= WaitForData8;
                
   when WaitForData8 =>
  state_next <= FetchDXX2_4;
           
            when FetchDXX2_4 =>
                -- Capture the data from BRAM for the fourth pixel of dxx2
                dxx2_sum_next <= std_logic_vector(signed(dxx2_sum_reg) - resize(signed(bram_data_i), FIXED_SIZE + 2)); 
                state_next <= ComputeDXX2;
            
            when ComputeDXX2 =>
                -- Final computation for dxx2
                dxx2_next <= dxx2_sum_reg;
                -- Set BRAM addresses for the first pixel for dyy1
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r + 1) * IMG_WIDTH + to_integer(c + to_integer(addSampleStep) + 1)), PIXEL_SIZE));
                state_next <= WaitForData9;
                
  when WaitForData9 =>
  state_next <= FetchDYY1_1;   
         
            when FetchDYY1_1 =>
                -- Capture the data from BRAM for the first pixel of dyy1
                dyy1_sum_next <=  std_logic_vector(resize(signed(bram_data_i), FIXED_SIZE + 2));
                -- Set BRAM addresses for the second pixel for dyy1
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r - to_integer(addSampleStep)) * IMG_WIDTH + to_integer(c - to_integer(addSampleStep))), PIXEL_SIZE));
                state_next <= WaitForData10;
                
 when WaitForData10 =>
  state_next <= FetchDYY1_2; 
            
            when FetchDYY1_2 =>
                -- Capture the data from BRAM for the second pixel of dyy1
                dyy1_sum_next <= std_logic_vector(signed(dyy1_sum_reg) + resize(signed(bram_data_i), FIXED_SIZE + 2)); 
                -- Set BRAM addresses for the third pixel for dyy1
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r - to_integer(addSampleStep)) * IMG_WIDTH + to_integer(c + to_integer(addSampleStep) + 1)), PIXEL_SIZE));
                state_next <= WaitForData11;
                
 when WaitForData11 =>
  state_next <= FetchDYY1_3; 
            
            when FetchDYY1_3 =>
                -- Capture the data from BRAM for the third pixel of dyy1
                dyy1_sum_next <= std_logic_vector(signed(dyy1_sum_reg) - resize(signed(bram_data_i), FIXED_SIZE + 2));                 
                -- Set BRAM addresses for the fourth pixel for dyy1
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r + 1) * IMG_WIDTH + to_integer(c - to_integer(addSampleStep))), PIXEL_SIZE));
                state_next <= WaitForData12;
                
 when WaitForData12 =>
  state_next <= FetchDYY1_4;      
       
            when FetchDYY1_4 =>
                -- Capture the data from BRAM for the fourth pixel of dyy1
                dyy1_sum_next <= std_logic_vector(signed(dyy1_sum_reg) - resize(signed(bram_data_i), FIXED_SIZE + 2)); 
                state_next <= ComputeDYY1;
            
            when ComputeDYY1 =>
                -- Final computation for dyy1
                dyy1_next <= dyy1_sum_reg;
                -- Set BRAM addresses for the first pixel for dyy2
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r + to_integer(addSampleStep) + 1) * IMG_WIDTH + to_integer(c + to_integer(addSampleStep) + 1)), PIXEL_SIZE));
                state_next <= WaitForData13;
                
 when WaitForData13 =>
  state_next <= FetchDYY2_1; 
            
            when FetchDYY2_1 =>
                -- Capture the data from BRAM for the first pixel of dyy2
                dyy2_sum_next <= std_logic_vector(resize(signed(bram_data_i), FIXED_SIZE + 2));
                -- Set BRAM addresses for the second pixel for dyy2
                bram_addr1_o_next <= std_logic_vector(to_unsigned(to_integer(r) * IMG_WIDTH + to_integer(c - to_integer(addSampleStep)), PIXEL_SIZE));
                state_next <= WaitForData14;
                
  when WaitForData14 =>
  state_next <= FetchDYY2_2;    
        
            when FetchDYY2_2 =>
                -- Capture the data from BRAM for the second pixel of dyy2
                dyy2_sum_next <= std_logic_vector(signed(dyy2_sum_reg) + resize(signed(bram_data_i), FIXED_SIZE + 2)); 
                -- Set BRAM addresses for the third pixel for dyy2
                bram_addr1_o_next <= std_logic_vector(to_unsigned(to_integer(r) * IMG_WIDTH + to_integer(c + to_integer(addSampleStep) + 1), PIXEL_SIZE));
                state_next <= WaitForData15;
                
  when WaitForData15 =>
  state_next <= FetchDYY2_3;
            
            when FetchDYY2_3 =>
                -- Capture the data from BRAM for the third pixel of dyy2
                dyy2_sum_next <= std_logic_vector(signed(dyy2_sum_reg) - resize(signed(bram_data_i), FIXED_SIZE + 2)); 
                -- Set BRAM addresses for the fourth pixel for dyy2
                bram_addr1_o_next <= std_logic_vector(to_unsigned((to_integer(r + to_integer(addSampleStep) + 1) * IMG_WIDTH + to_integer(c - to_integer(addSampleStep))), PIXEL_SIZE));
                state_next <= WaitForData16;
                
 when WaitForData16 =>
  state_next <= FetchDYY2_4;
             
            when FetchDYY2_4 =>
                -- Capture the data from BRAM for the fourth pixel of dyy2
                dyy2_sum_next <= std_logic_vector(signed(dyy2_sum_reg) - resize(signed(bram_data_i), FIXED_SIZE + 2)); 
                state_next <= ComputeDYY2;
            
            when ComputeDYY2 =>
                -- Final computation for dyy2
                dyy2_next <= dyy2_sum_reg;
                state_next <= CalculateDerivatives;

            when CalculateDerivatives =>
                dxx_next <= std_logic_vector(resize(signed(weight) * (signed(dxx1) - signed(dxx2)), 2*FIXED_SIZE + 2)); 
                dyy_next <= std_logic_vector(resize(signed(weight) * (signed(dyy1) - signed(dyy2)), 2*FIXED_SIZE + 2)); 
                state_next <= ApplyOrientationTransform;

            when ApplyOrientationTransform =>
                dx_next <= std_logic_vector(resize(signed(i_cose) * signed(dxx) + signed(i_sine) * signed(dyy), 3*FIXED_SIZE + 3)); 
                dy_next <= std_logic_vector(resize(signed(i_sine) * signed(dxx) - signed(i_cose) * signed(dyy), 3*FIXED_SIZE + 3)); 
                state_next <= SetOrientations;

            when SetOrientations =>
                if signed(dx) < 0 then
                    ori1_next <= to_unsigned(0, WIDTH);
                else
                    ori1_next <= to_unsigned(1, WIDTH);
                end if;
                if signed(dy) < 0 then
                    ori2_next <= to_unsigned(2, WIDTH);
                else
                    ori2_next <= to_unsigned(3, WIDTH);
                end if;
                state_next <= UpdateIndex;
            
            when UpdateIndex =>
                -- Check rx and set ri accordingly
                if signed(rx) < 0 then
                    ri_next <= to_unsigned(0, WIDTH);
                elsif signed(rx) >= to_signed(INDEX_SIZE, 2*WIDTH + 2*FIXED_SIZE) then
                    ri_next <= to_unsigned(INDEX_SIZE - 1, WIDTH);
                else
                    ri_next <= to_unsigned(to_integer(signed(rx)), WIDTH);
                end if;

                -- Check ci and update ci accordingly
                if signed(cx) < 0 then
                    ci_next <= to_unsigned(0, WIDTH);
                elsif signed(cx) >= to_signed(INDEX_SIZE, 2*WIDTH + 2*FIXED_SIZE) then
                    ci_next <= to_unsigned(INDEX_SIZE - 1, WIDTH);
                else
                    ci_next <= to_unsigned(to_integer(signed(cx)), WIDTH);
                end if;
                state_next <= ComputeFractionalComponents;

            when ComputeFractionalComponents =>
                -- Compute fractional components
                rfrac_next <= std_logic_vector(signed(rx) - signed(resize(ri, 2*WIDTH + 2*FIXED_SIZE)));
                cfrac_next <= std_logic_vector(signed(cx) - signed(resize(ci, 2*WIDTH + 2*FIXED_SIZE)));
                state_next <= ValidateIndices;
                
            when ValidateIndices =>
                if signed(rfrac) < 0 then
                    rfrac_next <= std_logic_vector(to_signed(0, 2*FIXED_SIZE + 2*WIDTH));
                elsif signed(rfrac) >= ONE_FP then
                    rfrac_next <= std_logic_vector(ONE_FP);
                end if;
            
                if signed(cfrac) < 0 then
                    cfrac_next <= std_logic_vector(to_signed(0, 2*FIXED_SIZE + 2*WIDTH));
                elsif signed(cfrac) >= ONE_FP then
                    cfrac_next <= std_logic_vector(ONE_FP);
                end if;
            
                state_next <= ComputeWeightsR;

            when ComputeWeightsR =>
                rweight1_next <= std_logic_vector(resize(unsigned(dx) * (unsigned(to_signed(1, 2*FIXED_SIZE + 2*WIDTH)) - unsigned(rfrac)), 5*FIXED_SIZE + 2*WIDTH + 4));
                rweight2_next <= std_logic_vector(resize(unsigned(dy) * (unsigned(to_signed(1, 2*FIXED_SIZE + 2*WIDTH)) - unsigned(rfrac)), 5*FIXED_SIZE + 2*WIDTH + 4));
                state_next <= ComputeWeightsC;

            when ComputeWeightsC =>                
                cweight1_next <= std_logic_vector(resize(unsigned(rweight1) * (unsigned(to_signed(1, 2*FIXED_SIZE + 2*WIDTH)) - unsigned(cfrac)), 7*FIXED_SIZE + 4*WIDTH + 5));
                cweight2_next <= std_logic_vector(resize(unsigned(rweight2) * (unsigned(to_signed(1, 2*FIXED_SIZE + 2*WIDTH)) - unsigned(cfrac)), 7*FIXED_SIZE + 4*WIDTH + 5));
                bram2_phase_next <= 0;
                state_next <= UpdateIndexArray0;
                
            when UpdateIndexArray0 =>
                if ri >= 0 and ri < INDEX_SIZE and ci >= 0 and ci < INDEX_SIZE then
                    if bram2_phase = 0 then
                        bram_addr_int <= std_logic_vector(to_unsigned((to_integer(unsigned(ri)) * (INDEX_SIZE * 4)) + to_integer(unsigned(ci)) * 4 + to_integer(unsigned(ori1)), INDEX_ADDRESS_SIZE));
                        bram_data_out_next <= std_logic_vector(resize(unsigned(data1_o_reg), 10*FIXED_SIZE + 4*WIDTH) + resize(unsigned(cweight1), 10*FIXED_SIZE + 4*WIDTH));
                        bram_en_int <= '1';
                        bram_we_int <= '1';
                        bram2_phase_next <= 1;
                        state_next <= UpdateIndexArray1;
                    end if;
                end if;
                data1_o <= bram_data_out;

            when UpdateIndexArray1 =>
                if bram2_phase = 1 then
                    bram_addr_int <= std_logic_vector(to_unsigned((to_integer(unsigned(ri)) * (INDEX_SIZE * 4)) + to_integer(unsigned(ci)) * 4 + to_integer(unsigned(ori2)), INDEX_ADDRESS_SIZE));
                    bram_data_out_next <= std_logic_vector(resize(unsigned(data2_o_reg), 10*FIXED_SIZE + 4*WIDTH) + resize(unsigned(cweight2), 10*FIXED_SIZE + 4*WIDTH));
                    bram_en_int <= '1';
                    bram_we_int <= '1';
                    bram2_phase_next <= 0;
                    state_next <= CheckNextColumn0;
                end if;
                data1_o <= bram_data_out;

            when CheckNextColumn0 =>
                if ci + 1 < INDEX_SIZE then
                    if bram2_phase = 0 then
                        bram_addr_int <= std_logic_vector(to_unsigned(to_integer(unsigned(ri)) * (INDEX_SIZE * 4) + to_integer(unsigned(ci+1)) * 4 + to_integer(unsigned(ori1)), INDEX_ADDRESS_SIZE));
                        bram_data_out_next <= std_logic_vector(resize(unsigned(data1_o_reg), 10*FIXED_SIZE + 4*WIDTH) + resize(unsigned(rweight1) * resize(to_unsigned(to_integer(signed(cfrac)), 2*FIXED_SIZE + 2*WIDTH), 2*FIXED_SIZE + 2*WIDTH), 10*FIXED_SIZE + 4*WIDTH));
                        bram_en_int <= '1';
                        bram_we_int <= '1';
                        bram2_phase_next <= 1;                            
                        state_next <= CheckNextColumn1;
                    end if;
                else
                    state_next <= CheckNextRow0;
                end if;
                data1_o <= bram_data_out;

            when CheckNextColumn1 =>
                if bram2_phase = 1 then
                    bram_addr_int <= std_logic_vector(to_unsigned(to_integer(unsigned(ri)) * (INDEX_SIZE * 4) + to_integer(unsigned(ci+1)) * 4 + to_integer(unsigned(ori2)), INDEX_ADDRESS_SIZE));
                    bram_data_out_next <= std_logic_vector(resize(unsigned(data2_o_reg), 10*FIXED_SIZE + 4*WIDTH) + resize(unsigned(rweight2) * resize(to_unsigned(to_integer(signed(cfrac)), 2*FIXED_SIZE + 2*WIDTH), 2*FIXED_SIZE + 2*WIDTH), 10*FIXED_SIZE + 4*WIDTH));
                    bram_en_int <= '1';
                    bram_we_int <= '1';
                    bram2_phase_next <= 0;
                    state_next <= CheckNextRow0;
                end if;
                data1_o <= bram_data_out;

            when CheckNextRow0 =>
                if ri + 1 < INDEX_SIZE then
                    if bram2_phase = 0 then
                        bram_addr_int <= std_logic_vector(to_unsigned(to_integer(unsigned(ri + 1)) * (INDEX_SIZE * 4) + to_integer(unsigned(ci)) * 4 + to_integer(unsigned(ori1)), INDEX_ADDRESS_SIZE));
                        bram_data_out_next <= std_logic_vector(resize(unsigned(data1_o_reg), 10 * FIXED_SIZE + 4 * WIDTH) + resize(unsigned(dx) * unsigned(rfrac) * (unsigned(to_signed(1, 2 * WIDTH + 2 * FIXED_SIZE)) - unsigned(cfrac)), 10*FIXED_SIZE + 4 * WIDTH));
                        bram_en_int <= '1';
                        bram_we_int <= '1';
                        bram2_phase_next <= 1;
                        state_next <= CheckNextRow1;
                    end if;
                else
                    state_next <= NextSample;
                end if;
                data1_o <= bram_data_out;

            when CheckNextRow1 =>
                if bram2_phase = 1 then
                    bram_addr_int <= std_logic_vector(to_unsigned(to_integer(unsigned(ri + 1)) * (INDEX_SIZE * 4) + to_integer(unsigned(ci)) * 4 + to_integer(unsigned(ori2)), INDEX_ADDRESS_SIZE));
                    bram_data_out_next <= std_logic_vector(resize(unsigned(data2_o_reg), 10 * FIXED_SIZE + 4 * WIDTH) + resize(unsigned(dy) * unsigned(rfrac) * (unsigned(to_signed(1, 2 * WIDTH + 2 * FIXED_SIZE)) - unsigned(cfrac)), 10*FIXED_SIZE + 4 * WIDTH));
                    bram_en_int <= '1';
                    bram_we_int <= '1';
                    bram2_phase_next <= 0;
                    state_next <= NextSample;
                end if;
                data1_o <= bram_data_out;

            when NextSample =>
                j_next <= j_reg + 1;
                if (j_next >= to_unsigned(2 * to_integer(iradius), WIDTH)) then
                    state_next <= IncrementI;
                else
                    state_next <= InnerLoop;
                end if;

            when IncrementI =>
                i_next <= i_reg + 1;
                if (i_next >= to_unsigned(2 * to_integer(iradius), WIDTH)) then
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

    -- Azuriranje izlaznih portova 
    addr_do1_o <= bram_addr_int;
    c1_data_o <= bram_en_int;
    bram_we1_o <= bram_we_int;
    rom_addr <= rom_addr_int;  -- Azuriranje rom_addr signala
    
end Behavioral;