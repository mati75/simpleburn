typedef struct {
	gchar cddevice[20];
	gboolean hasmedia;
	gboolean hasrewritablemedia;
	gboolean hascdrommedia;
	gboolean hasdvdmedia;
	gboolean hasemptymedia;
	gboolean hasaudiocdmedia;
	gboolean hasdatamedia;
	gboolean hasiso9660media;
	gboolean hasudfmedia;
	guint64 mediasize; //KiB (for data or audio medias;)
	guint64 mediacapacity;
	guint8 trackscount;
} T_cdinfos;
