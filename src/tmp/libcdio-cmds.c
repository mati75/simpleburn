#define MMC_TRACK_INFORMATION_DATA_SIZE 48
#define CDIO_MMC_GPCMD_READ_TRACK_INFORMATION 0x52
/**
	Retrieves MMC TRACK_INFORMATION DATA.

	@param p_cdio the CD object to be acted upon.
	
	@param i_status, on return will be set indicate whether the operation was
	a success (DRIVER_OP_SUCCESS) or if not to some other value.
	
	@param mmc_track_information_data, in return. Free when no longer needed.
 */
void
_mmc_get_track_information_data( const CdIo_t *p_cdio, 
				driver_return_code_t *i_status,
				uint8_t **mmc_track_information_data,
				track_t trackno) {
	mmc_cdb_t cdb = {{0, }};
	
	*mmc_track_information_data = (uint8_t*) malloc (MMC_TRACK_INFORMATION_DATA_SIZE * sizeof (uint8_t));
	memset (*mmc_track_information_data, 0, MMC_TRACK_INFORMATION_DATA_SIZE);
	CDIO_MMC_SET_COMMAND (cdb.field, CDIO_MMC_GPCMD_READ_TRACK_INFORMATION);
	CDIO_MMC_SET_READ_LENGTH16 (cdb.field, MMC_TRACK_INFORMATION_DATA_SIZE);
	cdb.field[1] = 0x01; //to specify track number
	cdb.field[5] = trackno;

	*i_status = mmc_run_cmd (p_cdio, 0, &cdb, SCSI_MMC_DATA_READ, 
		MMC_TRACK_INFORMATION_DATA_SIZE, *mmc_track_information_data);
}


#define MMC_DISC_INFORMATION_DATA_SIZE 42
/**
	Retrieves MMC DISC_INFORMATION DATA.

	@param p_cdio the CD object to be acted upon.
	
	@param i_status, on return will be set indicate whether the operation was
	a success (DRIVER_OP_SUCCESS) or if not to some other value.
	
	@param mmc_disc_information_data, in return. Free when no longer needed.
 */
void
_mmc_get_disc_information_data( const CdIo_t *p_cdio, 
				driver_return_code_t *i_status,
				uint8_t **mmc_disc_information_data) {
	mmc_cdb_t cdb = {{0, }};
	
	*mmc_disc_information_data = (uint8_t*) malloc (MMC_DISC_INFORMATION_DATA_SIZE * sizeof (uint8_t));
	memset (*mmc_disc_information_data, 0, MMC_DISC_INFORMATION_DATA_SIZE);
	CDIO_MMC_SET_COMMAND (cdb.field, CDIO_MMC_GPCMD_READ_DISC_INFO);
	CDIO_MMC_SET_READ_LENGTH16 (cdb.field, MMC_DISC_INFORMATION_DATA_SIZE);

	*i_status = mmc_run_cmd (p_cdio, 0, &cdb, SCSI_MMC_DATA_READ, 
			MMC_DISC_INFORMATION_DATA_SIZE, *mmc_disc_information_data);
}


/**
	Detects if a disc (CD or DVD) is empy or not.
  ** Warning: blanked DVD-RW report non empty ! **

	@param p_cdio the CD object to be acted upon.
	
	@param opt_i_status, if not NULL, on return will be set indicate whether
	the operation was a success (DRIVER_OP_SUCCESS) or if not to some
	other value.

	@return true if the disc is detected as empty, false otherwise.
 */
bool
_mmc_get_disc_empty( const CdIo_t *p_cdio, 
				driver_return_code_t *opt_i_status) {
	driver_return_code_t i_status;
	uint8_t *mmc_disc_information_data;
	uint8_t emptybyte; 
	
	_mmc_get_disc_information_data (p_cdio, &i_status, &mmc_disc_information_data);
	if (opt_i_status != NULL) *opt_i_status = i_status;
	emptybyte = mmc_disc_information_data[2] & 0x03;
	free (mmc_disc_information_data);

	return (DRIVER_OP_SUCCESS == i_status) ? 
			((emptybyte == 0x00) ? true : false)
			: false;
}


/**
	Detects a disc (CD or DVD) capacity.
	@param p_user_data the CD object to be acted upon.
	
	@param opt_i_status, if not NULL, on return will be set indicate whether
	the operation was a success (DRIVER_OP_SUCCESS) or if not to some
	other value.

	@return the detected disc capacity, or 0 on error;
	for non writable CDs, capacity is equal to size.
 */
uint32_t
_mmc_get_disc_capacity ( const CdIo_t *p_cdio, 
				driver_return_code_t *opt_i_status) {
	driver_return_code_t i_status;
	uint8_t *mmc_track_information_data;
	uint32_t capacity;
	uint8_t *p;
	
	_mmc_get_track_information_data (p_cdio, &i_status, &mmc_track_information_data, 0x01); //will "not work" with multi track CDs, but they have to be blanked before use
	if (opt_i_status != NULL) *opt_i_status = i_status;
	
	if (i_status == DRIVER_OP_SUCCESS) {
		p = mmc_track_information_data + 24;
		capacity = CDIO_MMC_GET_LEN32 (p);
	} else {
		capacity = 0;
	}
	
	free (mmc_track_information_data);
	
	capacity = capacity * 2;
	return capacity;
}
