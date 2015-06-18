----------------------------------------------------------------------------------
-- Company: SoCDesign
-- Engineer: Kristjan Lužkov
-- 
-- Create Date:    13:56:54 04/10/2015
-- Design Name: 2-Channel Mono Audio Signal Mixer
-- Module Name:    mixer - Behavioral
-- Project Name: Audio Mixer
-- Target Devices:
-- Tool versions:
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
use IEEE.STD_LOGIC_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;
 
-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;
 
entity Mixer is
    generic(INTBIT_WIDTH  : positive;
            FRACBIT_WIDTH : positive);
    Port(CLK             : in  STD_LOGIC;
         RESET           : in  STD_LOGIC;
         IN_CH0          : in  std_logic_vector((INTBIT_WIDTH - 1) downto 0);
         IN_CH1          : in  std_logic_vector((INTBIT_WIDTH - 1) downto 0);
         IN_CH2          : in  std_logic_vector((INTBIT_WIDTH - 1) downto 0);
         IN_CH3          : in  std_logic_vector((INTBIT_WIDTH - 1) downto 0);
         OUT_MIX         : out std_logic_vector((INTBIT_WIDTH - 1) downto 0);
         OUT_mixer_ready : out STD_LOGIC
    );
end Mixer;
 
architecture Behavioral of Mixer is
    COMPONENT AmplifierFP
        PORT(
            CLK     : IN  std_logic;
            RESET   : IN  std_logic;
            IN_SIG  : IN  signed((INTBIT_WIDTH - 1) downto 0);
            IN_COEF : IN  signed(((INTBIT_WIDTH + FRACBIT_WIDTH) - 1) downto 0);
            OUT_AMP : OUT signed((INTBIT_WIDTH - 1) downto 0);
            OUT_RDY : OUT std_logic
        );
    END COMPONENT;
 
    signal mix_ch0 : signed((INTBIT_WIDTH - 1) downto 0) := (others => '0');
    signal mix_ch1 : signed((INTBIT_WIDTH - 1) downto 0) := (others => '0');
    signal mix_ch2 : signed((INTBIT_WIDTH - 1) downto 0) := (others => '0');
    signal mix_ch3 : signed((INTBIT_WIDTH - 1) downto 0) := (others => '0');
    signal mix_out : signed((INTBIT_WIDTH - 1) downto 0) := (others => '0');
 
    signal mix_rdy0 : std_logic;
    signal mix_rdy1 : std_logic;
    signal mix_rdy2 : std_logic;
    signal mix_rdy3 : std_logic;
 
begin
    Ch0_amp : AmplifierFP PORT MAP(
            CLK     => CLK,
            RESET   => RESET,
            IN_SIG  => signed(IN_CH0),
            IN_COEF => "00000000000000000000000010000000",
            OUT_AMP => mix_ch0,
            OUT_RDY => mix_rdy0
        );
 
    Ch1_amp : AmplifierFP PORT MAP(
            CLK     => CLK,
            RESET   => RESET,
            IN_SIG  => signed(IN_CH1),
            IN_COEF => "00000000000000000000000010000000",
            OUT_AMP => mix_ch1,
            OUT_RDY => mix_rdy1
        );
 
    Ch2_amp : AmplifierFP PORT MAP(
            CLK     => CLK,
            RESET   => RESET,
            IN_SIG  => signed(IN_CH2),
            IN_COEF => "00000000000000000000000010000000",
            OUT_AMP => mix_ch2,
            OUT_RDY => mix_rdy2
        );
 
    Ch3_amp : AmplifierFP PORT MAP(
            CLK     => CLK,
            RESET   => RESET,
            IN_SIG  => signed(IN_CH3),
            IN_COEF => "00000000000000000000000010000000",
            OUT_AMP => mix_ch3,
            OUT_RDY => mix_rdy3
        );
 
    mixing_process : process(CLK)
    begin
        if (CLK'event and CLK = '1') then
            if (mix_rdy0 = '1' and mix_rdy1 = '1' and mix_rdy2 = '1' and mix_rdy3 = '1') then
                mix_out         <= mix_ch0 + mix_ch1 + mix_ch2 + mix_ch3;
                OUT_mixer_ready <= '1';
            else
                OUT_mixer_ready <= '0';
            end if;
        end if;
    end process;
    OUT_MIX <= STD_LOGIC_VECTOR(mix_out);
 
end Behavioral;