typedef enum {
  MMC_DISCTYPE_NO_DISC,
  
  MMC_DISCTYPE_CD_ROM,
  MMC_DISCTYPE_CD_R,
  MMC_DISCTYPE_CD_RW,
  
  MMC_DISCTYPE_DVD_ROM,
  MMC_DISCTYPE_DVD_RAM,
  MMC_DISCTYPE_DVD_R, //DVD-R
  MMC_DISCTYPE_DVD_RW_RO, //DVD-RW Restricted Overwrite
  MMC_DISCTYPE_DVD_RW_SR, //DVD-RW Sequential Recording
  MMC_DISCTYPE_DVD_R_DL_SR, //DVD-R Dual Layer Sequential Recording
  MMC_DISCTYPE_DVD_R_DL_JR, //DVD-R Dual Layer Jump Recording
  MMC_DISCTYPE_DVD_PRW, //DVD+RW
  MMC_DISCTYPE_DVD_PR, //DVD+R
  MMC_DISCTYPE_DVD_PRW_DL, //DVD+RW Dual Layer
  MMC_DISCTYPE_DVD_PR_DL, //DVD+R Dual Layer
  
  MMC_DISCTYPE_BD_ROM,
  MMC_DISCTYPE_BD_R_SR, //Sequential Recording
  MMC_DISCTYPE_BD_R_RR, //Random Recording
  MMC_DISCTYPE_BD_RE,
  
  MMC_DISCTYPE_HD_DVD_ROM, 
  MMC_DISCTYPE_HD_DVD_R,
  MMC_DISCTYPE_HD_DVD_RAM
} _disctype_t;


/**
  Detects the disc type using the SCSI-MMC GET CONFIGURATION command.

  @param p_cdio the CD object to be acted upon.
  
  @param opt_i_status, if not NULL, on return will be set indicate whether
  the operation was a success (DRIVER_OP_SUCCESS) or if not to some
  other value.
  
  @return the disc type.
 */
_disctype_t
_mmc_get_disctype (const CdIo_t *p_cdio, 
      driver_return_code_t *opt_i_status) {
  
  uint8_t buf[500] = { 0, };
  mmc_cdb_t cdb = {{0, }};
  driver_return_code_t i_status;
  uint8_t *p, *q;
  uint8_t profiles_list_length;
  uint16_t profile_number;
  bool profile_active;
  _disctype_t disctype;
  
  CDIO_MMC_SET_COMMAND(cdb.field, CDIO_MMC_GPCMD_GET_CONFIGURATION);
  CDIO_MMC_SET_READ_LENGTH8(cdb.field, sizeof(buf));
  cdb.field[1] = CDIO_MMC_GET_CONF_ALL_FEATURES;
  cdb.field[3] = 0x0;

  i_status = mmc_run_cmd(p_cdio, 0, &cdb, SCSI_MMC_DATA_READ, 
        sizeof(buf), &buf);
  if (opt_i_status != NULL) *opt_i_status = i_status;
  
  if (i_status == DRIVER_OP_SUCCESS) {
    p = buf + 8; //there is always a profile list feature listed at the first place of the features list
    profiles_list_length = p[3];
    q = p+4;
    disctype = MMC_DISCTYPE_NO_DISC;
    
    while ((disctype == MMC_DISCTYPE_NO_DISC) && (q < p + profiles_list_length)) {
      profile_number = CDIO_MMC_GET_LEN16(q);
      profile_active = q[2] & 0x01;
     
      switch (profile_number) {
        case 0x08: disctype = MMC_DISCTYPE_CD_ROM; break;
        case 0x09: disctype = MMC_DISCTYPE_CD_R; break;
        case 0x0A: disctype = MMC_DISCTYPE_CD_RW; break;
        
        case 0x10: disctype = MMC_DISCTYPE_DVD_ROM; break;
        case 0x11: disctype = MMC_DISCTYPE_DVD_R; break;
        case 0x12: disctype = MMC_DISCTYPE_DVD_RAM; break;
        case 0x13: disctype = MMC_DISCTYPE_DVD_RW_RO; break;
        case 0x14: disctype = MMC_DISCTYPE_DVD_RW_SR; break;
        case 0x15: disctype = MMC_DISCTYPE_DVD_R_DL_SR; break;
        case 0x16: disctype = MMC_DISCTYPE_DVD_R_DL_JR; break;
        case 0x1A: disctype = MMC_DISCTYPE_DVD_PRW; break;
        case 0x1B: disctype = MMC_DISCTYPE_DVD_PR; break;
        case 0x2A: disctype = MMC_DISCTYPE_DVD_PRW_DL; break;
        case 0x2B: disctype = MMC_DISCTYPE_DVD_PR_DL; break;
        
        case 0x40: disctype = MMC_DISCTYPE_BD_ROM; break;
        case 0x41: disctype = MMC_DISCTYPE_BD_R_SR; break;
        case 0x42: disctype = MMC_DISCTYPE_BD_R_RR; break;
        case 0x43: disctype = MMC_DISCTYPE_BD_RE; break;
        
        case 0x50: disctype = MMC_DISCTYPE_HD_DVD_ROM; break;
        case 0x51: disctype = MMC_DISCTYPE_HD_DVD_R; break;
        case 0x52: disctype = MMC_DISCTYPE_HD_DVD_RAM; break;
      }
      if (! profile_active) //whatever the profile is, disctype is reset in not active
        disctype = MMC_DISCTYPE_NO_DISC;
      q += 4;
    }
  }
  
  return disctype; //the first active profile found (profiles are ordered
        //from the most desirable to least desirable) or MMC_DISCTYPE_NO_DISC
}


bool
_mmc_is_disctype_cdrom (_disctype_t disctype) {
  switch (disctype) {
  case MMC_DISCTYPE_CD_ROM:
  case MMC_DISCTYPE_CD_R:
  case MMC_DISCTYPE_CD_RW:
    return true;
  default:
    return false;
  }
}


bool
_mmc_is_disctype_dvd (_disctype_t disctype) {
  switch (disctype) {
  case MMC_DISCTYPE_DVD_ROM:
  case MMC_DISCTYPE_DVD_RAM:
  case MMC_DISCTYPE_DVD_R:
  case MMC_DISCTYPE_DVD_RW_RO:
  case MMC_DISCTYPE_DVD_RW_SR:
  case MMC_DISCTYPE_DVD_R_DL_SR:
  case MMC_DISCTYPE_DVD_R_DL_JR:
  case MMC_DISCTYPE_DVD_PRW:
  case MMC_DISCTYPE_DVD_PR:
  case MMC_DISCTYPE_DVD_PRW_DL:
  case MMC_DISCTYPE_DVD_PR_DL:
    return true;
  default:
    return false;
  }
}


bool
_mmc_is_disctype_bd (_disctype_t disctype) {
  switch (disctype) {
  case MMC_DISCTYPE_BD_ROM:
  case MMC_DISCTYPE_BD_R_SR:
  case MMC_DISCTYPE_BD_R_RR:
  case MMC_DISCTYPE_BD_RE:
    return true;
  default:
    return false;
  }
}

bool
_mmc_is_disctype_hd_dvd (_disctype_t disctype) {
  switch (disctype) {
  case MMC_DISCTYPE_HD_DVD_ROM:
  case MMC_DISCTYPE_HD_DVD_R:
  case MMC_DISCTYPE_HD_DVD_RAM:
    return true;
  default:
    return false;
  }
}


bool
_mmc_is_disctype_overwritable (_disctype_t disctype) {
  switch (disctype) {
  case MMC_DISCTYPE_DVD_RW_RO:
  case MMC_DISCTYPE_DVD_R_DL_JR:
  case MMC_DISCTYPE_DVD_PRW:
  case MMC_DISCTYPE_DVD_PRW_DL:
  case MMC_DISCTYPE_BD_R_RR: //pseudo-overwritable
  case MMC_DISCTYPE_BD_RE:
  case MMC_DISCTYPE_HD_DVD_RAM:
    return true;
  default:
    return false;
  }
}


bool
_mmc_is_disctype_rewritable (_disctype_t disctype) { //discs that needs blanking before re-use
  if (_mmc_is_disctype_overwritable (disctype))
    return true;

  switch (disctype) {
  case MMC_DISCTYPE_CD_RW:
  case MMC_DISCTYPE_DVD_RW_SR:
  case MMC_DISCTYPE_BD_R_SR:
    return true;
  default:
    return false;
  }
}

