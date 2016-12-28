
enum mode {
	RESET,
	POWER_UP,
	POWER_UP_STOP,
	SYNC,
	STOP,
	NORMAL
};


#define U_Reset.req		0x01
#define U_Reset.ind		0x03
#define U_State.req		0x02

#define U_State_ind_sc		0x80 //Slave Collision bit mask (see all page 34)
#define U_State_ind_re		0x40 //Receive Error bit mask
#define U_State_ind_te		0x20 //Transceiver Error bit mask
#define U_State_ind_pe		0x10 //Protocol Error bit mask
#define U_state_ind_tw		0x08 //Thermal Warning bit mask

#define U_SetBusy.req		0x03
#define U_QuitBusy.req  	0x04
#define U_Busmon.req		0x05

#define U_SetAddress.req  	0xF1

#define U_Configure.req		0x18 //Configure base
#define U_Configure_req_p	0x04 //Auto polling bit mask
#define U_Configure_req_c	0x02 //CRC_CCITT bit mask
#define U_Configure_req_m	0x01 //Frame end with Marker bit mask
		
#define U_Configure_ind_b	0x40 //Busy Mode bit mask	(page 34, 47)
#define U_Configure_ind_aa  	0x20 //Auto Acknowledge bit mask (page 35)
#define U_Configure_ind_ap	0x10 //Auto Polling bit mask (page 38)
#define U_Configure_ind_c	0x08 //CRC-CCITT bit mask (page 38)
#define U_Configure_ind_m       0x04 //Frame end with Marker bit mask (page 38)

#define U_SetRepetition.req	0xF2
#define U_SetRepetition_req_bbb 0x70 //Busy counter bit mask
#define U_SetRepetition_req_nnn 0x07 //Nack counter bit mask

#define U_SystemStat.req	0x0D 
#define U_SystemStat.ind	0x4B
#define U_SystemStat_ind_V20V   0x80 //V20V regulator bit mask
#define U_SystemStat_ind_VDD2	0x40 //DC2 regulator bit mask
#define U_SystemStat_ind_VBUS	0x20 //KNX VBUS voltage bit mask
#define U_SystemStat_ind_VFILT	0x10 //Cap Tank Voltage bit mask
#define U_SystemStat_ind_XTAL   0x08 //Xtal Osc frequency bit mask
#define U_SystemStat_ind_TW	0x04 //Thermal warning condition bit mask
#define U_SystemStat_ind_MODE	0x03 //Operation mode bit mask

#define U_StopMode.req		0x0E
#define U_StopMode.ind		0x2B
#define U_ExitStopMode.req	0x0F

#define U_IntRegWr.req		0x28 //Internal Register Write base
#define U_IntRegWr_req_aa	0x03 //Internal Register Write address bit mask

#define U_IntRegRd.ind		0x38 //Internal Register Read base
#define U_IntRegRd_ind_aa	0x03 //Internal Register Read address bit mask

#define U_L_DataStart.req	0x80 
#define U_L_DataCont.req	0x80 //Data Cont base
#define U_L_DataCont_req_ii	0x3F //Data Control index bit mask
#define U_L_DataOffset.req	0x08 //Data Offset base
#define U_L_DataOffset_req_ii   0x07 //Data Offset index bit mask
#define U_L_DataEnd.req		0x40 //Data End base
#define U_L_DataEnd_req_ll	0x3F //Data End index bit mask

#define U_FrameState.ind	0x13 //Frame State Base
#define U_FrameState_ind_re	0x80
#define U_FrameState_ind_ce	0x40
#define U_FrameState_ind_te	0x20
#define U_FrameState_ind_res	0x08

#define L_Data.con		0x0B //Data base
