bool NCN_slave_collision;
bool NCN_receive_error;
bool NCN_transceiver_error;
bool NCN_protocol_error;
bool NCN_thermal_warning;


unsigned short calc_CRC_CCITT(unsigned char* pBuf, unsigned short uLengith)
{
  unsigned short u_crc_ccitt;
  for (u_crc_ccitt = 0xFFFF; uLength−−; p++)
  {
    u_crc_ccitt = get_CRC_CCITT(u_crc_ccitt, *p);
  }
  return u_crc_ccitt;
}

unsigned short get_CRC_CCITT(unsigned short u_crc_val, unsigned char btVal)
{
  u_crc_val = ((unsigned char)(u_crc_val >> 8)) | (u_crc_val << 8);
  u_crc_val ^= btVal;
  u_crc_val ^= ((unsigned char)(u_crc_val & 0xFF)) >> 4;
  u_crc_val ^= u_crc_val << 12;
  u_crc_val ^= (u_crc_val & 0xFF) << 5;
  return u_crc_val;
}

/**
 * @brief 	Resets the NCN5151
 * @return	Returns U_Reset.ind when entering Normal State
 */
char NCN_Reset ()
{

}

/**
 * @brief	Get the internal communication state of the NCN5121
 * @return	Returns U_State.ind, global bool flags are also updated
 */
char NCN_State ()
{

}

/**
 * @brief
 */
void NCN_SetBusy ()
{
}

/**
 * @brief
 */
void NCN_QuitBusy ()
{

}

/**
 * @brief
 */
void NCN_BusMonitor ()
{

}

/**
 * @brief
 * @param
 * @return
 */
char NCN_SetAddress (unsigned short address)
{

}


