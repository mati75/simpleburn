struct {
	GPid pid;
	guint progressbar_handler;
	guint pulsebar_handler;
	guint elapsedtime_handler;
	GIOChannel *stdoutioc;
	GTimeVal starttime;
	GTimeVal progress_startttime;
	glong estimatedtime;
	//~ gchar *opticaldevice;
	gint actiontype; //EXTRACT, BLANK, BURN
} commandinfos;


#define EXTRACT 0
#define BLANK 1
#define BURN 2


void on_abort_clicked(GtkWidget *widget, gpointer user_data);

void startprogress(gchar *commandline, gboolean pulse, gchar *message, gint actiontype);

void stopprogress(GPid pid, gint status, gpointer data);

gboolean pulsebar_handler(gpointer data);

gboolean progressbar_handler(GIOChannel *source, GIOCondition condition, gpointer data);

gboolean elapsedtime_handler(gpointer data);

gchar *timeformat(glong timeinseconds);

void enablebuttons(gboolean enabled);