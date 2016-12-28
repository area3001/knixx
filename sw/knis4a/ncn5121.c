unsigned short calc_CRC_CCITT(unsigned char* pBuf, unsigned short uLength)
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
