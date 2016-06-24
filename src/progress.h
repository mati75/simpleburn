struct {
	GPid pid;
	guint pulseupdater;
	gint actiontype; //EXTRACT, BLANK, BURN
} commandinfos;


#define EXTRACT 0
#define BLANK 1
#define BURN 2


void on_abort_clicked(GtkWidget *widget, gpointer user_data);

void startprogress(gchar *commandline, gboolean pulse, gchar *message, gint actiontype);

void stopprogress(GPid pid, gint status, gpointer data);

gboolean pulseupdater(gpointer data);

void enablebuttons(gboolean enabled);
