#include <gtk/gtk.h>
#include <vte/vte.h>

static void split(GtkWidget *widget, GtkWidget *window) {
    GtkWidget *parent = gtk_widget_get_parent(widget);
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);

    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    GtkWidget *new_term = vte_terminal_new();
    gtk_widget_set_size_request(new_term, 1, 1);
    gtk_widget_set_vexpand(new_term, TRUE);

    g_object_ref(widget);
    gtk_container_remove(GTK_CONTAINER(parent), widget);

    gtk_paned_pack1(GTK_PANED(paned), widget, TRUE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned), new_term, TRUE, TRUE);

    gtk_paned_set_position(GTK_PANED(paned), alloc.height / 2);

    gtk_container_add(GTK_CONTAINER(parent), paned);
    g_object_unref(widget);
    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    GtkWidget *term1 = vte_terminal_new();
    gtk_widget_set_size_request(term1, 1, 1);
    gtk_widget_set_vexpand(term1, TRUE);
    gtk_box_pack_start(GTK_BOX(box), term1, TRUE, TRUE, 0);

    gtk_widget_show_all(window);
    
    // Simulate split after layout
    g_idle_add((GSourceFunc)split, term1);

    return 0;
}
